#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

#ifdef CHESS_WINDOWS_VERSION
int init_network_windows() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return 1;
    }
	return (0);
}

void cleanup_network_windows() {
    WSACleanup();
}
#else
int init_network_posix() {
    return (0); // No initialization needed for POSIX
}

void cleanup_network_posix() {
    // No cleanup needed for POSIX
}
#endif



static void player_color_set_info(PlayerInfo *info) {
	/* 1 for white, and 0 for black */
	info->turn = !(info->color);

	/* 5 * 0 for white, and 5 * 1 for black */
	info->piece_start = BLACK_PAWN * info->color;

	/* 5 * 0 for white, and 5 * 1 for black, + 5 */
	info->piece_end = BLACK_KING * info->color + 5;
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
	// struct timeval	timeout = {0, 500000}; /* 500000 microseconds = 0.5 seconds */
	struct timeval	timeout = {0, 10000}; /* 10000 microseconds = 0.01 seconds */
	s32				iter = 0;
	s8				ret = FALSE;

	player_info->nt_info = init_network(server_ip, timeout);
	if (has_flag(flag, FLAG_LISTEN)) {
		// player_info->color = random_player_color();
		player_info->color = IS_WHITE;
		CHESS_LOG(LOG_INFO, "Listen for player...%s", "\n");
		build_message(handle, player_info->msg_tosend, MSG_TYPE_COLOR, !player_info->color, 0, 0);
		chess_msg_send(player_info->nt_info, player_info->msg_tosend, MSG_SIZE);
	} else if (has_flag(flag, FLAG_JOIN)) {
		while (ret == FALSE && iter < MAX_ITER) {
			ret = chess_msg_receive(handle, player_info->nt_info, player_info->msg_receiv);
			iter++;
			SDL_Delay(1000);
		}
		/* Process message receive, here set color  */
		process_message_receive(handle, player_info->msg_receiv);
	} else if (has_flag(flag, FLAG_RECONNECT)) {
		char buffer[4096];
		fast_bzero(buffer, 4096);
		CHESS_LOG(LOG_INFO, "Reconnect to server, get game state\n");
		while (ret == FALSE && iter < MAX_ITER) {
			ret = chess_msg_receive(handle, player_info->nt_info, buffer);
			iter++;
			SDL_Delay(1000);
		}
		process_message_receive(handle, buffer);
		update_graphic_board(handle);
		return (TRUE);
	}
	player_color_set_info(player_info);

	update_graphic_board(handle);

	return (TRUE);
}

static s8 socket_no_block(NetworkInfo *info, struct timeval timeout) {
#ifdef CHESS_WINDOWS_VERSION
	/* Set the socket no block */
	u_long mode = 1;
	if (ioctlsocket(info->sockfd, FIONBIO, &mode) != 0) {
		perror("ioctlsocket failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (FALSE);
	}
#else
	/* Set the socket receive timeout */
	if (setsockopt(info->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		perror("Error setting socket timeout");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (FALSE);
	}
#endif
	return (TRUE);
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
	static const char magic_reco[MAGIC_SIZE] = MAGIC_RECONNECT_STR;
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
	if (ret == 16 + MAGIC_SIZE) {
		s8 check = check_reconnect_magic_value(buff);
		if (check == FALSE) {
			CHESS_LOG(LOG_ERROR, "Magic value reconnect check failed\n");
			return (FALSE);
		}
		info->peer_conected = TRUE;
		ft_memcpy(&info->peeraddr, buff + MAGIC_SIZE, sizeof(info->peeraddr));
		CHESS_LOG(LOG_INFO, "Peer info : %s:%d, addr_len %d\n", inet_ntoa(info->peeraddr.sin_addr), ntohs(info->peeraddr.sin_port), info->addr_len);
		return (TRUE);
	} 
	return (FALSE);
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
	if (socket_no_block(info, timeout) == FALSE) {
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

	/* Send a message to the server */
	sendto(info->sockfd, "Hello", fast_strlen("Hello"), 0, (struct sockaddr *)&info->servaddr, sizeof(info->servaddr));

	
	while (!wait_peer_info(info, "Wait peer info")) {
		SDL_Delay(1000);
	}
	return (info);
}

void send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr) {
	sendto(sockfd, DISCONNECT_MSG, fast_strlen(DISCONNECT_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void send_alive_to_server(int sockfd, struct sockaddr_in servaddr) {
	CHESS_LOG(LOG_INFO, ORANGE"Send alive message\n"RESET);
	sendto(sockfd, ALIVE_MSG, ALIVE_LEN, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}