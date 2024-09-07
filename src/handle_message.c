#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

/*
 * Potential struct :
 * typedef struct {
 * 		u8 msg_type;	MSG_TYPE_COLOR, MSG_TYPE_MOVE, MSG_TYPE_PROMOTION, MSG_TYPE_RECONNECT
 * 		u16 msg_id;		The message id
 * 		u8 tile_from;	The tile from
 * 		u8 tile_to;		The tile to
 * 		u8 piece_type;	The piece type
 * 		u64 remaining_time;	The remaining time
 * } Message;
 * Total size 1 + 2 + 1 + 1 + 1 + 8 = 14: MSG_SIZE is set to 16
 * 
 * General structure:
 * - 1	:	msg_type
 * - 2-3:	msg_id (u16)
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
 * MSG_TYPE_RECONNECT: Special message to reconnect to the server containing all the move list
 * - 3: color
 * - 4-5: msg_size: Len of the MESSAGE (u16)
 * - 5-6: list_size: Len of the move list (u16), is the number of element in the move list transmitted
 * - 6-7: list_byte_size: The len of the list is the list_size * sizeof(MoveSave) (u16)
 * 	- After this we store the move list in array format to be send
 * - list_byte_size-(list_byte_size + 8): enemy_remaining_time (u64)
 * - list_byte_size + 8 - end: my_remaining_time (u64)
 */


/* @brief Build the message
 * @param msg The message to build
 * @param msg_type The message type
 * @param tile_from_or_color The tile from or the color
 * @param tile_to The tile to
 * @param piece_type The piece type
*/
void build_message(SDLHandle *h, char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type) {


	fast_bzero(msg, MSG_SIZE);

	/* Set the elapsed time */
	ft_memcpy(&msg[IDX_TIMER], &h->my_remaining_time, sizeof(u64));

	/* Set the message type */
	msg[IDX_TYPE] = msg_type;


	/* Set the message id counter */
	// msg[IDX_TURN] = (u8)h->msg_id;
	ft_memcpy(&msg[IDX_MSG_ID], &h->msg_id, sizeof(u16));

	CHESS_LOG(LOG_INFO, PURPLE"Build: %s ID: %d\n"RESET, MsgType_to_str(msg_type), GET_MESSAGE_ID(msg));

	/* If the message is a quit message, return here */
	if (msg_type == MSG_TYPE_QUIT) {
		return ;
	}

	/* Set the tile_from or color (1st data) */
	msg[IDX_FROM] = (tile_from_or_color);

	/* If the message is a color message, return here */
	if (msg_type == MSG_TYPE_COLOR) {
		msg[IDX_MSG_ID] = 0;
		msg[IDX_MSG_ID + 1] = 0;
		return ;
	}

	/* Set the tile_to (2nd data) */
	msg[IDX_TO] = (tile_to);

	/* Set the piece_type (3rd data) */
	msg[IDX_PIECE] = (piece_type);
}


void display_message(char *msg) {
	MsgType msg_type = msg[IDX_TYPE];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	CHESS_LOG(LOG_INFO, YELLOW"Message type: %s: "RESET, MsgType_to_str(msg_type));

	if (msg_type == MSG_TYPE_COLOR) {
		CHESS_LOG(LOG_INFO, "ID: |%d| color |%d| -> ", GET_MESSAGE_ID(msg), msg[IDX_FROM]);
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, "Opponent quit the game, msg type %d\n", msg[0]);
		return ;
	}	
	
	/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
	tile_from = msg[IDX_FROM];
	tile_to = msg[IDX_TO];
	piece_type = msg[IDX_PIECE];
	// CHESS_LOG(LOG_INFO, PURPLE"brut data: |%d||%d||%d| Timer:|%lu|\n"RESET, msg[IDX_FROM], msg[IDX_TO], msg[IDX_PIECE], *(u64 *)&msg[IDX_TIMER]);
	if (msg_type == MSG_TYPE_MOVE) {
		CHESS_LOG(LOG_INFO, ORANGE"Move from %s to %s with piece %s\n"RESET, ChessTile_to_str(tile_from), ChessTile_to_str(tile_to), ChessPiece_to_str(piece_type));

	} else if (msg_type == MSG_TYPE_PROMOTION) {
		CHESS_LOG(LOG_INFO, ORANGE"Promotion from %s to %s with piece %s\n"RESET, ChessTile_to_str(tile_from), ChessTile_to_str(tile_to), ChessPiece_to_str(piece_type));
	} 
}

