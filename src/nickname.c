#include "../include/chess_log.h"
#include "../include/chess.h"
#include "../include/handle_sdl.h"


char *get_nickname_in_file() {
    char		*nickname = NULL, *nickname_file = NULL, **split_file_line = NULL, **split_line = NULL;
    SDL_RWops	*rw = NULL;
    s64 		size = 0;


    size = get_file_size_sdl(NICKNAME_FILE);
    if (size < 0) {
        CHESS_LOG(LOG_ERROR, "Getting file size\n");
		return (NULL);
    }

    rw = sdl_open(NICKNAME_FILE, "r");
    if (!rw) {
        CHESS_LOG(LOG_ERROR, "Opening nickname file\n");
        return (NULL);
    }

    nickname_file = ft_calloc(size + 1, sizeof(char));
    if (!nickname_file) {
        CHESS_LOG(LOG_ERROR, "Allocating memory for nickname file\n");
        goto close_rw;
    }

    if (sdl_read_complete_file(rw, nickname_file, size) != (size_t)size) {
        CHESS_LOG(LOG_ERROR, "Reading nickname file\n");
        goto free_file;
    }
    nickname_file[size] = '\0';

    split_file_line = ft_split(nickname_file, '\n');
    if (!split_file_line) {
        CHESS_LOG(LOG_ERROR, "No first line\n");
        goto free_file;
    } else if (double_char_size(split_file_line) < 1) {
        CHESS_LOG(LOG_ERROR, "Missing line need at least 1 line\n");
        goto free_first_split;
    }

    split_line = ft_split(split_file_line[0], ':');
    if (!split_line) {
        CHESS_LOG(LOG_ERROR, "First line error\n");
        goto free_first_split;
    } else if (double_char_size(split_line) != 2) {
        CHESS_LOG(LOG_ERROR, "Missing separator ':' or too many separator format is 'keyword:value'\n");
        goto free_second_split;
    } else if (fast_strcmp(split_line[0], "nickname") != 0) {
        CHESS_LOG(LOG_ERROR, "Wrong keyword need 'nickname'\n");
        goto free_second_split;
    }

    CHESS_LOG(LOG_INFO, "Nickname_file line[0]: %s\n", split_file_line[0]);
    nickname = ft_strtrim(split_line[1], " \t\n\r\v\f");

    if (fast_strlen(nickname) >= 8) {
        CHESS_LOG(LOG_ERROR, "Nickname too long max is 7 character\n");
        free(nickname);
        nickname = NULL;
    } else {
        CHESS_LOG(LOG_INFO, "Nickname: |%s|\n", nickname);
    }

	free_second_split:
		free_double_char(split_line);

	free_first_split:
		free_double_char(split_file_line);

	free_file:
		free(nickname_file);

	close_rw:
		sdl_close(rw);
		return nickname;
}

void register_nickname(char *nickname, char *path) {
    SDL_RWops	*rw = NULL;
    char		*new_nickname = NULL;

    new_nickname = ft_strjoin("nickname:", nickname);
    if (!new_nickname) {
        CHESS_LOG(LOG_ERROR, "Malloc new nickname\n");
        return;
    }

    rw = sdl_open(path, "w");
    if (!rw) {
        CHESS_LOG(LOG_ERROR, "Opening nickname file\n");
	    free(new_nickname);
    }

    if (sdl_write(rw, new_nickname, sizeof(char), fast_strlen(new_nickname)) < 0) {
        CHESS_LOG(LOG_ERROR, "Writing nickname file\n");
    }

    CHESS_LOG(LOG_INFO, "Registering nickname: %s\n", new_nickname);

	free(new_nickname);
    sdl_close(rw);
}

// char *get_nickname_in_file() {
// 	u64		size = 0;
// 	char	*nickname = NULL, *nickname_file, **split_file_line, **split_line = NULL;
// 	int		fd = -1;

// 	fd = open(NICKNAME_FILE, O_RDONLY);
// 	if (fd < 0) {
// 		CHESS_LOG(LOG_ERROR, "Opening nickname file\n");
// 		return (NULL);
// 	}

// 	nickname_file = sstring_read_fd(fd, NICKNAME_FILE, &size);
// 	if (nickname_file == NULL) {
// 		CHESS_LOG(LOG_ERROR, "Reading nickname file\n");
// 		goto close_fd;
// 	} else if (size < 0) {
// 		CHESS_LOG(LOG_ERROR, "Empty nickname file\n");
// 		goto free_file;
// 	}

// 	split_file_line = ft_split(nickname_file, '\n');

// 	if (!split_file_line) {
// 		CHESS_LOG(LOG_ERROR, "No first line\n");
// 		goto free_file;
// 	} else if (double_char_size(split_file_line) < 1) {
// 		CHESS_LOG(LOG_ERROR, "Missing line need at least 1 line\n");
// 		goto free_first_split;
// 	}

// 	split_line = ft_split(split_file_line[0], ':');

// 	if (!split_line) {
// 		CHESS_LOG(LOG_ERROR, "First line error\n");
// 		goto free_first_split;
// 	} else if (double_char_size(split_line) != 2) {
// 		CHESS_LOG(LOG_ERROR, "Missing separator ':' or too many separator format is 'keyword:value'\n");
// 		goto free_second_split;
// 	} else if (fast_strcmp(split_line[0], "nickname") != 0) {
// 		CHESS_LOG(LOG_ERROR, "Wrong keyword need 'nickname'\n");
// 		goto free_second_split;
// 	}

// 	CHESS_LOG(LOG_INFO, "Nickname_file line[0]: %s\n", split_file_line[0]);
// 	nickname = ft_strtrim(split_line[1], " \t\n\r\v\f");

// 	if (fast_strlen(nickname) >= 8) {
// 		CHESS_LOG(LOG_ERROR, "Nickname too long max is 7 character\n");
// 		free(nickname);
// 		nickname = NULL;
// 	} else {
// 		CHESS_LOG(LOG_INFO, "Nickname: |%s|\n", nickname);
// 	}

// 	free_second_split:
// 		free_double_char(split_line);

// 	free_first_split:
// 		free_double_char(split_file_line);

// 	free_file:
// 		free(nickname_file);

// 	close_fd:
// 		close(fd);
// 		return (nickname);
// }

// void register_nickname(char *nickname, char *path) {
// 	int		fd = -1;
// 	char	*new_nickname = NULL;

// 	new_nickname = ft_strjoin("nickname:", nickname);
// 	if (new_nickname == NULL) {
// 		CHESS_LOG(LOG_ERROR, "Malloc new nickname\n");
// 		return ;
// 	}

// 	fd = open(path, O_WRONLY | O_TRUNC);
// 	if (fd < 0) {
// 		CHESS_LOG(LOG_ERROR, "Opening nickname file\n");
// 		goto free_new_nickname;
// 	}

// 	if (write(fd, new_nickname, fast_strlen(new_nickname)) < 0) {
// 		CHESS_LOG(LOG_ERROR, "Writing nickname file\n");
// 	}

// 	CHESS_LOG(LOG_INFO, "Registering nickname: %s\n", new_nickname);

// 	close(fd);

// 	free_new_nickname:
// 		free(new_nickname);
// }