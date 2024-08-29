#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

t_list *array_to_list(MoveSave *arr, u16 size, u16 size_of_elem) {
	t_list *lst = NULL;
	MoveSave *move = NULL;

	for (int i = 0; i < size; i++) {
		move = ft_calloc(1, size_of_elem);
		ft_memcpy(move, &arr[i], size_of_elem);
		ft_lstadd_back(&lst, ft_lstnew(move));
	}
	return (lst);
}

void process_reconnect_message(SDLHandle *h, char *msg) {
	// ChessMoveList *move_list = h->board->lst;
	MoveSave *move_arr = NULL;
	u16 list_size = 0, array_byte_size = 0;
	u64 remaining_time = 0;

	/* Get the size of the move list */
	ft_memcpy(&list_size, &msg[5], sizeof(u16));
	ft_memcpy(&array_byte_size, &msg[7], sizeof(u16));
	ft_memcpy(&remaining_time, &msg[9 + array_byte_size], sizeof(u64));

	// printf("receive Size of the list: %d\n", list_size);
	// printf("receive Size of the list in byte: %d\n", array_byte_size);
	// printf("receive Remaining time: %lu\n", remaining_time);

	/* Transform the array in list */
	move_arr = (MoveSave *)&msg[9];
	for (int i = 0; i < list_size; i++) {
		// printf(ORANGE"Array Move Tile: %s to %s\n"RESET, TILE_TO_STRING(move_arr[i].tile_from), TILE_TO_STRING(move_arr[i].tile_to));
		// printf(ORANGE"Array Move Piece: %s to %s\n"RESET, chess_piece_to_string(move_arr[i].piece_from), chess_piece_to_string(move_arr[i].piece_to));
		move_piece(h, move_arr[i].tile_from, move_arr[i].tile_to, move_arr[i].piece_from);
	}

	/* Set the move list */
	h->board->lst = array_to_list(move_arr, list_size, sizeof(MoveSave));
	h->board->turn = msg[1];
	h->player_info.color = msg[2];
	h->my_remaining_time = remaining_time;
	h->enemy_remaining_time = remaining_time;

	ChessPiece piece = get_piece_from_tile(h->board, move_arr[list_size - 1].tile_to);
	s8 is_black_last = piece >= BLACK_PAWN;
	if (piece == EMPTY) {
		h->player_info.turn = !(h->player_info.color);
	} else {
		h->player_info.turn = !(is_black_last == h->player_info.color);
		// printf("Last piece: %s -> black_last: %d: color: %s\n", chess_piece_to_string(piece), is_black_last, h->player_info.color ? "BLACK" : "WHITE");
		// printf("Player Turn: %d\n", h->player_info.turn);
	}

	/* 5 * 0 for white, and 5 * 1 for black */
	h->player_info.piece_start = BLACK_PAWN * h->player_info.color;
	/* 5 * 0 for white, and 5 * 1 for black, + 5 */
	h->player_info.piece_end = BLACK_KING * h->player_info.color + 5;
}


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
	printf("In process Message type: %s\n", message_type_to_str(msg_type));

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
	} else if (msg_type == MSG_TYPE_RECONNECT)  {
		process_reconnect_message(handle, msg);
		update_msg_store(handle->player_info.last_msg, msg);
		return ;	
	} else {
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
 * - 4-5: msg_size: Len of the MESSAGE (u16)
 * - 5-6: list_size: Len of the move list (u16), is the number of element in the move list transmitted
 * - 6-7: list_byte_size: The len of the list is the list_size * sizeof(MoveSave) (u16)
 * - After we store the move list in array format to be send
 * - list_byte_size-(list_byte_size + 8): remaining_time (u64)
 */

char *build_reconnect_message(SDLHandle *h, u16 *msg_size) {
	ChessMoveList *move_list = h->board->lst;
	MoveSave *move_arr = NULL;

	/* Get the size of the move list */
	u16 list_size = (u16)ft_lstsize(move_list);
	printf("Move list size: %d\n", list_size);

	/* Get the size of the move list in byte */
	u16 array_byte_size = list_size * sizeof(MoveSave);

	printf("Array byte size: %d\n", array_byte_size);

	/* Transform the move list in array */
	move_arr = list_to_array(move_list, list_size, sizeof(MoveSave));

	printf("Sizeof move_arr %lu\n", sizeof(move_arr));

	// 3 byte for msg_type, turn and color
	// 4 byte for the size of the message and the size of the list
	// 2 byte for the size of the list in byte
	// size of array
	// 8 byte for the remaining time
	*msg_size = 3 + 4 + 2 + array_byte_size + 8;
	char *buff = ft_calloc(*msg_size, sizeof(char));
	buff[0] = MSG_TYPE_RECONNECT; // msg_type
	buff[1] = h->board->turn; // turn
	buff[2] = !h->player_info.color; // color
	ft_memcpy(&buff[3], msg_size, sizeof(u16)); // size of the message
	ft_memcpy(&buff[5], &list_size, sizeof(u16)); // size of the list
	ft_memcpy(&buff[7], &array_byte_size, sizeof(u16)); // size of the list in byte
	ft_memcpy(&buff[9], move_arr, array_byte_size); // move list
	ft_memcpy(&buff[9 + array_byte_size], &h->my_remaining_time, sizeof(u64)); // remaining time

	free(move_arr);

	move_arr = (MoveSave *)&buff[9];
	u64 time_remain = *(u64 *)&buff[9 + array_byte_size]; 
	printf("Affter copy time remain %lu\n", time_remain);
	printf("msg_size: %d, list_size: %d, array_byte_size: %d\n", *(u16 *)&buff[3], *(u16 *)&buff[5], *(u16 *)&buff[7]);
		
	for (int i = 0; i < list_size; i++) {
		printf(ORANGE"Array Move Tile: %s to %s\n"RESET, TILE_TO_STRING(move_arr[i].tile_from), TILE_TO_STRING(move_arr[i].tile_to));
		printf(ORANGE"Array Move Piece: %s to %s\n"RESET, chess_piece_to_string(move_arr[i].piece_from), chess_piece_to_string(move_arr[i].piece_to));
		printf(YELLOW"List Move Tile: %s %s\n"RESET, TILE_TO_STRING(((MoveSave *)(move_list->content))->tile_from), TILE_TO_STRING(((MoveSave *)(move_list->content))->tile_to));
		printf(YELLOW"List Move Piece: %s %s\n"RESET, chess_piece_to_string(((MoveSave *)(move_list->content))->piece_from), chess_piece_to_string(((MoveSave *)(move_list->content))->piece_to));
		move_list = move_list->next;
	}
	return (buff);
}

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
	char	buffer[4096];

	fast_bzero(buffer, 4096);
	rcv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->peeraddr, &info->addr_len);
	if (rcv_len > 0) {
		if (ignore_msg(h, buffer, last_msg_processed)) {
			return (FALSE);
		}
		buffer[rcv_len] = '\0';
		sendto(info->sockfd, ACK_STR, ACK_LEN, 0, (struct sockaddr *)&info->peeraddr, info->addr_len);
		CHESS_LOG(LOG_INFO, GREEN"Msg |%s| receive len : %lu -> ACK send\n"RESET, message_type_to_str(buffer[0]), rcv_len);
		ft_memcpy(rcv_buffer, buffer, rcv_len);
		return (TRUE);
	} 
	return (FALSE);
}

s8 chess_msg_send(NetworkInfo *info, char *msg, u16 msg_len) {
	ssize_t	rcv_len = 0;
	int		ack_received = 0, attempts = 0;
	char	buffer[1024];

	fast_bzero(buffer, 1024);
	CHESS_LOG(LOG_INFO, CYAN"Try to send %s len %u -> "RESET, message_type_to_str(msg[0]), msg_len);
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
		if (chess_msg_send(h->player_info.nt_info, h->player_info.msg_tosend, MSG_SIZE) == TRUE){
			return (TRUE);
		}
		nb_iter++;
		SDL_Delay(1000);
	}
	return (FALSE);
}