/* @brief Update the buffer with the message
 * @param buffer The buffer to update
 * @param msg The message to display
*/
void update_msg_store(char *buffer, char *msg) {
	fast_bzero(buffer, MSG_SIZE);
	// fast_strcpy(buffer, msg);
	ft_memcpy(buffer, msg, MSG_SIZE);
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


/* @brief Process the message receive
 * @param handle The SDLHandle pointer
 * @param msg The message to process
*/
void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[IDX_TYPE];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY;
	
	CHESS_LOG(LOG_INFO, YELLOW"Process: %s ID: %d\n"RESET, MsgType_to_str(msg_type), GET_MESSAGE_ID(msg));

	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[IDX_FROM];
	} else if (msg_type == MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, "%s\n", RED"Opponent quit the game"RESET);
		handle->player_info.nt_info->peer_conected = FALSE;
	} else if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) {
		/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
		tile_from = msg[IDX_FROM];
		tile_to = msg[IDX_TO];
		piece_type = msg[IDX_PIECE];
		
		/* If the message is a move, just call move piece */
		if (msg_type == MSG_TYPE_MOVE) {
			move_piece(handle, tile_from, tile_to, piece_type);
		}  else if (msg_type == MSG_TYPE_PROMOTION) {
			do_promotion_move(handle, tile_from, tile_to, piece_type, TRUE);			
		}
		handle->player_info.turn = TRUE;
		handle->enemy_remaining_time = *(u64 *)&msg[IDX_TIMER];
	} else if (msg_type == MSG_TYPE_RECONNECT)  {
		process_reconnect_message(handle, msg);
		update_msg_store(handle->player_info.last_msg, msg);
	} else {
		display_unknow_msg(msg);
		return ;
	}
	if (msg_type != MSG_TYPE_QUIT) {
		handle->msg_id = (GET_MESSAGE_ID(msg)) + 1;
	}
	// CHESS_LOG(LOG_INFO, ORANGE"MESSAGE ID: %hu\n"RESET, handle->msg_id);
	if (msg_type >= MSG_TYPE_COLOR && msg_type <= MSG_TYPE_PROMOTION) {
		update_msg_store(handle->player_info.last_msg, msg);
	}
}


s8 chess_msg_receive(SDLHandle *h, NetworkInfo *info, char *rcv_buffer) {
	ssize_t	rcv_len = 0;
	char	buffer[4096];

	// (void)last_msg_processed;
	fast_bzero(buffer, 4096);
	rcv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
	if (rcv_len > 0) {
		if (check_magic_value(buffer) == FALSE || ignore_msg(h, buffer + MAGIC_SIZE)) {
			return (FALSE);
		}
		buffer[rcv_len] = '\0';
		sendto(info->sockfd, ACK_STR, ACK_LEN, 0, (struct sockaddr *)&info->servaddr, info->addr_len);
		CHESS_LOG(LOG_INFO, GREEN"Msg |%s| receive len : %zd -> ACK send\n"RESET, MsgType_to_str(buffer[0 + MAGIC_SIZE]), rcv_len);
		ft_memcpy(rcv_buffer, buffer + MAGIC_SIZE, rcv_len - MAGIC_SIZE);
		return (TRUE);
	} 
	return (FALSE);
}

s8 chess_msg_send(NetworkInfo *info, char *msg, u16 msg_len) {
	ssize_t	rcv_len = 0;
	int		ack_received = 0, attempts = 0;
	char	buffer[4096];

	fast_bzero(buffer, 4096);
	CHESS_LOG(LOG_INFO, CYAN"Try to send %s len %u -> "RESET, MsgType_to_str(msg[0]), msg_len);
	while (attempts < MAX_ATTEMPTS && !ack_received) {
		sendto(info->sockfd, msg, msg_len, 0, (struct sockaddr *)&info->servaddr, info->addr_len);
		rcv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
		if (rcv_len > 0) {
			buffer[rcv_len] = '\0';
			if (check_magic_value(buffer) == TRUE && fast_strcmp(buffer + MAGIC_SIZE, ACK_STR) == 0) {
				CHESS_LOG(LOG_INFO, CYAN"ACK receive\n%s", RESET);
				ack_received = 1;
				break ;
			}
		} 
		attempts++;
		SDL_Delay(1000);
	}
	if (!ack_received) {
		CHESS_LOG(LOG_ERROR, "No ACK received for |%s| after %d sending try\nVerify your network connection\n",MsgType_to_str(msg[0]), MAX_ATTEMPTS);
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