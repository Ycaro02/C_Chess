#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

void display_message(char *msg) {
	MsgType msg_type = msg[IDX_TYPE];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	CHESS_LOG(LOG_INFO, YELLOW"Message type: %s: "RESET, message_type_to_str(msg_type));

	if (msg_type == MSG_TYPE_COLOR) {
		CHESS_LOG(LOG_INFO, "turn: |%d| color |%d| -> ", msg[IDX_TURN], msg[IDX_FROM]);
		CHESS_LOG(LOG_INFO, "Color: %s\n", (msg[IDX_TURN] - 1) == IS_WHITE ? "WHITE" : "BLACK");
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, "Opponent quit the game, msg type %d\n", msg[0]);
		return ;
	}	
	
	/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
	tile_from = msg[IDX_FROM] - 1;
	tile_to = msg[IDX_TO] - 1;
	piece_type = msg[IDX_PIECE] - 1;
	CHESS_LOG(LOG_INFO, PURPLE"brut data: |%d||%d||%d| Timer:|%lu|\n"RESET, msg[IDX_FROM], msg[IDX_TO], msg[IDX_PIECE], *(u64 *)&msg[IDX_TIMER]);
	if (msg_type == MSG_TYPE_MOVE) {
		CHESS_LOG(LOG_INFO, ORANGE"Move from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));

	} else if (msg_type == MSG_TYPE_PROMOTION) {
		CHESS_LOG(LOG_INFO, ORANGE"Promotion from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} 
}

/* @brief Update the buffer with the message
 * @param buffer The buffer to update
 * @param msg The message to display
*/
void update_msg_store(char *buffer, char *msg) {
	fast_bzero(buffer, MSG_SIZE);
	// ft_strlcpy(buffer, msg, ft_strlen(msg));
	fast_strcpy(buffer, msg);
}

/* @brief Display the message
 * @param msg The message to display
*/
void display_unknow_msg(char *msg) {
	int len = fast_strlen(msg);

	printf(RED"Unknown message -> |%s",RESET);

	for (int i = 0; i < len; i++) {
		printf("%d ", msg[i]);
	}
	printf("%s", "|\n");
}


s8 is_legal_promotion_pck(SDLHandle *handle, ChessPiece new_piece, ChessTile tile_to) {
	s8 enemy_color = handle->player_info.color == IS_WHITE ? IS_BLACK : IS_WHITE;
	ChessPiece start_piece = enemy_color == IS_WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
	ChessPiece end_piece = enemy_color == IS_WHITE ? WHITE_QUEEN : BLACK_QUEEN;

	if (new_piece < start_piece || new_piece > end_piece) {
		CHESS_LOG(LOG_INFO, "Piece type promotion is out of range %s\n", chess_piece_to_string(new_piece));
		return (FALSE);
	}

	/* Is if a promotion we need to check te pawn reach the raw of the promotion */
	if (enemy_color == IS_WHITE && tile_to < A8 && tile_to > H8) {
		CHESS_LOG(LOG_INFO, "Tile to is not in the promotion raw %s\n", TILE_TO_STRING(tile_to));
		return (FALSE);
	} else if (enemy_color == IS_BLACK && tile_to < A1 && tile_to > H1) {
		CHESS_LOG(LOG_INFO, "Tile to is not in the promotion raw %s\n", TILE_TO_STRING(tile_to));
		return (FALSE);
	}

	return (TRUE);
}

s8 is_legal_move_pck(SDLHandle *handle, ChessTile tile_from, ChessTile tile_to, ChessPiece piece_type) {
	Bitboard	possible_moves = 0;
	s8			enemy_color = handle->player_info.color == IS_WHITE ? IS_BLACK : IS_WHITE;
	ChessPiece	enemy_piece_start = enemy_color == IS_WHITE ? WHITE_PAWN : BLACK_PAWN;
	ChessPiece	enemy_piece_end = enemy_color == IS_WHITE ? WHITE_KING : BLACK_KING;

	/* Check if the tile is out of bound */
	if (tile_from < A1 || tile_from > H8 || tile_to < A1 || tile_to > H8) {
		CHESS_LOG(LOG_INFO, "Tile from or to is out of bound %s %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to));
		return (FALSE);
	}

	/* Check if the piece is out of range */
	if (piece_type < enemy_piece_start || piece_type > enemy_piece_end) {
		CHESS_LOG(LOG_INFO, "Piece type is out of range %s\n", chess_piece_to_string(piece_type));
		return (FALSE);
	}

	/* Check if the piece is on the tile */
	if ((handle->board->piece[piece_type] & (1ULL << tile_from)) == 0) {
		CHESS_LOG(LOG_INFO, "Piece is %s not on the tile %s\n", chess_piece_to_string(piece_type), TILE_TO_STRING(tile_from));
		return (FALSE);
	}

	/* Check if the move is possible */
	possible_moves = get_piece_move(handle->board, 1ULL << tile_from, piece_type, TRUE);
	if ((possible_moves & (1ULL << tile_to)) == 0) {
		CHESS_LOG(LOG_INFO, "Move is not possible from %s to %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to));
		return (FALSE);
	}
	return (TRUE);
}

/* @brief Process the message receive
 * @param handle The SDLHandle pointer
 * @param msg The message to process
*/
void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[IDX_TYPE];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY, opponent_pawn = EMPTY;
	
	// display_message(msg);

	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[IDX_FROM] - 1;
	} else if (msg_type == MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, "%s\n", "Opponent quit the game");
		handle->player_info.nt_info->peer_conected = FALSE;
	} else if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) {
		/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
		tile_from = msg[IDX_FROM] - 1;
		tile_to = msg[IDX_TO] - 1;
		piece_type = msg[IDX_PIECE] - 1;
		
		if (msg_type == MSG_TYPE_MOVE) {
			/* If the message is a move, just call move piece */

			move_piece(handle, tile_from, tile_to, piece_type);
		}  else if (msg_type == MSG_TYPE_PROMOTION) {
			
			/* If the message is a promotion message, promote the pawn */
			opponent_pawn = handle->player_info.color == IS_BLACK ? WHITE_PAWN : BLACK_PAWN;
			
			/* Remove opponent the pawn */
			handle->board->piece[opponent_pawn] &= ~(1ULL << tile_from);

			/* Remove the piece if there is one on the tile */
			ChessPiece piece_to_remove = get_piece_from_tile(handle->board, tile_to);
			if (piece_to_remove != EMPTY) {
				handle->board->piece[piece_to_remove] &= ~(1ULL << tile_to);
			}

			/* Add the new piece */
			handle->board->piece[piece_type] |= (1ULL << tile_to);
			update_piece_state(handle->board);
		}
		handle->player_info.turn = TRUE;
		handle->enemy_remaining_time = *(u64 *)&msg[IDX_TIMER];
	} 
	else {
		display_unknow_msg(msg);
		return ;
	}

	// u64 rcv_rime =  *(u64 *)&msg[5];
	// s32 min = rcv_rime / 60;
	// s32 sec = rcv_rime % 60;
	// char buff[TIME_STR_SIZE] = {0};
	// snprintf(buff, TIME_STR_SIZE, "%02d:%02d", min, sec);
	// printf("Enemy rest time: %lu: %s\n", rcv_rime, buff);
	/* Update the turn counter */
	handle->board->turn += 1;
	update_msg_store(handle->player_info.last_msg, msg);
}

