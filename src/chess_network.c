#include "../include/network.h"
#include "../include/handle_sdl.h"

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
	// 500000 microseconds = 0.5 seconds
	struct timeval timeout = {0, 500000};
	s32 test_iter = 0;
	s8 ret = FALSE;

	handle->board->turn = 1;

	player_info->nt_info = init_network(server_ip, player_info->running_port, timeout);
	if (has_flag(flag, FLAG_LISTEN)) {
		player_info->color = random_player_color();
		ft_printf_fd(1, "Listen for player...%s", "\n");
		build_message(player_info->msg_tosend, MSG_TYPE_COLOR, !player_info->color, 0, 0, handle->board->turn);
		chess_msg_send(player_info->nt_info, player_info->msg_tosend);
	}

	if (has_flag(flag, FLAG_JOIN)) {
		while (ret == FALSE && test_iter < MAX_ITER) {
			ret = chess_msg_receive(handle, player_info->nt_info, player_info->msg_receiv, player_info->last_msg);
			test_iter++;
			sleep(1);
		}
		// display_message(player_info->msg_receiv);
		process_message_receive(handle, player_info->msg_receiv);
	}
	player_color_set_info(player_info);
	return (TRUE);
}

s8 socket_no_block(NetworkInfo *info, struct timeval timeout) {
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

NetworkInfo *init_network(char *server_ip, int local_port, struct timeval timeout) {
    NetworkInfo *info = NULL;
    char buffer[1024];
	ssize_t ret_rcv = 0;

    fast_bzero(buffer, 1024);

	if (INIT_NETWORK() != 0) {
		ft_printf_fd(2, "Error %s\n", __func__);
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
	}

	
	/* Bind the socket */
	(void)local_port; // Bind can be done on any port
	ft_memset(&info->localaddr, 0, sizeof(info->localaddr));
	info->localaddr.sin_family = AF_INET;
	info->localaddr.sin_addr.s_addr = INADDR_ANY;
	info->localaddr.sin_port = htons(0);
	if (bind(info->sockfd, (struct sockaddr *)&info->localaddr, sizeof(info->localaddr)) < 0) {
		perror("Bind failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (NULL);
	}

	/* Get the local port */
	info->addr_len = sizeof(info->localaddr);
	if (getsockname(info->sockfd, (struct sockaddr *)&info->localaddr, &info->addr_len) == -1) {
		perror("getsockname failed");
		CLOSE_SOCKET(info->sockfd);
		free(info);
		return (NULL);
	}

	ft_printf_fd(1, "Server IP: %s, server port %d, Local port : %d\n", server_ip, SERVER_PORT, ntohs(info->localaddr.sin_port));


	/* Server addr configuration */
	ft_memset(&info->servaddr, 0, sizeof(info->servaddr));
	info->servaddr.sin_family = AF_INET;
	info->servaddr.sin_port = htons(SERVER_PORT);
	info->servaddr.sin_addr.s_addr = inet_addr(server_ip);

	/* Send a message to the server */
	sendto(info->sockfd, "Hello", fast_strlen("Hello"), 0, (struct sockaddr *)&info->servaddr, sizeof(info->servaddr));

	ft_printf_fd(1, "Waiting %s...\n", "for peer info");
	/* Receive the peer information */
	while (ret_rcv <= 0) {
		ret_rcv = recvfrom(info->sockfd, (char *)&info->peeraddr, sizeof(info->peeraddr), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
		sleep(1);
	}
	
	// recvfrom(info->sockfd, (char *)&info->peeraddr, sizeof(info->peeraddr), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);

	ft_printf_fd(1, "Peer info : %s:%d, addr_len %d\n", inet_ntoa(info->peeraddr.sin_addr), ntohs(info->peeraddr.sin_port), info->addr_len);
	/* Send a first message to the peer (handshake) */
	sendto(info->sockfd, "Hello", fast_strlen("Hello"), 0, (struct sockaddr *)&info->peeraddr, info->addr_len);

	return (info);
}

void send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr) {
	sendto(sockfd, DISCONNECT_MSG, fast_strlen(DISCONNECT_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}