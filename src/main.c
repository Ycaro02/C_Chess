#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unload_texture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
}

SDLHandle *init_game() {
	SDLHandle	*handle = NULL;

	handle = create_sdl_handle(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
	if (!handle) {
		ft_printf_fd(2, "Error: create_sdl_handle failed\n");
		return (NULL);
	}
	handle->board = ft_calloc(1, sizeof(ChessBoard));
	if (!handle->board) {
		ft_printf_fd(2, "Error: malloc failed\n");
		free(handle);
		return (NULL);
	}
	init_board(handle->board);
	handle->player_info.color = IS_WHITE;
	handle->player_info.turn = IS_WHITE;
	return (handle);
}

void chess_routine(SDLHandle *handle){
	ChessTile	tile_selected = INVALID_TILE;
	ChessPiece	piece_type = EMPTY;
	ChessBoard	*b = handle->board;
	
	while (1) {
		tile_selected = event_handler(handle->player_info.color);
		/* If the quit button is pressed */
		if (tile_selected == CHESS_QUIT) { break ; }
		
		/* If tile is selected */
		if (tile_selected != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (is_selected_possible_move(b->possible_moves, tile_selected)) {
				move_piece(b, b->selected_tile, tile_selected, piece_type);
				b->possible_moves = 0;
			} else { /* Update piece possible move and selected tile */
				piece_type = get_piece_from_tile(b, tile_selected);
				b->selected_tile = tile_selected;
				b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), piece_type, TRUE);
			}
		}

		/* Draw logic */
		window_clear(handle->renderer);
		draw_board(handle, handle->player_info.color);
		SDL_RenderPresent(handle->renderer);
	}

	/* Free memory */
	destroy_sdl_handle(handle);
	window_close(handle->window, handle->renderer);
	free(handle);
}

typedef enum msg_type {
	MSG_TYPE_COLOR,
	MSG_TYPE_MOVE,
} MsgType;

/**
 * Packet format 4 char
 * 1: msg_type
 * if (msg_type == MSG_TYPE_MOVE)
 * 2: tile_from
 * 3: tile_to
 * 4: piece_type
 * if (msg_type == MSG_TYPE_COLOR)
 * 2: color
 */


#include "../libft/parse_flag/parse_flag.h"

typedef t_flag_context ChessFlagContext;

#define LISTEN_OPT_CHAR	'l'
#define JOIN_OPT_CHAR	'j'
#define PORT_OPT_CHAR	'p'

enum chess_flag_value {
	FLAG_LISTEN=1<<0,
	FLAG_JOIN=1<<1,
	FLAG_PORT=1<<2,
};

#define LISTEN_STR		"listen"
#define JOIN_STR		"join"
#define PORT_STR		"port"
#define DEFAULT_PORT 	24242
#define MAX_PORT		65535

/*
	@Brief Parser flag to handle listen and join mode
	- Listen mode: Wait for connection
		* -l --listen, wait for connection, default port 8080
		* -p <port>, default port 8080
		* example ./C_chess -l 8081
	- Join mode: Connect to a server
		* -j --join <ip> 
		* -p <port>, default port 8080
		* example ./C_chess -j 192.168.1.1 -p 8081
*/
u32 handle_chess_flag(int argc, char **argv, s8 *error, PlayerInfo *player_info) {
	u32					*alloc_port = NULL;
	ChessFlagContext	flag_ctx;
	u32					flag_value = 0, port = DEFAULT_PORT;

	ft_bzero(&flag_ctx, sizeof(ChessFlagContext));

	add_flag_option(&flag_ctx, LISTEN_OPT_CHAR, FLAG_LISTEN, OPT_NO_VALUE, OPT_NO_VALUE, LISTEN_STR);
	add_flag_option(&flag_ctx, JOIN_OPT_CHAR, FLAG_JOIN, 15, CHAR_VALUE, JOIN_STR);
	add_flag_option(&flag_ctx, PORT_OPT_CHAR, FLAG_PORT, 65535, DECIMAL_VALUE, PORT_STR);

	flag_value = parse_flag(argc, argv, &flag_ctx, error);
	if (*error == -1) {
		ft_printf_fd(2, "Error: Flag parser%s\n");
		display_option_list(flag_ctx);
		goto flag_error;
	}
	if (has_flag(flag_value, FLAG_LISTEN) && has_flag(flag_value, FLAG_JOIN)) {
		ft_printf_fd(2, "Error: Can't have listen and join flag at the same time\n");
		goto flag_error;
	} else if (!has_flag(flag_value, FLAG_LISTEN) && !has_flag(flag_value, FLAG_JOIN)) {
		ft_printf_fd(2, "Error: Need to have listen or join flag\n");
		goto flag_error;
	}

	if (has_flag(flag_value, FLAG_PORT)) {
		alloc_port = get_opt_value(flag_ctx.opt_lst, flag_value, FLAG_PORT);
		port = *alloc_port;
		free(alloc_port);
	}

	player_info->dest_ip = NULL;
	player_info->running_port = port;

	if (has_flag(flag_value, FLAG_JOIN)) {
		player_info->dest_ip = get_opt_value(flag_ctx.opt_lst, flag_value, FLAG_JOIN);
	}
	
	display_option_list(flag_ctx);
	return (flag_value);

	flag_error:
		display_option_list(flag_ctx);
		*error = -1;
		return (0);
}



int main(int argc, char **argv) {
	SDLHandle	*handle = NULL;
	PlayerInfo	player_info = {0};
	u32			flag = 0;
	s8			error = 0;

	flag = handle_chess_flag(argc, argv, &error, &player_info);
	if (error == -1) {
		return (1);
	}
	if (has_flag(flag, FLAG_LISTEN)) {
		// player_info.color = random_player_color();
		player_info.color = IS_WHITE;
		ft_printf_fd(1, "Player color: %s\n", player_info.color == IS_WHITE ? "WHITE" : "BLACK");
		ft_printf_fd(1, "Running port: %d\n", player_info.running_port);
		ft_printf_fd(1, "Waiting for connection...\n");
	} else {
		/* Need to receive color from first player here */
		// player_info.color = random_player_color();
		player_info.color = IS_BLACK;
		ft_printf_fd(1, "Ip adress dest %s\n", player_info.dest_ip);
		ft_printf_fd(1, "Running port: %d\n", player_info.running_port);
	}


	handle = init_game();
	if (!handle) {
		return (1);
	}

	handle->player_info = player_info;

	chess_routine(handle);
	return (0);
}


// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));
