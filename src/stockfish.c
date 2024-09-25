#include "../include/chess.h"
#include "../include/chess_log.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/FEN_notation.h"

#include "../include/chess_bot.h"

#define STOCKFISH_URL "https://stockfish.online/api/s/v2.php?fen="


typedef enum {
	LEVEL_EASY,
	LEVEL_MEDIUM,
	LEVEL_HARD,
	LEVEL_EXPERT,
} BotLevel;

typedef struct s_bot_skill_level {
	BotLevel	level;
	char		*name;
	u8			depth_min;
	u8			depth_max;
} BotSkillLevel;

#define SKILL_LEVEL_ARRAY_SIZE 4

#define SKILL_LVL_ARRAY { \
	{LEVEL_EASY, "Easy", 1, 2}, \
	{LEVEL_MEDIUM, "Medium", 3, 4}, \
	{LEVEL_HARD, "Hard", 5, 7}, \
	{LEVEL_EXPERT, "Expert", 8, 12} \
}

u8 get_random_depth(u8 min_depth, u8 max_depth) {
	return (rand() % (max_depth - min_depth + 1) + min_depth);
}


ChessTile str_to_chesstile(char *str) {
    char file = str[0]; // 'a' to 'h'
    char rank = str[1]; // '1' to '8'

    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
        fprintf(stderr, "Invalid chess tile string: %s\n", str);
        exit(EXIT_FAILURE);
    }

    int file_index = file - 'a'; // 0 to 7
    int rank_index = rank - '1'; // 0 to 7

    return (ChessTile)(rank_index * 8 + file_index);
}

typedef struct s_move_struct {
	ChessTile	from;
	ChessTile	to;
	char		promotion_piece;
} MoveStruct;

char *get_keyword_line(char **split_comma, char *keyword) {
	
	
	for (int i = 0; split_comma[i]; i++) {
		if (ft_strncmp(split_comma[i], keyword, ft_strlen(keyword)) == 0) {
			return (ft_strdup(split_comma[i]));
		}
	}
	return (NULL);
}

s8 verify_request_status(char **split_comma) {
	int success_len = ft_strlen("\"success\"");

	for (int i = 0; split_comma[i]; i++) {
		if (ft_strncmp(split_comma[i], "\"success\"", success_len) == 0) {
			/* Chess + success_len + 1 for the double dot */
			if (ft_strncmp(split_comma[i] + success_len + 1, "true", ft_strlen("true")) != 0) {
				CHESS_LOG(LOG_ERROR, "Failed to get the move, request fail\n");
				return (FALSE);
			}
		}
	}
	return (TRUE);
}

void extract_move_from_str(char *move_str, MoveStruct *move) {
	char move_from[3] = {0};
	char move_to[3] = {0};
	int move_str_len = ft_strlen(move_str);
	if (move_str_len < 4) {
		CHESS_LOG(LOG_ERROR, "Incomplete move string\n");
		return ;
	} else if (move_str_len == 5) {
		move->promotion_piece = move_str[4];
	}
	ft_memcpy(move_from, move_str, 2);
	ft_memcpy(move_to, move_str + 2, 2);
	move->from = str_to_chesstile(move_from);
	move->to = str_to_chesstile(move_to);
	CHESS_LOG(LOG_INFO, "Receive: from: %s, to: %s\n", ChessTile_to_str(move->from), ChessTile_to_str(move->to));
}

/**
 * @brief Get the move from the Stockfish response
 * @param response The response from Stockfish
 * @return The move
 */