/*
 * Packet format: 5 char + 8 char (for u64) = 13 char
 * 
 * General structure:
 * - 1: msg_type
 * - 2: turn
 * 
 * Specific fields based on msg_type:
 * 
 * MSG_TYPE_COLOR:
 * - 3: color
 * 
 * MSG_TYPE_MOVE:
 * - 3: tile_from
 * - 4: tile_to
 * - 5: piece_type
 * - 6-13: remaining_time (u64)
 * 
 * MSG_TYPE_PROMOTION:
 * - 3: tile_from
 * - 4: tile_to
 * - 5: NEW_piece_type (QUEEN, ROOK, BISHOP, KNIGHT)
 * - 6-13: remaining_time (u64)
 * - @note: The piece type is the new piece type, not the pawn type (WHITE_PAWN, BLACK_PAWN)
 * 
 * MSG_TYPE_RECONNECT:
 * - 3: color
 * - 4-5: number of messages to represent the current board state (u16)
 * - 6-13: remaining_time (u64)
 * - @note: We use +1 to avoid sending 0, interpreted like '\0'
 */


/* @brief Build the message
 * @param msg The message to build
 * @param msg_type The message type
 * @param tile_from_or_color The tile from or the color
 * @param tile_to The tile to
 * @param piece_type The piece type
 * @note We use +1 to avoid sending 0, interpreted like '\0'
*/
void build_message(SDLHandle *h, char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type) {
	// char *msg = ft_calloc(msg_size, sizeof(char));

	fast_bzero(msg, MSG_SIZE);

	/* Set the elapsed time */
	ft_memcpy(&msg[IDX_TIMER], &h->my_remaining_time, sizeof(u64));

	/* Set the message type */
	msg[IDX_TYPE] = msg_type;


	/* Set the message turn counter */
	msg[IDX_TURN] = h->board->turn;

	/* If the message is a quit message, return here */
	if (msg_type == MSG_TYPE_QUIT) {
		return ;
	}

	/* Set the tile_from or color (1st data) */
	msg[IDX_FROM] = (tile_from_or_color + 1);

	/* If the message is a color message, return here */
	if (msg_type == MSG_TYPE_COLOR) {
		return ;
	}

	/* Set the tile_to (2nd data) */
	msg[IDX_TO] = (tile_to + 1);

	/* Set the piece_type (3rd data) */
	msg[IDX_PIECE] = (piece_type + 1);
}

