#include <stdio.h>
#include <arpa/inet.h>

#include "../include/chess.h"
#include <signal.h>

#define PORT 24242
#define DISCONNECT_MSG "DISCONNECT"

typedef t_list RoomList;

typedef struct s_client_info {
    struct sockaddr_in	addr;
    s8					connected;
} ClientInfo;

typedef struct s_chess_room {
	u32			room_id;
	ClientInfo	cliA;
	ClientInfo	cliB;
} ChessRoom;

typedef struct s_chess_server {
	int					sockfd;
	struct sockaddr_in	addr;
	RoomList			*room_lst;
} ChessServer;


ChessServer *g_server = NULL;

ChessRoom *room_create(u32 id) {
	ChessRoom *room = ft_calloc(1, sizeof(ChessRoom));
	if (!room) {
		ft_printf_fd(2, RED"Error: alloc %s\n"RESET, __func__);
		return (NULL);
	}
	room->room_id = id;
	return (room);
}

void room_list_add(RoomList **lst, ChessRoom *room) {
	t_list *new = ft_lstnew(room);
	if (!new) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return ;
	}
	ft_lstadd_back(lst, new);
}

s8 handle_client_disconect(ChessRoom *r, struct sockaddr_in *cliaddr, char *buff) {
    if (fast_strcmp(buff, DISCONNECT_MSG) == 0) {
		if (r->cliA.connected && ft_memcmp(cliaddr, &r->cliA.addr, sizeof(struct sockaddr_in)) == 0) {
			fast_bzero(&r->cliA.addr, sizeof(struct sockaddr_in));
            r->cliA.connected = FALSE;
        } else if (r->cliB.connected && ft_memcmp(cliaddr, &r->cliB.addr, sizeof(struct sockaddr_in)) == 0) {
			fast_bzero(&r->cliB.addr, sizeof(struct sockaddr_in));
            r->cliB.connected = FALSE;
        }
		ft_printf_fd(1, RED"Client disconnected: %s:%d\n"RESET, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		return (TRUE);
    }
	return (FALSE);
}

void connect_client_together(int sockfd, ChessRoom *r) {
	ft_printf_fd(1, PURPLE"Room is Ready send info: ClientA : %s:%d, ClientB : %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port), inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
	
	/* Send information from B to A */
	sendto(sockfd, (char *)&r->cliB.addr, sizeof(r->cliB.addr), 0, (struct sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	/* Send information from A to B */
	sendto(sockfd, (char *)&r->cliA.addr, sizeof(r->cliA.addr), 0, (struct sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));
}

void handle_client_message(int sockfd, ChessRoom *r, struct sockaddr_in *cliaddr, char *buffer) {
	
	if (handle_client_disconect(r, cliaddr, buffer)) {
		return ;
	} else if (r->cliA.connected && r->cliB.connected) {
		ft_printf_fd(1, YELLOW"Already 2 clients are connected, need to implement multiroom handling\n"RESET);
		return ;
	}

	if (!r->cliA.connected) {
		ft_memcpy(&r->cliA.addr, cliaddr, sizeof(struct sockaddr_in));
		r->cliA.connected = TRUE;
		ft_printf_fd(1, GREEN"Client A connected: %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
	} else if (!r->cliB.connected) {
		ft_memcpy(&r->cliB.addr, cliaddr, sizeof(struct sockaddr_in));
		r->cliB.connected = TRUE;
		ft_printf_fd(1, GREEN"Client B connected: %s:%d\n"RESET, inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
	}

	if (r->cliA.connected && r->cliB.connected) {
		connect_client_together(sockfd, r);		
	}
}

ChessServer *server_setup() {
	ChessServer *server = ft_calloc(1, sizeof(ChessServer));
	struct timeval timeout = {1, 0};

	if (!server) {
		ft_printf_fd(2, RED"Error: alloc %s\n"RESET, __func__);
		return (NULL);
	}

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

	if (setsockopt(server->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0) {
		perror("setsockopt failed");
		free(server);
		close(server->sockfd);
		return (NULL);
	}

	return (server);
}

void server_destroy(ChessServer *server) {
	ft_lstclear(&server->room_lst, free);
	close(server->sockfd);
	free(server);
}

void server_routine(ChessServer *server) {
	struct sockaddr_in	cliaddr;
	socklen_t			addr_len = sizeof(cliaddr);
	char				buffer[1024];
	ssize_t				len = 0;

	fast_bzero(buffer, sizeof(buffer));
	fast_bzero(&cliaddr, sizeof(cliaddr));

	ft_printf_fd(1, ORANGE"Server waiting on port %d...\n"RESET, PORT);

	ChessRoom *room = room_create(1);
	if (!room) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return ;
	}
	room_list_add(&server->room_lst, room);

	while (1) {
		len = recvfrom(server->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &addr_len);
		if (len > 0) {
			buffer[len] = '\0';
			ft_printf_fd(1, CYAN"Server Received: %s from %s:%d\n"RESET, buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
			handle_client_message(server->sockfd, room, &cliaddr, buffer);
			fast_bzero(buffer, sizeof(buffer));
			fast_bzero(&cliaddr, sizeof(cliaddr));
		}
	}
	server_destroy(server);
}

static void signal_handler_server(int signum)
{
	(void)signum;
	ft_printf_fd(1, RED"\nSignal Catch: %d\n"RESET, signum);
	if (g_server) {
		server_destroy(g_server);
	}
	exit(signum);
}

int init_sig_handler(void)
{
	struct sigaction sa;

	sa.sa_handler = signal_handler_server;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	/* Handle SIGINT */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Can't catch SIGINT");
		return (FALSE);
	}
	/* Handle SIGTERM */
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("Can't catch SIGTERM");
		return (FALSE);
	}

	/* Handle SIGQUIT */
	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		perror("Can't catch SIGQUIT");
		return (FALSE);
	}

	/* Handle SIGHUP */
	if (sigaction(SIGHUP, &sa, NULL) == -1) {
		perror("Can't catch SIGHUP");
		return (FALSE);
	}
	return (TRUE);
}

int main() {

	init_sig_handler();

	if (!(g_server = server_setup())) {
		return (1);
	}
	server_routine(g_server);
    return (0);
}
