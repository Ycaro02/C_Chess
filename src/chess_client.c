#include "../include/chess.h"
#include "../include/network.h"


char *chess_msg_receive(int sockfd, struct sockaddr_in peeraddr, socklen_t addr_len) {
	int len = 0;
	char buffer[1024];
	char *msg = NULL;

	ft_bzero(buffer, 1024);
	
	len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
	if (len > 0) {
		buffer[len] = '\0';
		sendto(sockfd, "ACK", ft_strlen("ACK"), 0, (struct sockaddr *)&peeraddr, addr_len);
		ft_printf_fd(1, YELLOW"Chess Msg receive : |%s| -> Send ACK\n"RESET, buffer);
		msg = ft_calloc(sizeof(char), (len + 1));
		ftlib_strcpy(msg, buffer, len);
	} 
	return (msg);
}

int chess_msg_send(int sockfd, struct sockaddr_in peeraddr, socklen_t addr_len, char *msg) {
	int attempts = 0;
	int ack_received = 0;
	char buffer[1024];
	int ret = TRUE;

	ft_bzero(buffer, 1024);
	
	while (attempts < MAX_ATTEMPTS && !ack_received) {
		sendto(sockfd, msg, ft_strlen(msg), 0, (struct sockaddr *)&peeraddr, addr_len);
		int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
		if (recv_len > 0) {
			buffer[recv_len] = '\0';
			if (ftlib_strcmp(buffer, "ACK") == 0) {
				ft_printf_fd(1, GREEN"ACK receive for msg: |%s|\n"RESET, msg);
				ack_received = 1;
			} 
		} 
		attempts++;
		sleep(1);
	}
	if (!ack_received) {
		ft_printf_fd(1, "No ACK received after 10 try give up msg %s\nVerify your network connection\n", msg);
		ret = FALSE;
	}
	free(msg);
	return (ret);
}


NetworkInfo *init_network(char *server_ip, int local_port, struct timeval timeout) {
    NetworkInfo *info = ft_calloc(sizeof(NetworkInfo),1);
    char buffer[1024];

	if (!info) {
		return (NULL);
	}

    ft_bzero(buffer, 1024);

	if ((info->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket creation failed");
		free(info);
		return (NULL);
	}

	ft_printf_fd(1, "Server IP: %s, Local port : %d\n", server_ip, local_port);

	/* Bind the socket */
	ft_memset(&info->localaddr, 0, sizeof(info->localaddr));
	info->localaddr.sin_family = AF_INET;
	info->localaddr.sin_addr.s_addr = INADDR_ANY;
	info->localaddr.sin_port = htons(local_port);
	if (bind(info->sockfd, (struct sockaddr *)&info->localaddr, sizeof(info->localaddr)) < 0) {
		perror("Bind failed");
		close(info->sockfd);
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

	/* Configure timeout for socket */
	// timeout.tv_sec = TIMEOUT_SEC;
	// timeout.tv_usec = 0;

	/* Send a first message to the peer (handshake) */
	sendto(info->sockfd, "Hello", ft_strlen("Hello"), 0, (struct sockaddr *)&info->peeraddr, info->addr_len);

	if (setsockopt(info->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("Error setting socket timeout");
	}
	return (info);
}

void send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr) {
	sendto(sockfd, DISCONNECT_MSG, ft_strlen(DISCONNECT_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}