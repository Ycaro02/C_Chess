#include <stdio.h>
#include <arpa/inet.h>
#include <time.h>

#include "../include/chess.h"
#include "../include/network.h"
#include <signal.h>

#define PORT 24242

typedef t_list RoomList;

typedef struct s_chess_client {
    SockaddrIn		addr;			/* Client address */
	struct timeval 	last_alive;		/* Last alive packet */
	s8				client_state;	/* Client state */
    s8				connected;		/* Client connected */
} ChessClient;

typedef struct s_chess_room {
	ChessClient	cliA;			/* Client A */
	ChessClient	cliB;			/* Client B */
	u32			room_id;		/* Room ID */
	RoomState	state;			/* Room state */
} ChessRoom;

typedef struct s_chess_server {
	int			sockfd;			/* Server socket */
	SockaddrIn	addr;			/* Server address */
	RoomList	*room_lst;		/* Room list */
} ChessServer;

void send_quit_msg(int sockfd, ChessClient *client);

ChessServer *g_server = NULL;

ChessRoom *room_create(u32 id) {
	ChessRoom *room = ft_calloc(1, sizeof(ChessRoom));
	if (!room) {
		ft_printf_fd(2, RED"Error: alloc %s\n"RESET, __func__);
		return (NULL);
	}
	room->room_id = id;
	fast_bzero(&room->cliA, sizeof(ChessClient));
	fast_bzero(&room->cliB, sizeof(ChessClient));
	room->state = ROOM_STATE_WAITING;
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

s8 client_disconnect_msg(ChessRoom *r, SockaddrIn *cliaddr, char *buff) {
	if (fast_strcmp(buff, DISCONNECT_MSG) == 0) {
		if (r->cliA.connected && addr_cmp(cliaddr, &r->cliA.addr)) {
			send_quit_msg(g_server->sockfd, &r->cliB);
			fast_bzero(&r->cliA, sizeof(ChessClient));
        } else if (r->cliB.connected && addr_cmp(cliaddr, &r->cliB.addr)) {
			send_quit_msg(g_server->sockfd, &r->cliA);
			fast_bzero(&r->cliB, sizeof(ChessClient));
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

char *format_connect_packet(char *msg, u64 message_size, s8 player_state){
	char *data = ft_calloc(1, CONNECT_PACKET_SIZE);
	if (!data) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_CONNECT_STR, MAGIC_SIZE);
	ft_memcpy(data + MAGIC_SIZE, msg, message_size);
	data[CONNECT_PACKET_SIZE - 1] = player_state;
	return (data);
}

void connect_client_together(int sockfd, ChessRoom *r) {

	char *dataClientA = format_connect_packet((char *)&r->cliA.addr, sizeof(r->cliA.addr), r->cliA.client_state);
	char *dataClientB = format_connect_packet((char *)&r->cliB.addr, sizeof(r->cliB.addr), r->cliB.client_state);

	if (!dataClientA || !dataClientB) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return ;
	}

	ft_printf_fd(1, PURPLE"Room is Ready send info: ClientA : %s:%d, ClientB : %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port), inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));

	/* Send information from B to A */
	sendto(sockfd, dataClientB, CONNECT_PACKET_SIZE, 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	/* Send information from A to B */
	sendto(sockfd, dataClientA, CONNECT_PACKET_SIZE, 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));

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
	}
	free(data);
}

s8 is_alive_message(ChessRoom *r, SockaddrIn *addr, char *buffer, ssize_t msg_size) {
	struct timeval now;
	
	if (msg_size != ALIVE_LEN || ft_memcmp(buffer, ALIVE_MSG, ALIVE_LEN) != 0) {
		return (FALSE);
	}

	gettimeofday(&now, NULL);

	if (r->cliA.connected && addr_cmp(addr, &r->cliA.addr)) {
		ft_memcpy(&r->cliA.last_alive, &now, sizeof(struct timeval));
	} else if (r->cliB.connected && addr_cmp(addr, &r->cliB.addr)) {
		ft_memcpy(&r->cliB.last_alive, &now, sizeof(struct timeval));
	}
	// printf("Receive alive packet from %s:%hu\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	return (TRUE);
}


s8 client_timeout_alive(struct timeval *last_alive) {
	struct timeval now;
	gettimeofday(&now, NULL);
	
	s64 diff = (now.tv_sec - last_alive->tv_sec);
	if (diff > CLIENT_NOT_ALIVE_TIMEOUT) {
		printf("Client timeout: %ld\n", diff);
		printf("Last alive: %ld\n", last_alive->tv_sec);
		printf("Now: %ld\n", now.tv_sec);
		return (TRUE);
	}
	return (FALSE);
}

char *build_client_msg_quit() {
	char *data = ft_calloc(1, MAGIC_SIZE + MSG_SIZE);
	if (!data) {
		ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_STRING, MAGIC_SIZE);
	data[MAGIC_SIZE] = MSG_TYPE_QUIT;
	return (data);
}

void send_quit_msg(int sockfd, ChessClient *client) {
	char	*quit_msg = NULL; 
	size_t	len = MAGIC_SIZE + MSG_SIZE;

	if (client->connected) {
		quit_msg = build_client_msg_quit();
		if (!quit_msg) {
			ft_printf_fd(2, RED"Error: %s\n"RESET, __func__);
			return ;
		}
		sendto(sockfd, quit_msg, len, 0, (Sockaddr *)&client->addr, sizeof(client->addr));
		free(quit_msg);
	}
}

void handle_client_timeout(ChessRoom *r) {
	if (r->cliA.connected && client_timeout_alive(&r->cliA.last_alive)) {
		ft_printf_fd(1, RED"Client A timeout: %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
		send_quit_msg(g_server->sockfd, &r->cliB);
		fast_bzero(&r->cliA, sizeof(ChessClient));
	} else if (r->cliB.connected && client_timeout_alive(&r->cliB.last_alive)) {
		ft_printf_fd(1, RED"Client B timeout: %s:%d\n"RESET, inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
		send_quit_msg(g_server->sockfd, &r->cliA);
		fast_bzero(&r->cliB, sizeof(ChessClient));
	}
}

ChessClient set_client_data(SockaddrIn *cliaddr, struct timeval *now, ClientState state ,s8 connected) {
	ChessClient client;

	ft_memcpy(&client.addr, cliaddr, sizeof(SockaddrIn));
	ft_memcpy(&client.last_alive, now, sizeof(struct timeval));
	client.connected = connected;
	client.client_state = state;
	return (client);
}

void handle_client_connect(ChessRoom *r, SockaddrIn *cliaddr, int sockfd) {
	struct timeval now;
	ClientState clientA_state = CLIENT_STATE_INIT, clientB_state = CLIENT_STATE_INIT;

	if (r->state == ROOM_STATE_WAITING) {
		ft_printf_fd(1, RED"Error: Room is playing\n"RESET);
		clientA_state = CLIENT_STATE_WAIT_COLOR;
		clientB_state = CLIENT_STATE_SEND_COLOR;
	} else if (r->state == ROOM_STATE_WAIT_RECONNECT) {
		ft_printf_fd(1, RED"Error: Room is waiting for reconnect\n"RESET);
		clientA_state = CLIENT_STATE_RECONNECT;
		clientB_state = CLIENT_STATE_RECONNECT;
	} else if (r->state == ROOM_STATE_END || r->state == ROOM_STATE_PLAYING) {
		ft_printf_fd(1, RED"Error: Room is end or in playing\n"RESET);
		return ;
	}

	gettimeofday(&now, NULL);
	if (!r->cliA.connected && !addr_cmp(cliaddr, &r->cliB.addr)) {
		r->cliA = set_client_data(cliaddr, &now, clientA_state, TRUE);
		ft_printf_fd(1, GREEN"Client A connected: %s:%d\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
	} else if (!r->cliB.connected && !addr_cmp(cliaddr, &r->cliA.addr)) {
		r->cliB = set_client_data(cliaddr, &now, clientB_state, TRUE);
		ft_printf_fd(1, GREEN"Client B connected: %s:%d\n"RESET, inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
	}
	if (r->cliA.connected && r->cliB.connected) {
		connect_client_together(sockfd, r);		
	}
}

void handle_client_message(int sockfd, ChessRoom *r, SockaddrIn *cliaddr, char *buffer, ssize_t msg_size) {

	if (r->cliA.connected && r->cliB.connected && !addr_cmp(cliaddr, &r->cliA.addr) && !addr_cmp(cliaddr, &r->cliB.addr)) {
		ft_printf_fd(2, RED"Error: not a valid client: %s:%d\n"RESET, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		return ;
	}

	/* Check if the message is a hello message */
	if (msg_size == HELLO_LEN && ft_memcmp(buffer, HELLO_STR, HELLO_LEN) == 0) {
		/* Handle client connection */
		handle_client_connect(r, cliaddr, sockfd);
		return ;
	}

	/* Check if the message is an alive message */
	if (is_alive_message(r, cliaddr, buffer, msg_size)) {
		return ;
	}

	/* Check if the message is a disconnect message */
	if (client_disconnect_msg(r, cliaddr, buffer)) {
		return ;
	} else if (r->cliA.connected && r->cliB.connected) {
		/* Send message to the other client */
		transmit_message(sockfd, r, cliaddr, buffer, msg_size);
		return ;
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
		/* Check if the client is timeout */
		handle_client_timeout(room);
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
