#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_PORT 24242
#define TEST_MSG_NB 3
#define MAX_ATTEMPTS 10
#define TIMEOUT_SEC 2


int safe_udp_msg(int sockfd, struct sockaddr_in peeraddr, socklen_t addr_len, char *msg) {
	int attempts = 0;
	int ack_received = 0;
	char buffer[1024];

	bzero(buffer, 1024);
	
	while (attempts < MAX_ATTEMPTS && !ack_received) {
		sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&peeraddr, addr_len);
		// printf("Try to send : %s, nb %d\n", msg, attempts + 1);
		// Wait for ACK
		int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
		if (recv_len > 0) {
			buffer[recv_len] = '\0';
			if (strcmp(buffer, "ACK") == 0) {
				printf("ACK receive for: |%s|\n", msg);
				ack_received = 1;
			} else { // Other msg send ACK for it
				sendto(sockfd, "ACK", strlen("ACK"), 0, (struct sockaddr *)&peeraddr, addr_len);
				printf("Msg receive : |%s| -> Send ACK\n", buffer);
			}
		} 
		attempts++;
		sleep(1);
	}
	if (!ack_received) {
		printf("No ACK received give up msg %s\n", msg);
	}
}

typedef struct {
	struct sockaddr_in	localaddr;
	struct sockaddr_in	servaddr;
	struct sockaddr_in	peeraddr;
	socklen_t			addr_len;
	int					sockfd;
} NetworkInfo;

NetworkInfo *setup_client(int argc, char **argv) {
    NetworkInfo *info = calloc(sizeof(NetworkInfo),1);
	char server_ip[16];
    char buffer[1024];
    int local_port = atoi(argv[1]);

	if (!info) {
		return (NULL);
	}

    bzero(server_ip, 16);
    bzero(buffer, 1024);
    if (argc == 3) {
        strcpy(server_ip, argv[2]);
    } else {
        printf("Usage: %s <local_port> <server_ip>\n", argv[0]);
		free(info);
		return (NULL);
    }

	if ((info->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket creation failed");
		free(info);
		return (NULL);
	}

	printf("Server IP: %s, Local port : %d\n", server_ip, local_port);

	/* Bind the socket */
	memset(&info->localaddr, 0, sizeof(info->localaddr));
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
	memset(&info->servaddr, 0, sizeof(info->servaddr));
	info->servaddr.sin_family = AF_INET;
	info->servaddr.sin_port = htons(SERVER_PORT);
	info->servaddr.sin_addr.s_addr = inet_addr(server_ip);

	/* Send a message to the server */
	sendto(info->sockfd, "Hello", strlen("Hello"), 0, (struct sockaddr *)&info->servaddr, sizeof(info->servaddr));

	/* Receive the peer information */
	recvfrom(info->sockfd, &info->peeraddr, sizeof(info->peeraddr), 0, (struct sockaddr *)&info->servaddr, &info->addr_len);
	printf("Peer info : %s:%d\n", inet_ntoa(info->peeraddr.sin_addr), ntohs(info->peeraddr.sin_port));

	/* Configure timeout for socket */
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT_SEC;
	timeout.tv_usec = 0;

	if (setsockopt(info->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("Error setting socket timeout");
	}

	return (info);
}

int main(int argc, char **argv) {
	NetworkInfo *ctx = setup_client(argc, argv);
	char msg[16];

	bzero(msg, 16);
	if (!ctx) {
		printf("Error setting up client\n");
		return (1);
	}

    /* Send a few messages to the peer with sequence numbers */
    for (int i = 0; i < TEST_MSG_NB; i++) {
        sprintf(msg, "Ping %d", i);
		safe_udp_msg(ctx->sockfd, ctx->peeraddr, ctx->addr_len, msg);
        sleep(1);
    }

    /* Wait for Bye */
	safe_udp_msg(ctx->sockfd, ctx->peeraddr, ctx->addr_len, "Bye");

    close(ctx->sockfd);
    return 0;
}
