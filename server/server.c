#include <stdio.h>
#include <arpa/inet.h>

#include "../include/chess.h"
#include "../include/network.h"
#include <signal.h>

#define PORT 24242

typedef t_list RoomList;

typedef struct s_client_info {
    SockaddrIn	addr;
    s8					connected;
} ClientInfo;

typedef struct s_chess_room {
	u32			room_id;
	ClientInfo	cliA;
	ClientInfo	cliB;
} ChessRoom;

typedef struct s_chess_server {
	int					sockfd;
	SockaddrIn	addr;
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
	fast_bzero(&room->cliA, sizeof(ClientInfo));
	fast_bzero(&room->cliB, sizeof(ClientInfo));
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

s8 addr_cmp(SockaddrIn *client, SockaddrIn *receive) {
	if (client->sin_addr.s_addr == receive->sin_addr.s_addr && client->sin_port == receive->sin_port && client->sin_family == receive->sin_family) {
		return (TRUE);
	}
	return (FALSE);
}


s8 handle_client_disconect(ChessRoom *r, SockaddrIn *cliaddr, char *buff) {
    if (fast_strcmp(buff, DISCONNECT_MSG) == 0) {
		if (r->cliA.connected && addr_cmp(cliaddr, &r->cliA.addr)) {
			fast_bzero(&r->cliA.addr, sizeof(SockaddrIn));
            r->cliA.connected = FALSE;
        } else if (r->cliB.connected && addr_cmp(cliaddr, &r->cliB.addr)) {
			fast_bzero(&r->cliB.addr, sizeof(SockaddrIn));
            r->cliB.connected = FALSE;
        }
		ft_printf_fd(1, RED"Client disconnected: %s:%d\n"RESET, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		return (TRUE);
    }
	return (FALSE);
}

char *format_client_message(char *msg, u64 message_size){
	char *data = ft_calloc(1, MAGIC_SIZE + message_size);
	if (!data) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_STRING, MAGIC_SIZE);
	ft_memcpy(data + MAGIC_SIZE, msg, message_size);
	return (data);
}

char *format_connect_packet(char *msg, u64 message_size){
	char *data = ft_calloc(1, MAGIC_SIZE + message_size);
	if (!data) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_RECONNECT_STR, MAGIC_SIZE);
	ft_memcpy(data + MAGIC_SIZE, msg, message_size);
	return (data);
}

void connect_client_together(int sockfd, ChessRoom *r) {

	char *dataClientA = format_connect_packet((char *)&r->cliA.addr, sizeof(r->cliA.addr));
	char *dataClientB = format_connect_packet((char *)&r->cliB.addr, sizeof(r->cliB.addr));

	if (!dataClientA || !dataClientB) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return ;
	}

	ft_printf_fd(1, PURPLE"Room is Ready send info: ClientA : %s:%d, ClientB : %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port), inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));

	/* Send information from B to A */
	// sendto(sockfd, (char *)&r->cliB.addr, MAGIC_SIZE + sizeof(r->cliB.addr), 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	sendto(sockfd, dataClientB, MAGIC_SIZE + sizeof(r->cliB.addr), 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	/* Send information from A to B */
	// sendto(sockfd, (char *)&r->cliA.addr, MAGIC_SIZE + sizeof(r->cliA.addr), 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));
	sendto(sockfd, dataClientA, MAGIC_SIZE + sizeof(r->cliA.addr), 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));

	free(dataClientA);
	free(dataClientB);
}

void transmit_message(int sockfd, ChessRoom *r, SockaddrIn *addr_from, char *buffer, ssize_t msg_size) {
	s8 is_client_a = addr_cmp(addr_from, &r->cliA.addr);
	s8 is_client_b = addr_cmp(addr_from, &r->cliB.addr);

	char *data = format_client_message(buffer, msg_size);
	if (!data) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return ;
	}
	if (is_client_a) {
		sendto(sockfd, data, msg_size + MAGIC_SIZE, 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));
	} else if (is_client_b) {
		sendto(sockfd, data, msg_size + MAGIC_SIZE, 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	} else {
		ft_printf_fd(2, RED"Error not a valid client: %s:%s\n"RESET, inet_ntoa(addr_from->sin_addr), ntohs(addr_from->sin_port));
	}
	free(data);
}

void handle_client_message(int sockfd, ChessRoom *r, SockaddrIn *cliaddr, char *buffer, ssize_t msg_size) {
	
	s8 is_hello = FALSE;

	if (msg_size == HELLO_LEN && ft_strlcpy(buffer, HELLO_STR, HELLO_LEN + 1) == HELLO_LEN) {
		ft_printf_fd(1, "Don't send hello message\n");
		is_hello = TRUE;
	}

	if (handle_client_disconect(r, cliaddr, buffer)) {
		return ;
	} else if (r->cliA.connected && r->cliB.connected && !is_hello) {
		/* Send message to the other client */
		transmit_message(sockfd, r, cliaddr, buffer, msg_size);
		return ;
	}

	if (!r->cliA.connected && !addr_cmp(cliaddr, &r->cliB.addr)) {
		ft_memcpy(&r->cliA.addr, cliaddr, sizeof(SockaddrIn));
		r->cliA.connected = TRUE;
		ft_printf_fd(1, GREEN"Client A connected: %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
	} else if (!r->cliB.connected && !addr_cmp(cliaddr, &r->cliA.addr)) {
		ft_memcpy(&r->cliB.addr, cliaddr, sizeof(SockaddrIn));
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
    if (bind(server->sockfd, (Sockaddr *)&server->addr, sizeof(server->addr)) < 0) {
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
	SockaddrIn			cliaddr;
	socklen_t			addr_len = sizeof(cliaddr);
	char				buffer[4096];
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
		len = recvfrom(server->sockfd, buffer, sizeof(buffer), 0, (Sockaddr *)&cliaddr, &addr_len);
		if (len > 0) {
			buffer[len] = '\0';
			ft_printf_fd(1, CYAN"Server Received: %s from %s:%d\n"RESET, message_type_to_str(buffer[0]), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
			handle_client_message(server->sockfd, room, &cliaddr, buffer, len);
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
