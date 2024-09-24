#include "../include/chess_log.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"

/** File data format:
 * keyword: Nickame,		max_size: 8, 	idx: 0
 * keyword: Server,			max_size: 15,	idx: 1
 * keyword: NetworkPause,	max_size: 1,	idx: 2
 */

typedef struct s_file_data {
	char	*keyword;
	s32		data_size;
	u32		line_idx;
} FileData;

#define FILE_DATA_ARRAY { \
	{"Nickname", 8, 0}, \
	{"Server", 15, 1}, \
	{"NetworkPause", 1, 2}, \
	} \


char *get_file_data(char *path, char *keyword, u32 line_idx, int max_size) {
	// static const FileData file_data[] = FILE_DATA_ARRAY;
	char		*data = NULL, *data_file = NULL, **split_file_line = NULL, **split_line = NULL;
    SDL_RWops	*rw = NULL;
    s64 		size = 0;

    size = get_file_size_sdl(path);
    if (size < 0) {
        CHESS_LOG(LOG_DEBUG, "Getting file size\n");
		return (NULL);
    }

    rw = sdl_open(path, "r");
    if (!rw) {
        CHESS_LOG(LOG_DEBUG, "Opening data file\n");
		return (NULL);
    }

    data_file = ft_calloc(size + 1, sizeof(char));
    if (!data_file) {
        CHESS_LOG(LOG_DEBUG, "Allocating memory for data file\n");
        goto close_rw;
    }

    if (sdl_read_complete_file(rw, data_file, size) != (size_t)size) {
        CHESS_LOG(LOG_DEBUG, "Reading data file\n");
        goto free_file;
    }
    data_file[size] = '\0';

    split_file_line = ft_split(data_file, '\n');
    if (!split_file_line) {
        CHESS_LOG(LOG_DEBUG, "No first line\n");
        goto free_file;
    } else if (double_char_size(split_file_line) < line_idx + 1) {
        CHESS_LOG(LOG_DEBUG, "Missing line need at least %d line\n", line_idx + 1);
        goto free_first_split;
    }

    split_line = ft_split(split_file_line[line_idx], ':');
    if (!split_line) {
        CHESS_LOG(LOG_DEBUG, "First line error\n");
        goto free_first_split;
    } else if (double_char_size(split_line) != 2) {
        CHESS_LOG(LOG_DEBUG, "Missing separator ':' or too many separator format is 'keyword:value'\n");
        goto free_second_split;
    } else if (ft_strncmp(split_line[0], keyword, ft_strlen(keyword)) != 0) {
        CHESS_LOG(LOG_DEBUG, "Wrong keyword need '%s', got: |%s|\n"RESET, keyword, split_line[0]);
        goto free_second_split;
    }

    CHESS_LOG(LOG_INFO, "data_file line[%u]: %s\n", line_idx, split_file_line[0]);
    data = ft_strtrim(split_line[1], " \t\n\r\v\f");

    if (fast_strlen(data) >= max_size) {
        CHESS_LOG(LOG_DEBUG, "Nickname too long max is 7 character\n");
        free(data);
        data = NULL;
    } else {
        CHESS_LOG(LOG_INFO, "Nickname: |%s|\n", data);
    }

	free_second_split:
		free_double_char(split_line);

	free_first_split:
		free_double_char(split_file_line);

	free_file:
		free(data_file);

	close_rw:
		sdl_close(rw);

	return (data);
}

void register_file_data(char *path, char *keyword, char *data) {
	SDL_RWops	*rw = NULL;
	char		*new_data = NULL;

	new_data = ft_strjoin(keyword, data);
	if (!new_data) {
		CHESS_LOG(LOG_DEBUG, "Malloc new data\n");
		return;
	}

	// open in append mode
	rw = sdl_open(path, "a");
	if (!rw) {
		CHESS_LOG(LOG_DEBUG, "Opening file\n");
	    free(new_data);
	}

	if (sdl_write(rw, new_data, sizeof(char), fast_strlen(new_data)) < 0) {
		CHESS_LOG(LOG_DEBUG, "Writing file\n");
	}

	CHESS_LOG(LOG_INFO, "Registering data: %s\n", new_data);

	free(new_data);
	sdl_close(rw);

}

void set_netword_pause_data(char *path, s8 state) {
	char data[2] = {0};

	data[0] = state ? '1' : '0';
	register_file_data(path, "\nNetworkPause:", data);
}

void register_data(SDLHandle *h, char *relatif_path) {
	sdl_erase_file_data(relatif_path);
	register_file_data(relatif_path, "Nickname:", h->player_info.name);
	register_file_data(relatif_path, "\nServer:", h->player_info.dest_ip);

	s8 to_reconnect = FALSE;
	if (has_flag(h->flag, FLAG_NETWORK) && h->game_start) {
		if (h->player_info.nt_info && h->player_info.nt_info->peer_conected) {
			to_reconnect = TRUE;
		}
	}

	set_netword_pause_data(relatif_path, to_reconnect);
}


