#include "../include/network.h"
#include "../include/handle_sdl.h"

void display_message(char *msg) {
	MsgType msg_type = msg[0];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	ft_printf_fd(1, YELLOW"Message type: %s: "RESET, message_type_to_str(msg_type));

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
	ft_printf_fd(1, PURPLE"brut data: |%d||%d||%d||%d|\n"RESET, msg[0], msg[1], msg[2], msg[3]);
	if (msg_type == MSG_TYPE_MOVE) {
		ft_printf_fd(1, ORANGE"Move from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else if (msg_type == MSG_TYPE_PROMOTION) {
		ft_printf_fd(1, ORANGE"Promotion from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} 
}

/* @brief Update the buffer with the message
 * @param buffer The buffer to update
 * @param msg The message to display
*/
void update_msg_store(char *buffer, char *msg) {
	ft_bzero(buffer, 5);
	ft_strlcpy(buffer, msg, ft_strlen(msg));
}

/* @brief Display the message
 * @param msg The message to display
*/
void display_unknow_msg(char *msg) {
	int len = ft_strlen(msg);

	ft_printf_fd(1, RED"Unknown message -> |%s",RESET);

	for (int i = 0; i < len; i++) {
		ft_printf_fd(1, "%d ", msg[i]);
	}
	ft_printf_fd(1, "%s", "|\n");
}

/* @brief Process the message receive
 * @param handle The SDLHandle pointer
 * @param msg The message to process
*/
void process_message_receive(SDLHandle *handle, char *msg) {
	MsgType 	msg_type = msg[0];
	ChessTile	tile_from = 0, tile_to = 0;
	ChessPiece	piece_type = EMPTY, opponent_pawn = EMPTY;
	
	// display_message(msg);

	/* If the message is a color message, set the player color */
	if (msg_type == MSG_TYPE_COLOR) {
		handle->player_info.color = msg[1] - 1;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "%s\n", "Opponent quit the game");
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

/* Packet format 4 char
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

/* @brief Build the message
 * @param msg The message to build
 * @param msg_type The message type
 * @param tile_from_or_color The tile from or the color
 * @param tile_to The tile to
 * @param piece_type The piece type
 * @note We use +1 to avoid sending 0, interpreted like '\0'
*/
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

s8 chess_msg_receive(NetworkInfo *info, char *rcv_buffer, char *last_msg_processed) {
	int len = 0;
	char buffer[1024];

	ft_bzero(buffer, 1024);
	
	#ifdef CHESS_WINDOWS_VERSION
		ft_printf_fd(1, "win before to try receive %s", "Message");
	#endif
	
	len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->peeraddr, &info->addr_len);
	
	#ifdef CHESS_WINDOWS_VERSION
		ft_printf_fd(1, "win after to try receive %s: len %d", "Message", len);
	#endif
	if (len > 0) {
		if (ftlib_strcmp(buffer, "Hello") == 0 || ftlib_strcmp(buffer, "ACK") == 0) {
			ft_printf_fd(1, PURPLE"Hello OR ACK receive continue listening\n%s", RESET);
			return (FALSE);
		} else if (ftlib_strcmp(buffer, last_msg_processed) == 0) {
			ft_printf_fd(1, YELLOW"Double message receive skip it\n%s", RESET);
			return (FALSE);
		}
		buffer[len] = '\0';
		sendto(info->sockfd, "ACK", ft_strlen("ACK"), 0, (struct sockaddr *)&info->peeraddr, info->addr_len);
		ft_printf_fd(1, GREEN"Msg |%s| receive\n"RESET, message_type_to_str(buffer[0]));
		ftlib_strcpy(rcv_buffer, buffer, len);
		return (TRUE);
	} 
	return (FALSE);
}

s8 chess_msg_send(NetworkInfo *info, char *msg) {
	int attempts = 0;
	int ack_received = 0;
	char buffer[1024];

	ft_bzero(buffer, 1024);
	
	while (attempts < MAX_ATTEMPTS && !ack_received) {
		sendto(info->sockfd, msg, ft_strlen(msg), 0, (struct sockaddr *)&info->peeraddr, info->addr_len);
		
		#ifdef CHESS_WINDOWS_VERSION
			ft_printf_fd(1, "win send: %s\nMaybe block here in rcv ACK\n", message_type_to_str(msg[0]));
		#endif
		
		int recv_len = recvfrom(info->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&info->peeraddr, &info->addr_len);
		
		#ifdef CHESS_WINDOWS_VERSION
			ft_printf_fd(1, "After send: %s", msg);
		#endif
		
		if (recv_len > 0) {
			buffer[recv_len] = '\0';
			if (ftlib_strcmp(buffer, "ACK") == 0) {
				// ft_printf_fd(1, GREEN"ACK receive for msg: |%s|\n"RESET, msg);
				ft_printf_fd(1, CYAN"ACK |%s| receive\n"RESET, message_type_to_str(msg[0]));
				ack_received = 1;
			} 
		} 
		attempts++;
		sleep(1);
	}
	if (!ack_received) {
		ft_printf_fd(1, "No ACK received after 10 try give up msg %s\nVerify your network connection\n", message_type_to_str(msg[0]));
		return (FALSE);
	}
	return (TRUE);
}