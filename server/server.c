#include <stdio.h>
#include <arpa/inet.h>

#include "../include/chess.h"

#define PORT 24242
#define DISCONNECT_MSG "DISCONNECT"

typedef struct s_client_info {
    struct sockaddr_in addr;
    int connected;
} ClientInfo;

typedef struct s_chess_room {
	ClientInfo clientA;
	ClientInfo clientB;
} ChessRoom;

typedef struct s_chess_server {
	int sockfd;
	struct sockaddr_in addr;
	ChessRoom room;
} ChessServer;

void handle_client_message(int sockfd, ClientInfo *clientA, ClientInfo *clientB, struct sockaddr_in *cliaddr, char *buffer) {
    if (fast_strcmp(buffer, DISCONNECT_MSG) == 0) {
		if (clientA->connected && ft_memcmp(cliaddr, &clientA->addr, sizeof(struct sockaddr_in)) == 0) {
            clientA->connected = 0;
            ft_printf_fd(1, RED"Client A disconnected: %s:%d\n"RESET, inet_ntoa(clientA->addr.sin_addr), ntohs(clientA->addr.sin_port));
        } else if (clientB->connected && ft_memcmp(cliaddr, &clientB->addr, sizeof(struct sockaddr_in)) == 0) {
            clientB->connected = 0;
            ft_printf_fd(1, RED"Client B disconnected: %s:%d\n"RESET, inet_ntoa(clientB->addr.sin_addr), ntohs(clientB->addr.sin_port));
        }
		return ;
    }

	if (clientA->connected && clientB->connected) {
		ft_printf_fd(1, YELLOW"Already 2 clients are connected, need to implement multiroom handling\n"RESET);
		return ;
	}


	if (!clientA->connected) {
		clientA->addr = *cliaddr;
		clientA->connected = 1;
		ft_printf_fd(1, GREEN"Client A connected: %s:%d\n"RESET, inet_ntoa(clientA->addr.sin_addr), ntohs(clientA->addr.sin_port));
	} else if (!clientB->connected) {
		clientB->addr = *cliaddr;
		clientB->connected = 1;
		ft_printf_fd(1, GREEN"Client B connected: %s:%d\n"RESET, inet_ntoa(clientB->addr.sin_addr), ntohs(clientB->addr.sin_port));
	}

	if (clientA->connected && clientB->connected) {
		/* Send information from B to A */
		sendto(sockfd, &clientB->addr, sizeof(clientB->addr), 0, (struct sockaddr *)&clientA->addr, sizeof(clientA->addr));
		/* Send information from A to B */
		sendto(sockfd, &clientA->addr, sizeof(clientA->addr), 0, (struct sockaddr *)&clientB->addr, sizeof(clientB->addr));
	}
}

ChessServer *server_setup() {
	ChessServer *server = ft_calloc(1, sizeof(ChessServer));
	if (!server) {
		ft_printf_fd(2, RED"Error: alloc %s\n"RESET, __func__);
		return (NULL);
	}

	fast_bzero(&server->room, sizeof(ChessRoom));

    /* Create UDP socket */
    if ((server->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
		free(server);
		return (NULL);
    }

    ft_memset(&server->addr, 0, sizeof(server->addr));
	/* Server addr configuration */
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = INADDR_ANY;
    server->addr.sin_port = htons(PORT);


	/* Bind the socket */
    if (bind(server->sockfd, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0) {
        perror("Bind failed");
		free(server);
        close(server->sockfd);
		return (NULL);
    }
	return (server);
}

void server_routine(ChessServer *server) {
	struct sockaddr_in	cliaddr;
	socklen_t			addr_len = sizeof(cliaddr);
	char				buffer[1024];
	ssize_t				len = 0;

	ft_memset(buffer, 0, sizeof(buffer));
	ft_memset(&cliaddr, 0, sizeof(cliaddr));

	ft_printf_fd(1, ORANGE"Server waiting on port %d...\n"RESET, PORT);

	while (1) {
		len = recvfrom(server->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &addr_len);
		if (len > 0) {
			buffer[len] = '\0';
			ft_printf_fd(1, CYAN"Server Received: %s\n"RESET, buffer);
			handle_client_message(server->sockfd, &server->room.clientA, &server->room.clientB, &cliaddr, buffer);
			ft_memset(buffer, 0, sizeof(buffer));
		}
	}

    close(server->sockfd);
	free(server);
}

int main() {
	ChessServer			*server = NULL;


	if (!(server = server_setup())) {
		return (1);
	}
	server_routine(server);
    return (0);
}