#define ACK_STR "ACK"
#define HELLO_STR "Hello"
#define ACK_LEN 3
#define HELLO_LEN 5

/**
 * @brief Check if the packet is illegal
 * @param h The SDLHandle pointer
 * @param buffer The buffer
 * @param len The length of the buffer
 * @return s8 TRUE if the packet is illegal, FALSE otherwise
*/
s8 is_illegal_packet(SDLHandle *h, char *buffer, int len) {
	ChessTile	tile_from = INVALID_TILE, tile_to = INVALID_TILE;
	ChessPiece	piece_type = EMPTY, piece_check_legal = EMPTY;
	s8			msg_type = buffer[IDX_TYPE];
	s8			color = -1;
	
	(void)len;

	/* If the message is not a valid message type return here */
	if (msg_type < MSG_TYPE_COLOR || msg_type > MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, RED"Buffer message type is not valid %s\n", RESET);
		return (TRUE);
	}

	if (msg_type != MSG_TYPE_QUIT && fast_strcmp(buffer, h->player_info.last_msg) == 0) {
		return (TRUE);
	}

	/* If the message is COLOR and not the right size return here */
	if (msg_type == MSG_TYPE_COLOR) {
		color = buffer[IDX_FROM] - 1;
		if (color != IS_WHITE && color != IS_BLACK) {
			CHESS_LOG(LOG_INFO, RED"Buffer color is not WHITE or BLACK%s\n", RESET);
			return (TRUE);
		}
	}

	if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) {
		
		tile_from = buffer[IDX_FROM] - 1;
		tile_to = buffer[IDX_TO] - 1;
		piece_type = buffer[IDX_PIECE] - 1;

		piece_check_legal = piece_type;

		/* If the message is PROMOTION, we need to check for pawn move instead of piece receive */
		if (msg_type == MSG_TYPE_PROMOTION) {
			piece_check_legal = h->player_info.color == IS_WHITE ? BLACK_PAWN : WHITE_PAWN;
		}

		/* If the message is MOVE or PROMOTION and the data for move are not valid return here */
		if (is_legal_move_pck(h, tile_from, tile_to, piece_check_legal) == FALSE) {
			return (TRUE);
		}

		/* If the message is PROMOTION and the data are not valid return here */
		if (msg_type == MSG_TYPE_PROMOTION && is_legal_promotion_pck(h, piece_type, tile_to) == FALSE) {
			return (TRUE);
		}
	}
	return (FALSE);
}


