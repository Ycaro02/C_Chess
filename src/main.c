#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unload_texture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
	if (handle->player_info.dest_ip) {
		free(handle->player_info.dest_ip);
	}
	send_disconnect_to_server(handle->player_info.nt_info->sockfd, handle->player_info.nt_info->servaddr);
	if (handle->player_info.nt_info) {
		close(handle->player_info.nt_info->sockfd);
		free(handle->player_info.nt_info);
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
	handle->player_info.turn = FALSE;
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

void netword_chess_routine(SDLHandle *h) {
	ChessTile	tile_selected = INVALID_TILE;
	ChessPiece	piece_type = EMPTY;
	ChessBoard	*b = h->board;
	s32			ret = TRUE;
	
	while (1) {
		tile_selected = event_handler(h->player_info.color);
		/* If the quit button is pressed */
		if (tile_selected == CHESS_QUIT) { break ; } // Send quit message to the other player
		
		/* If tile is selected */
		if (tile_selected != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (h->player_info.turn == TRUE && is_selected_possible_move(b->possible_moves, tile_selected)) {
				ret = move_piece(h, b->selected_tile, tile_selected, piece_type);
				b->possible_moves = 0;
				/* Send move message to the other player */
				// char *msg = build_message(4, MSG_TYPE_MOVE, b->selected_tile, tile_selected, piece_type);
				// ret = chess_msg_send(..., msg); // Send move message to the other player
				// if (ret == FALSE) { If the send fail, try to resend message or quit the game }
				// else { h->player_info.turn = FALSE ; } 
			} else { /* Update piece possible move and selected tile */
				piece_type = get_piece_from_tile(b, tile_selected);
				b->selected_tile = tile_selected;
				b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), piece_type, TRUE);
			}
			/* Receive message from the other player */
			// need to adapt the function to receive message to be non blocking
			// char *msg_rcv = chess_msg_receive(...);
			// if (msg_rcv) {
			// process_message_receive(h, msg_rcv);
			// }
		}

		if (ret == CHESS_QUIT) {
			break ; // Send quit message to the other player
		}

		/* Draw logic */
		window_clear(h->renderer);
		draw_board(h, h->player_info.color);
		SDL_RenderPresent(h->renderer);
	}

	/* Free memory */
	destroy_sdl_handle(h);
	window_close(h->window, h->renderer);
	free(h);
}


void display_message(char *msg) {
	MsgType msg_type = msg[0];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	ft_printf_fd(1, "Message type: %s: ", message_type_to_str(msg_type));

	if (msg_type == MSG_TYPE_COLOR) {
		ft_printf_fd(1, "brut data: |%d||%d| ->", msg[0], msg[1]);
		ft_printf_fd(1, "Color: %s\n", (msg[1] - 1) == IS_WHITE ? "WHITE" : "BLACK");
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game, msg type %d\n", msg[0]);
		return ;
	}	
	
	/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
	tile_from = msg[1] - 1;
	tile_to = msg[2] - 1;
	piece_type = msg[3] - 1;
	ft_printf_fd(1, "brut data: |%d||%d||%d||%d|\n", msg[0], msg[1], msg[2], msg[3]);
	if (msg_type == MSG_TYPE_MOVE) {
		ft_printf_fd(1, "Move from %s to %s with piece %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else if (msg_type == MSG_TYPE_PROMOTION) {
		ft_printf_fd(1, "Promotion from %s to %s with piece %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else {
		ft_printf_fd(1, "Unknown message type\n");
	}
}


#define MAX_ITER 50

int network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip) {
	struct timeval timeout = {TIMEOUT_SEC, 0};
	int test_iter = 0;
	char *tmp = NULL;

	player_info->nt_info = init_network(server_ip, player_info->running_port, timeout);
	if (has_flag(flag, FLAG_LISTEN)) {
		player_info->color = random_player_color();
		ft_printf_fd(1, "Waiting for player...\n");
		tmp = build_message(3, MSG_TYPE_COLOR, !player_info->color, 0, 0);
		chess_msg_send(player_info->nt_info, tmp);
	}

	if (has_flag(flag, FLAG_JOIN)) {
		tmp = NULL;
		while (tmp == NULL && test_iter < MAX_ITER) {
			tmp = chess_msg_receive(player_info->nt_info);
			test_iter++;
			sleep(1);
		}
		display_message(tmp);
		process_message_receive(handle, tmp);
	}

	ft_printf_fd(1, YELLOW"Player color: %s\n"RESET, player_info->color == IS_WHITE ? "WHITE" : "BLACK");
	if (player_info->color == IS_WHITE) {
		player_info->turn = TRUE;
	} else {
		player_info->turn = FALSE;
	}

	return (TRUE);
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
	
	handle = init_game();
	if (!handle) {
		return (1);
	}
	handle->player_info = player_info;

	network_setup(handle, flag, &handle->player_info, "127.0.0.1");
	chess_routine(handle);
	return (0);
}



void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[0];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY, opponent_pawn = EMPTY;
	
	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[1] - 1;
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game\n");
		return ;
	}
	
	/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
	tile_from = msg[1] - 1;
	tile_to = msg[2] - 1;
	piece_type = msg[3] - 1;
	
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
	handle->player_info.turn = TRUE;
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
	msg[1] = tile_from_or_color + 1;
	ft_printf_fd(1, "brut data in send: |%d||%d| ->", msg[0], msg[1]);
	/* If the message is a color message, return here */
	if (msg_type == MSG_TYPE_COLOR) {
		return (msg);
	}

	/* Set the tile_to (2nd data) */
	msg[2] = tile_to + 1;

	/* Set the piece_type (3rd data) */
	msg[3] = piece_type + 1;
	return (msg);
}


// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));
