#include "../include/network.h"
#include "../include/handle_sdl.h"

#ifdef CHESS_WINDOWS_VERSION
int init_network_windows() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void cleanup_network_windows() {
    WSACleanup();
}
#else
int init_network_posix() {
    return (TRUE); // No initialization needed for POSIX
}

void cleanup_network_posix() {
    // No cleanup needed for POSIX
}
#endif


/**
 * @brief Move piece logic for network game
 * @param h 
 * @param tile_selected 
 * @param piece_type 
 * @return s32 FALSE if we can't send the message, CHESS_QUIT if the player quit the game, TRUE otherwise 
 */
s32 network_move_piece(SDLHandle *h, ChessTile tile_selected) {
	ChessBoard	*b = h->board;
	s32			ret = FALSE;
	s32 		nb_iter = 0;
	s8			send = FALSE;

	/* If a piece is selected and the tile selected is a possible move */
	if (is_selected_possible_move(b->possible_moves, tile_selected)) {
		// piece_type = get_piece_from_tile(b, b->selected_tile);
		ret = move_piece(h, b->selected_tile, tile_selected, b->selected_piece);
		b->possible_moves = 0;
		h->over_piece_select = EMPTY;
		
		update_graphic_board(h);
		/* Send move message to the other player if is not pawn promotion or chess quit */
		if (ret == TRUE) {
			build_message(h->player_info.msg_tosend, MSG_TYPE_MOVE, b->selected_tile, tile_selected, b->selected_piece);
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
		reset_selected_tile(h);
		return (ret);
	} else { /* Update piece possible move and selected tile */
		if (h->over_piece_select != EMPTY) {
			b->selected_piece = get_piece_from_tile(b, tile_selected);
			if (b->selected_piece >= h->player_info.piece_start && b->selected_piece <= h->player_info.piece_end) {
				b->selected_tile = tile_selected;
				b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), b->selected_piece, TRUE);
				if (b->possible_moves == 0) {
					h->over_piece_select = EMPTY;
				}
				return (TRUE);
			} 
			reset_selected_tile(h);
			return (TRUE);
		} 
		reset_selected_tile(h);
	}
	return (TRUE);
}

/**
 * @brief Network chess routine
 * @param h The SDLHandle pointer
 */
void network_chess_routine(SDLHandle *h) {
	ChessTile	tile_selected = INVALID_TILE;
	s32			ret = FALSE;
	s8			rcv_ret = FALSE;
	
	while (1) {
		tile_selected = event_handler(h, h->player_info.color);
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
}

void player_color_set_info(PlayerInfo *info) {
	// ft_printf_fd(1, YELLOW"Player color: %s\n"RESET, info->color == IS_WHITE ? "WHITE" : "BLACK");
	if (info->color == IS_WHITE) {
		info->turn = TRUE;
		info->piece_start = WHITE_PAWN;
		info->piece_end = WHITE_KING;

	} else {
		info->turn = FALSE;
		info->piece_start = BLACK_PAWN;
		info->piece_end = BLACK_KING;
	}
}

/**
 * @brief Network setup
 * @param handle The SDLHandle pointer
 * @param flag The flag
 * @param player_info The player info
 * @param server_ip The server ip
 * @return s8 TRUE if the network is setup, FALSE otherwise
 */
s8 network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip) {
	struct timeval timeout = {0, 100000};
	s32 test_iter = 0;
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
	player_color_set_info(player_info);
	return (TRUE);
}

NetworkInfo *init_network(char *server_ip, int local_port, struct timeval timeout) {
    NetworkInfo *info = NULL;
    char buffer[1024];

	if (!INIT_NETWORK()) {
		ft_printf_fd(1, "Network initialization failed\n");
		return (NULL);
	}

	if (!(info = ft_calloc(sizeof(NetworkInfo), 1))) {
		return (NULL);
	}

    ft_bzero(buffer, 1024);

	if ((info->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket creation failed");
		free(info);
		return (NULL);
	}

	ft_printf_fd(1, "Server IP: %s, server port %d, Local port : %d\n", server_ip, SERVER_PORT, local_port);

	/* Bind the socket */
	ft_memset(&info->localaddr, 0, sizeof(info->localaddr));
	info->localaddr.sin_family = AF_INET;
	info->localaddr.sin_addr.s_addr = INADDR_ANY;
	info->localaddr.sin_port = htons(local_port);
	if (bind(info->sockfd, (struct sockaddr *)&info->localaddr, sizeof(info->localaddr)) < 0) {
		perror("Bind failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (NULL);
	}

	/* Server addr configuration */
	ft_memset(&info->servaddr, 0, sizeof(info->servaddr));
	info->servaddr.sin_family = AF_INET;
	info->servaddr.sin_port = htons(SERVER_PORT);
	info->servaddr.sin_addr.s_addr = inet_addr(server_ip);

	/* Send a message to the server */
	sendto(info->sockfd, "Hello", ft_strlen("Hello"), 0, (struct sockaddr *)&info->servaddr, sizeof(info->servaddr));

	/* Receive the peer information */
	recvfrom(info->sockfd, &info->peeraddr, sizeof(info->peeraddr), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
	ft_printf_fd(1, "Peer info : %s:%d\n", inet_ntoa(info->peeraddr.sin_addr), ntohs(info->peeraddr.sin_port));
	/* Send a first message to the peer (handshake) */
	sendto(info->sockfd, "Hello", ft_strlen("Hello"), 0, (struct sockaddr *)&info->peeraddr, info->addr_len);

	if (setsockopt(info->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("Error setting socket timeout");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (NULL);
	}
	return (info);
}

void send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr) {
	sendto(sockfd, DISCONNECT_MSG, ft_strlen(DISCONNECT_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}