/**
 * @brief Ignore the message
 * @param h The SDLHandle pointer
 * @param buffer The buffer
 * @param last_msg_processed The last message processed
 * @return s8 TRUE if the message is ignored, FALSE otherwise
*/
s8 ignore_msg(SDLHandle *h, char *buffer, char *last_msg_processed) {
	int			len = fast_strlen(buffer);

	/* If the message is not the right size return here */
	// if (len < 2) {
	// 	CHESS_LOG(LOG_INFO, RED"Buffer size is less than 2 %s\n", RESET);
	// 	return (TRUE);
	// }

	(void)last_msg_processed;
	/* If the message is a hello,ack message or same than last msg process return here */
	if (fast_strcmp(buffer, HELLO_STR) == 0 || fast_strcmp(buffer, ACK_STR) == 0) {
		// || fast_strcmp(buffer, last_msg_processed) == 0) {
		return (TRUE);
	}

	if (is_illegal_packet(h, buffer, len)) {
		return (TRUE);
	}
	return (FALSE);
}


s8 chess_msg_receive(SDLHandle *h, NetworkInfo *info, char *rcv_buffer, char *last_msg_processed) {
	ssize_t	rcv_len = 0;
	char	buffer[1024];

	fast_bzero(buffer, 1024);
	rcv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->peeraddr, &info->addr_len);
	if (rcv_len > 0) {
		if (ignore_msg(h, buffer, last_msg_processed)) {
			return (FALSE);
		}
		buffer[rcv_len] = '\0';
		sendto(info->sockfd, ACK_STR, ACK_LEN, 0, (struct sockaddr *)&info->peeraddr, info->addr_len);
		CHESS_LOG(LOG_INFO, GREEN"Msg |%s| receive -> ACK send\n"RESET, message_type_to_str(buffer[0]));
		ftlib_strcpy(rcv_buffer, buffer, rcv_len);
		return (TRUE);
	} 
	return (FALSE);
}

s8 chess_msg_send(NetworkInfo *info, char *msg) {
	ssize_t	rcv_len = 0;
	int		ack_received = 0, attempts = 0, msg_len = fast_strlen(msg);
	char	buffer[1024];

	fast_bzero(buffer, 1024);
	CHESS_LOG(LOG_INFO, CYAN"Try to send %s -> "RESET, message_type_to_str(msg[0]));
	while (attempts < MAX_ATTEMPTS && !ack_received) {
		sendto(info->sockfd, msg, msg_len, 0, (struct sockaddr *)&info->peeraddr, info->addr_len);
		rcv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->peeraddr, &info->addr_len);
		if (rcv_len > 0) {
			buffer[rcv_len] = '\0';
			if (fast_strcmp(buffer, ACK_STR) == 0) {
				CHESS_LOG(LOG_INFO, CYAN"ACK receive\n%s", RESET);
				ack_received = 1;
				break ;
			} 
		} 
		attempts++;
		SDL_Delay(1000);
	}
	if (!ack_received) {
		CHESS_LOG(LOG_ERROR, "No ACK received for |%s| after %d sending try\nVerify your network connection\n",message_type_to_str(msg[0]), MAX_ATTEMPTS);
		return (FALSE);
	}
	return (TRUE);
}

/* Need to refactore this to store last message unsend and resend it when needed */
s8 safe_msg_send(SDLHandle *h) {
	s32 		nb_iter = 0;

	while (nb_iter < MAX_ITER) {
		if (chess_msg_send(h->player_info.nt_info, h->player_info.msg_tosend) == TRUE){
			return (TRUE);
		}
		nb_iter++;
		SDL_Delay(1000);
	}
	return (FALSE);
}