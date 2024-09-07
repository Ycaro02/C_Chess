#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


static void player_color_set_info(PlayerInfo *info) {
	/* 1 for white, and 0 for black */
	info->turn = !(info->color);
	/* 5 * 0 for white, and 5 * 1 for black */
	info->piece_start = BLACK_PAWN * info->color;
	/* 5 * 0 for white, and 5 * 1 for black, + 5 */
	info->piece_end = BLACK_KING * info->color + 5;
}

void wait_message_receive(SDLHandle *h, char *buff) {
	s8				ret = FALSE;

	while (ret == FALSE) {
		ret = chess_msg_receive(h, h->player_info.nt_info, buff);
		update_graphic_board(h);
		SDL_Delay(1000);
	}
}

void handle_network_client_state(SDLHandle *handle, u32 flag, PlayerInfo *player_info) {
	char			buff[4096];

	fast_bzero(buff, 4096);
	if (has_flag(flag, FLAG_LISTEN)) {
		// player_info->color = IS_WHITE;
		player_info->color = random_player_color();
		CHESS_LOG(LOG_INFO, "Listen for player...%s", "\n");
		build_message(handle, player_info->msg_tosend, MSG_TYPE_COLOR, !player_info->color, 0, 0);
		chess_msg_send(player_info->nt_info, player_info->msg_tosend, MSG_SIZE);
	} else if (has_flag(flag, FLAG_JOIN)) {
		wait_message_receive(handle, buff);
		/* Process message receive, here set color  */
		process_message_receive(handle, buff);
	} else if (has_flag(flag, FLAG_RECONNECT)) {
		CHESS_LOG(LOG_INFO, "Reconnect to server, get game state\n");
		wait_message_receive(handle, buff);
		process_message_receive(handle, buff);
		update_graphic_board(handle);
		return ;
	}
	player_color_set_info(player_info);
	update_graphic_board(handle);
}


