#include "../include/chess.h"
#include "../include/chess_log.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/FEN_notation.h"
#include "../include/chess_bot.h"


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
    CHESS_LOG(LOG_INFO, "Extracting move from string: %s, len: %d\n", move_str, move_str_len);
	if (move_str_len < 4) {
		CHESS_LOG(LOG_ERROR, "Incomplete move string\n");
		return ;
	} else if (move_str_len == 5) {
        ChessGenericPieceLetter promotion_letter = (ChessGenericPieceLetter)move_str[4];
        if (promotion_letter != LETTER_QUEEN && promotion_letter != LETTER_ROOK && \
            promotion_letter != LETTER_BISHOP && promotion_letter != LETTER_KNIGHT) {
            CHESS_LOG(LOG_ERROR, "Invalid promotion piece letter: %c\n", move_str[4]);
            return ;
        }
		move->promotion_piece_letter = promotion_letter;
	}
	ft_memcpy(move_from, move_str, 2);
	ft_memcpy(move_to, move_str + 2, 2);
	move->from = str_to_chesstile(move_from);
	move->to = str_to_chesstile(move_to);
	CHESS_LOG(LOG_INFO, "Receive: from: %s, to: %s\n", ChessTile_to_str(move->from), ChessTile_to_str(move->to));
    CHESS_LOG(LOG_INFO, "Promotion piece letter: %c\n", move->promotion_piece_letter);
}

MoveStruct invalid_move_struct() {
    MoveStruct move = {INVALID_TILE, INVALID_TILE, LETTER_INVALID};
    return (move);
}

/**
 * @brief Get the move from the Stockfish response
 * @param response The response from Stockfish
 * @return The move
 */
MoveStruct get_move_from_response(char *response) {
	MoveStruct move = invalid_move_struct();

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

    char *trim_move_str = ft_strtrim(split_space[1], " \t\n\r\"'");

	/* Extract the move from the split space string */
	extract_move_from_str(trim_move_str, &move);
	free(trim_move_str);

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

s8 is_valid_move_struct(MoveStruct move) {
    if (move.from == INVALID_TILE || move.to == INVALID_TILE) {
        CHESS_LOG(LOG_ERROR, "Invalid move received from Stockfish\n");
        return (FALSE);
    } else if (move.from < A1 || move.from > H8 || move.to < A1 || move.to > H8) {
        CHESS_LOG(LOG_ERROR, "Move out of bounds received from Stockfish\n");
        return (FALSE);
    }
    return (TRUE);
}



/**
 * @brief Send the FEN string to the Stockfish API
 * @param fen_str The FEN string
 */
MoveStruct send_stockfish_fen(char *fen_str) {
	char *url = NULL;
	
    // url = build_stockfish_request(fen_str, get_random_depth(1, 2));
    url = build_stockfish_request(fen_str, 8);
	if (!url) {
		CHESS_LOG(LOG_ERROR, "Failed to build the URL\n");
		return (invalid_move_struct());
	}

    char *response = send_http_request(url);
    CHESS_LOG(LOG_INFO, "Response from Stockfish:\n%s\n", response ? response : "NULL");
    MoveStruct move = get_move_from_response(response);
    CHESS_LOG(LOG_INFO, "Move from Stockfish: from %s to %s\n", ChessTile_to_str(move.from), ChessTile_to_str(move.to));
    free(response);
	free(url);

    return (move);
}

static ChessPiece promotion_letter_to_piece(ChessGenericPieceLetter letter, s8 is_black) {
    switch (letter) {
        case LETTER_QUEEN:
            return is_black ? BLACK_QUEEN : WHITE_QUEEN;
        case LETTER_ROOK:
            return is_black ? BLACK_ROOK : WHITE_ROOK;
        case LETTER_BISHOP:
            return is_black ? BLACK_BISHOP : WHITE_BISHOP;
        case LETTER_KNIGHT:
            return is_black ? BLACK_KNIGHT : WHITE_KNIGHT;
        default:
            return EMPTY; // Invalid letter
    }
    return EMPTY;
}


void play_stockfish_move(SDLHandle *h) {
    char *fen = build_FEN_notation(h);
    MoveStruct move = send_stockfish_fen(fen);
    free(fen);

    if (is_valid_move_struct(move)) {
        CHESS_LOG(LOG_INFO, "Stockfish suggests move from %s to %s\n", ChessTile_to_str(move.from), ChessTile_to_str(move.to));
        
        h->board->selected_tile = move.from;
        h->board->last_clicked_tile = move.to;
        h->board->selected_piece = get_piece_from_tile(h->board, h->board->selected_tile);
        h->board->is_bot_playing = TRUE;

        call_move_piece_handling(h, h->board);
        
        s8 is_black = h->board->selected_piece >= BLACK_PAWN;

        if (move.promotion_piece_letter != LETTER_INVALID) {
            ChessPiece promotion_piece = promotion_letter_to_piece(move.promotion_piece_letter, h->board->selected_piece);
            CHESS_LOG(LOG_INFO, "Promoting pawn to %s\n", ChessPiece_to_str(promotion_piece));
            promote_pawn(h->board, move.to, promotion_piece, is_black ? BLACK_PAWN : WHITE_PAWN);
        }
        
        // handle_locale_turn(h);
        h->board->is_bot_playing = FALSE;
        
    } else {
        CHESS_LOG(LOG_INFO, "Stockfish did not return a valid move\n");
    }
    // (void)fen;
    return ;
}


void stockfish_enable(SDLHandle *h) {
    if (!has_flag(h->flag, FLAG_NETWORK)) {
        CHESS_LOG(LOG_INFO, "Enabling/Disabling Stockfish bot mode.\n");
        if (has_flag(h->flag, FLAG_STOCKFISH_BOT)) {
            unset_flag(&h->flag, FLAG_STOCKFISH_BOT);
            s8 is_black = (h->player_info.color == IS_BLACK);
            h->player_info.turn = (is_black && h->board->halfmove_count % 2 == 0) || (!is_black && h->board->halfmove_count % 2 != 0) ? FALSE : TRUE;
        } else {
            init_board(h->board, &h->flag);
            set_flag(&h->flag, FLAG_STOCKFISH_BOT);
            s8 is_black = (h->player_info.color == IS_BLACK);
            s8 color_turn = (h->board->halfmove_count % 2 == 0) ? IS_WHITE : IS_BLACK;
            CHESS_LOG(LOG_INFO, "Color turn: %s\n", color_turn == IS_WHITE ? "White" : "Black");
            h->player_info.turn = (is_black && color_turn == IS_BLACK) || (!is_black && color_turn == IS_WHITE) ? TRUE : FALSE;
            h->player_info.piece_start = is_black ? BLACK_PAWN : WHITE_PAWN;
            h->player_info.piece_end = is_black ? BLACK_KING : WHITE_KING;
        }
        CHESS_LOG(LOG_INFO, "Stockfish bot mode %s.\n", has_flag(h->flag, FLAG_STOCKFISH_BOT) ? "enabled" : "disabled");
        CHESS_LOG(LOG_INFO, "HALF MOVE COUNT: %d\n", h->board->halfmove_count);
        CHESS_LOG(LOG_INFO, "TURN: %s, Player Color: %s, Start Piece: %s, End Piece: %s\n",
            h->player_info.turn ? "TRUE" : "FALSE",
            h->player_info.color == IS_WHITE ? "White" : "Black",
            h->player_info.piece_start == WHITE_PAWN ? "WHITE_PAWN" : "BLACK_PAWN",
            h->player_info.piece_end == WHITE_KING ? "White King" : "Black King"
        );
    }
}