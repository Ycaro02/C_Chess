#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unload_texture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
	if (handle->player_info.dest_ip) {
		free(handle->player_info.dest_ip);
	}
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
	s32			ret = TRUE;
	
	while (1) {
		tile_selected = event_handler(handle->player_info.color);
		/* If the quit button is pressed */
		if (tile_selected == CHESS_QUIT) { break ; }
		
		/* If tile is selected */
		if (tile_selected != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (is_selected_possible_move(b->possible_moves, tile_selected)) {
				ret = move_piece(handle, b->selected_tile, tile_selected, piece_type);
				b->possible_moves = 0;
			} else { /* Update piece possible move and selected tile */
				piece_type = get_piece_from_tile(b, tile_selected);
				b->selected_tile = tile_selected;
				b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), piece_type, TRUE);
			}
		}

		if (ret == CHESS_QUIT) {
			break ;
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
		// player_info.color = IS_BLACK;
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

typedef enum msg_type {
	MSG_TYPE_COLOR,
	MSG_TYPE_MOVE,
	MSG_TYPE_PROMOTION,
	MSG_TYPE_QUIT,
} MsgType;

/**
 * Packet format 4 char
 * -	1: msg_type
 * if (msg_type == MSG_TYPE_MOVE)
 * - 	2: tile_from
 * - 	3: tile_to
 * - 	4: piece_type
 * if (msg_type == MSG_TYPE_COLOR)
 * -	2: color
 * if (msg_type == MSG_TYPE_PROMOTION)
 * -	2: tile_from
 * -	3: tile_to
 * -	4: NEW_piece_type (QUEEN, ROOK, BISHOP, KNIGHT)
 * -	@note The piece type is the new piece type, not the pawn type (WHITE_PAWN, BLACK_PAWN)
 * 
 */

void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[0];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY, opponent_pawn = EMPTY;
	

	// ft_printf_fd(1, "Message received: %d\n", msg_type);

	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[1];
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game\n");
		return ;
	}
	
	tile_from = msg[1];
	tile_to = msg[2];
	piece_type = msg[3];
	
	if (msg_type == MSG_TYPE_MOVE) {
		/* If the message is a move, just call move piece */
		move_piece(handle, tile_from, tile_to, piece_type);
	}  else if (msg_type == MSG_TYPE_PROMOTION) {
		/* If the message is a promotion message, promote the pawn */
		opponent_pawn = handle->player_info.color == IS_BLACK ? BLACK_PAWN : WHITE_PAWN;
		/* Remove the pawn */
		handle->board->piece[opponent_pawn] &= ~(1ULL << tile_from);
		/* Add the new piece */
		handle->board->piece[piece_type] |= (1ULL << tile_to);
		update_piece_state(handle->board);
	} 
}


char *build_message(s32 msg_size, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type) {
	char *msg = ft_calloc(msg_size, sizeof(char));

	/* If the alloc fail, return NULL */
	if (!msg) {
		return (NULL);
	}

	/* Set the message type */
	msg[0] = msg_type;
	if (msg_type == MSG_TYPE_QUIT) {
		return (msg);
	}

	/* Set the tile_from or color (1st data) */
	msg[1] = tile_from_or_color;
	/* If the message is a color message, return here */
	if (msg_type == MSG_TYPE_COLOR) {
		return (msg);
	}

	/* Set the tile_to (2nd data) */
	msg[2] = tile_to;

	/* Set the piece_type (3rd data) */
	msg[3] = piece_type;
	return (msg);
}

void display_message(char *msg) {
	MsgType msg_type = msg[0];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	if (msg_type == MSG_TYPE_COLOR) {
		ft_printf_fd(1, "Message color brut data: |%d||%d|\n", msg[0], msg[1]);
		ft_printf_fd(1, "Color: %s\n", msg[1] == IS_WHITE ? "WHITE" : "BLACK");
	} else if (msg_type == MSG_TYPE_MOVE) {
		tile_from = msg[1];
		tile_to = msg[2];
		piece_type = msg[3];
		ft_printf_fd(1, "Message move brut data: |%d||%d||%d||%d|\n", msg[0], msg[1], msg[2], msg[3]);
		ft_printf_fd(1, "Move from %s to %s with piece %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else if (msg_type == MSG_TYPE_PROMOTION) {
		tile_from = msg[1];
		tile_to = msg[2];
		piece_type = msg[3];
		ft_printf_fd(1, "Message promotion brut data: |%d||%d||%d||%d|\n", msg[0], msg[1], msg[2], msg[3]);
		ft_printf_fd(1, "Promotion from %s to %s with piece %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game, msg type %d\n", msg[0]);
	} else {
		ft_printf_fd(1, "Unknown message type\n");
	}
}

// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));