static s8 local_socket_setup(NetworkInfo *info) {
	/* Bind the socket */
	ft_memset(&info->localaddr, 0, sizeof(info->localaddr));
	info->localaddr.sin_family = AF_INET;
	info->localaddr.sin_addr.s_addr = INADDR_ANY;
	info->localaddr.sin_port = htons(0);
	if (bind(info->sockfd, (struct sockaddr *)&info->localaddr, sizeof(info->localaddr)) < 0) {
		perror("Bind failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (FALSE);
	}

	/* Get the local port */
	info->addr_len = sizeof(info->localaddr);
	if (getsockname(info->sockfd, (struct sockaddr *)&info->localaddr, &info->addr_len) == -1) {
		perror("getsockname failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (FALSE);
	}
	return (TRUE);
}

s8 check_magic_value(char *buff) {
	static const char magic[MAGIC_SIZE] = MAGIC_STRING;
	u64 i = 0;
	
	while (i < MAGIC_SIZE) {
		if (buff[i] != magic[i]) {
			// CHESS_LOG(LOG_INFO, "Magic value check failed buff |%d|, magic|%d| idx %llu\n", buff[i], magic[i], i);
			return (FALSE);
		}
		i++;
	}
	return (TRUE);
}

s8 check_reconnect_magic_value(char *buff) {
	static const char magic_reco[MAGIC_SIZE] = MAGIC_CONNECT_STR;
	u64 i = 0;
	
	while (i < MAGIC_SIZE) {
		if (buff[i] != magic_reco[i]) {
			// CHESS_LOG(LOG_INFO, "Magic value check failed buff |%d|, magic|%d| idx %llu\n", buff[i], magic[i], i);
			return (FALSE);
		}
		i++;
	}
	return (TRUE);
}

s8 wait_peer_info(NetworkInfo *info, const char *msg) {
	ssize_t ret = 0;

	// CHESS_LOG(LOG_INFO, "%s...\n", msg);
	(void)msg;

	char buff[1024];
	fast_bzero(buff, 1024);
	
	send_alive_packet(info);

	/* Receive the peer information */
	ret = recvfrom(info->sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
	if (ret == CONNECT_PACKET_SIZE) {
		s8 check = check_reconnect_magic_value(buff);
		if (check == FALSE) {
			CHESS_LOG(LOG_ERROR, "Magic value reconnect check failed\n");
			return (FALSE);
		}
		info->peer_conected = TRUE;
		ft_memcpy(&info->peeraddr, buff + MAGIC_SIZE, sizeof(info->peeraddr));
		info->client_state = buff[CONNECT_PACKET_SIZE - 1];
		CHESS_LOG(LOG_INFO, "Client state: %s: Peer info : %s:%d\n"\
			, ClientState_to_str(info->client_state)
			, inet_ntoa(info->peeraddr.sin_addr)
			, ntohs(info->peeraddr.sin_port));
		return (TRUE);
	} 
	// else if (ret > 0) {
		// printf("Received %ld bytes\n", ret);
		// for (ssize_t i = 0; i < ret; i++) {
			// printf("%d ", buff[i]);
		// }
	// }
	return (FALSE);
}

void wait_for_player(SDLHandle *h) {
	s32 event = 0;

	while (!wait_peer_info(h->player_info.nt_info, "Wait for player")) {
		event = event_handler(h, h->player_info.color);
		if (!has_flag(h->flag, FLAG_NETWORK)) {
			break;
		} else if (event == CHESS_QUIT) {
			chess_destroy(h);
			// break;
		}
		update_graphic_board(h);
		SDL_Delay(100);
	}
}

void destroy_network_info(SDLHandle *h) {
	if (h->player_info.nt_info) {
		CHESS_LOG(LOG_INFO, ORANGE"Send disconnect to server%s\n", RESET);
		send_disconnect_to_server(h->player_info.nt_info->sockfd, h->player_info.nt_info->servaddr);
		close(h->player_info.nt_info->sockfd);
		free(h->player_info.nt_info);
		h->player_info.nt_info = NULL;
	}
}


NetworkInfo *init_network(char *server_ip, struct timeval timeout) {
    NetworkInfo *info = NULL;
    char buffer[1024];

    fast_bzero(buffer, 1024);
	if (INIT_NETWORK() != 0) {
		CHESS_LOG(LOG_ERROR, "%s\n", __func__);
		return (NULL);
	}

	if (!(info = ft_calloc(sizeof(NetworkInfo), 1))) {
		return (NULL);
	}

	/* Create the socket */
	if ((info->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket creation failed");
		free(info);
		return (NULL);
	}

	/* Set the socket no block */
	if (SOCKET_NO_BLOCK(info->sockfd, timeout) == FALSE) {
		free(info);
		return (NULL);
	} else if (!local_socket_setup(info)) {
		return (NULL);
	}

	CHESS_LOG(LOG_INFO, "Server IP: %s, server port %d, Local port : %d\n", server_ip, SERVER_PORT, ntohs(info->localaddr.sin_port));


	/* Server addr configuration */
	ft_memset(&info->servaddr, 0, sizeof(info->servaddr));
	info->servaddr.sin_family = AF_INET;
	info->servaddr.sin_port = htons(SERVER_PORT);
	info->servaddr.sin_addr.s_addr = inet_addr(server_ip);

	/* Send Hello to the server */
	sendto(info->sockfd, "Hello", fast_strlen("Hello"), 0, (struct sockaddr *)&info->servaddr, sizeof(info->servaddr));

	// while (!wait_peer_info(info, "Wait peer info")) {
	// 	SDL_Delay(1000);
	// }
	return (info);
}

void send_game_end_to_server(int sockfd, struct sockaddr_in servaddr) {
	CHESS_LOG(LOG_INFO, ORANGE"Send game end message\n"RESET);
	sendto(sockfd, GAME_END_MSG, strlen(GAME_END_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr) {
	sendto(sockfd, DISCONNECT_MSG, fast_strlen(DISCONNECT_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void send_alive_to_server(int sockfd, struct sockaddr_in servaddr) {
	// CHESS_LOG(LOG_DEBUG, ORANGE"Send alive message\n"RESET);
	sendto(sockfd, ALIVE_MSG, ALIVE_LEN, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}