MoveStruct get_move_from_response(char *response) {
	MoveStruct move = {INVALID_TILE, INVALID_TILE, EMPTY};

	/* Split the response by comma to get field line */
	char **split_comma = ft_split(response, ',');
	if (!split_comma) {
		CHESS_LOG(LOG_ERROR, "Failed to split the response\n");
		return (move);
	}

	/* Verify the request status (success field) */
	if (!verify_request_status(split_comma)) {
		goto free_coma_split;
	}

	/* Get the best move line */
	char *best_move_line = get_keyword_line(split_comma, "\"bestmove\"");
	if (!best_move_line) {
		CHESS_LOG(LOG_ERROR, "Failed to find the best move line\n");
		goto free_coma_split;
	}

	CHESS_LOG(LOG_INFO, "Best move line: %s\n", best_move_line);

	/* Split the best move line by double dot to gey, keyword + value */
	char **split_double_dot = ft_split(best_move_line, ':');
	if (!split_double_dot) {
		CHESS_LOG(LOG_ERROR, "Failed to split the best move line\n");
		goto free_best_move_line;
	}

	/* Check if the split is not empty and contain keyword + value */
	if (double_char_size(split_double_dot) < 2) {
		CHESS_LOG(LOG_ERROR, "Failed to find the best move\n");
		goto free_split_double_dot;
	}

	/* Split the value by space to get the move */
	char **split_space = ft_split(split_double_dot[1], ' ');
	if (!split_space) {
		CHESS_LOG(LOG_ERROR, "Failed to split the best move\n");
		goto free_split_double_dot;
	}

	/* Check if the split is not empty and contain the move */
	if (double_char_size(split_space) < 2) {
		CHESS_LOG(LOG_ERROR, "Failed to find the best move string\n");
		goto free_split_space;
	}

	/* Extract the move from the split space string */
	extract_move_from_str(split_space[1], &move);

	free_split_space:
		free_double_char(split_space);
	free_split_double_dot:
		free_double_char(split_double_dot);
	free_best_move_line:
		free(best_move_line);
	free_coma_split:
		free_double_char(split_comma);

	return (move);
}


/**
 * @brief Replace a character by a string
 * @param str The string to replace character
 * @param to_replace The character to replace
 * @param replace_with The string to replace with
 * @param last_replace Last replace boolean, for the last replace or not
 * @return The new string
 */
char *replace_char_by_str(char *str, char to_replace, char *replace_with, s8 last_replace) {
	char **split = ft_split(str, to_replace);
	char *new_str = NULL;

	int count = double_char_size(split);

	// Loop through the split string
	for (int i = 0; split[i]; i++) {
		// If the split string is not the last element
		new_str = ft_strjoin_free(new_str, split[i], 'a');
		if (!last_replace && i < count - 1) {
			// Concatenate the split string with a '%20'
			new_str = ft_strjoin_free(new_str, replace_with, 'f');
		}

	}
	free(split);
	CHESS_LOG(LOG_INFO, "New string: "PINK"|%s|\n"RESET, new_str);
	return (new_str);
}

/**
 * @brief Build the Stockfish request URL
 * @param fen_str The FEN string
 * @param depth The depth of the search
 * @return The URL
 */
char *build_stockfish_request(char *fen_str, int depth) {
	char *encode_fen = replace_char_by_str(fen_str, ' ', "%20", FALSE);

	char *url = ft_strjoin_free(STOCKFISH_URL, encode_fen, 's');

	url = ft_strjoin_free(url, "&depth=", 'f');
	url = ft_strjoin_free(url, ft_itoa(depth), 'a');

	CHESS_LOG(LOG_INFO, "URL: |%s|\n", url);
	return (url);

}

/**
 * @brief Write callback for libcurl
 * @param ptr The pointer to the data
 * @param size The size of the data
 * @param nmemb The number of members
 * @param userdata The user data
 * @return The total size
 */
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    ft_memcpy(userdata, ptr, total_size);
    return total_size;
}


/**
 * @brief Send the FEN string to the Stockfish API
 * @param fen_str The FEN string
 */
void send_stockfish_fen(char *fen_str) {
	CURL *curl = NULL;
    CURLcode res = 0;
	char *url = NULL;
	// Create a buffer to store the response data
	char response_data[4096];
	ft_bzero(response_data, 4096);

	url = build_stockfish_request(fen_str, get_random_depth(1, 2));
	if (!url) {
		CHESS_LOG(LOG_ERROR, "Failed to build the URL\n");
		return ;
	}

    // Initialize libcurl
    curl = curl_easy_init();
    if(curl) {
        // Set the URL with FEN and depth parameters
        curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);


        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            CHESS_LOG(LOG_INFO, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			CHESS_LOG(LOG_INFO, "Response: %s\n", response_data);
			MoveStruct move = get_move_from_response(response_data);
			(void)move;
		}

        // Libcurl cleanup
        curl_easy_cleanup(curl);
    }
	free(url);
}
