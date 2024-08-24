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
	if (handle->player_info.nt_info) {
		send_disconnect_to_server(handle->player_info.nt_info->sockfd, handle->player_info.nt_info->servaddr);
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



void update_graphic_board(SDLHandle *h) {
	window_clear(h->renderer);
	draw_board(h, h->player_info.color);
	SDL_RenderPresent(h->renderer);
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
		update_graphic_board(handle);
	}

	/* Free memory */
	destroy_sdl_handle(handle);
	window_close(handle->window, handle->renderer);
	free(handle);
}


/**
 * @brief Move piece logic for network game
 * @param h 
 * @param tile_selected 
 * @param piece_type 
 * @return s32 FALSE if we can't send the message, CHESS_QUIT if the player quit the game, TRUE otherwise 
 */
s32 network_move_piece(SDLHandle *h, ChessTile tile_selected) {
	ChessBoard	*b = h->board;
	ChessPiece	piece_type = EMPTY;
	ChessPiece	color_piece_start = h->player_info.color == IS_WHITE ? WHITE_PAWN : BLACK_PAWN;
	ChessPiece	color_piece_end = h->player_info.color == IS_WHITE ? WHITE_KING : BLACK_KING;
	s32			ret = FALSE;
	s32 		nb_iter = 0;
	s8			send = FALSE;

	/* If a piece is selected and the tile selected is a possible move */
	if (is_selected_possible_move(b->possible_moves, tile_selected)) {
		piece_type = get_piece_from_tile(b, b->selected_tile);
		ret = move_piece(h, b->selected_tile, tile_selected, piece_type);
		b->possible_moves = 0;
		update_graphic_board(h);

		/* Send move message to the other player if is not pawn promotion or chess quit */
		if (ret == TRUE) {
			build_message(h->player_info.msg_tosend, MSG_TYPE_MOVE, b->selected_tile, tile_selected, piece_type);
		}

		while (send == FALSE) {
			send = chess_msg_send(h->player_info.nt_info, h->player_info.msg_tosend);
			nb_iter++;
			if (nb_iter > MAX_ITER) {
				ft_printf_fd(1, "Max iter reached\n");
				return (ret);
			}
			sleep(1);
		}
		h->player_info.turn = FALSE;
		return (ret);
	} else { /* Update piece possible move and selected tile */
		piece_type = get_piece_from_tile(b, tile_selected);
		if (piece_type >= color_piece_start && piece_type <= color_piece_end) {
			b->selected_tile = tile_selected;
			b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), piece_type, TRUE);
		} else {
			b->selected_tile = INVALID_TILE;
			b->possible_moves = 0;
		}
	}
	return (TRUE);
}

void network_chess_routine(SDLHandle *h) {
	ChessTile	tile_selected = INVALID_TILE;
	s32			ret = FALSE;
	s8			rcv_ret = FALSE;
	
	while (1) {
		tile_selected = event_handler(h->player_info.color);
		/* If the quit button is pressed */
		if (tile_selected == CHESS_QUIT) { break ; } // Send quit message to the other player
		
		/* If tile is selected and is player turn, try to send move piece and move it */
		if (h->player_info.turn == TRUE && tile_selected != INVALID_TILE) {
			ret = network_move_piece(h, tile_selected);
			if (ret == CHESS_QUIT || ret == FALSE) {
				break ;
			}
		}

		/* Receive message from the other player */
		if (!h->player_info.turn) {
			rcv_ret = chess_msg_receive(h->player_info.nt_info, h->player_info.msg_receiv, h->player_info.last_msg);
			if (rcv_ret) {
				process_message_receive(h, h->player_info.msg_receiv);
			}
		}

		/* Draw logic */
		update_graphic_board(h);
	}

	/* Free memory */
	destroy_sdl_handle(h);
	window_close(h->window, h->renderer);
	free(h);
}

int network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip) {
	struct timeval timeout = {0, 100000};
	int test_iter = 0;
	s8 ret = FALSE;

	player_info->nt_info = init_network(server_ip, player_info->running_port, timeout);
	if (has_flag(flag, FLAG_LISTEN)) {
		player_info->color = random_player_color();
		ft_printf_fd(1, "Waiting for player...\n");
		build_message(player_info->msg_tosend, MSG_TYPE_COLOR, !player_info->color, 0, 0);
		chess_msg_send(player_info->nt_info, player_info->msg_tosend);
	}

	if (has_flag(flag, FLAG_JOIN)) {
		while (ret == FALSE && test_iter < MAX_ITER) {
			ret = chess_msg_receive(player_info->nt_info, player_info->msg_receiv, player_info->last_msg);
			test_iter++;
			sleep(1);
		}
		display_message(player_info->msg_receiv);
		process_message_receive(handle, player_info->msg_receiv);
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

	if (has_flag(flag, FLAG_NETWORK)) {
		ft_printf_fd(1, ORANGE"Try to connect to Server at : %s:%d\n"RESET, player_info.dest_ip, SERVER_PORT);
		network_setup(handle, flag, &handle->player_info, player_info.dest_ip);
		network_chess_routine(handle);
	} else {
		chess_routine(handle);
	}
	return (0);
}



void update_msg_store(char *buffer, char *msg) {
	ft_bzero(buffer, 5);
	ft_strlcpy(buffer, msg, ft_strlen(msg));
}

void display_unknow_msg(char *msg) {
	int len = ft_strlen(msg);

	ft_printf_fd(1, RED"Unknown message -> |"RESET);

	for (int i = 0; i < len; i++) {
		ft_printf_fd(1, "%d ", msg[i]);
	}
	ft_printf_fd(1, "|\n");
}


void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[0];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY, opponent_pawn = EMPTY;
	
	// display_message(msg);

	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[1] - 1;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game\n");
	} else if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) {
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
	else {
		display_unknow_msg(msg);
		return ;
	}
	update_msg_store(handle->player_info.last_msg, msg);
}


void build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type) {
	// char *msg = ft_calloc(msg_size, sizeof(char));

	ft_bzero(msg, MSG_SIZE);

	/* Set the message type */
	msg[0] = (char)msg_type;
	if (msg_type == MSG_TYPE_QUIT) {
		return ;
	}

	/* Set the tile_from or color (1st data) */
	msg[1] = (char)(tile_from_or_color + 1);

	/* If the message is a color message, return here */
	if (msg_type == MSG_TYPE_COLOR) {
		return ;
	}

	/* Set the tile_to (2nd data) */
	msg[2] = (char)(tile_to + 1);

	/* Set the piece_type (3rd data) */
	msg[3] = (char)(piece_type + 1);
}


// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));



