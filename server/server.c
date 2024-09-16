#include "../include/chess.h"
#include "../include/network.h"
#include "../include/handle_signal.h"

#define INVALID_CLIENT -1
#define CLIENT_A 0
#define CLIENT_B 1

typedef t_list RoomList;

typedef struct s_chess_client {
	char 			nickname[8];		/* Client nickname */
    SockaddrIn		addr;				/* Client address */
	struct timeval 	last_alive;			/* Last alive packet */
	u64				my_remain_time;	/* Client remaining time */
	u64				enemy_remain_time;	/* Enemy remaining time */
	s8				color;				/* Client color */
	s8				client_state;		/* Client state */
    s8				connected;			/* Client connected */
	s8				player_ready;		/* Player ready */
} ChessClient;

typedef struct s_chess_game_state {
	char 			cliA_nickname[8];
	char 			cliB_nickname[8];
	ChessMoveList	*move_lst;
	u64 			room_id;
	u64 			cliA_timer;
	u64 			cliB_timer;
	u16 			msg_id;
	s8 				cliA_color;
	s8 				cliB_color;
} ChessGameState;

typedef struct s_chess_room {
	ChessGameState	game_state;		/* Game state */
	ChessClient		cliA;			/* Client A */
	ChessClient		cliB;			/* Client B */
	ChessMoveList	*move_lst;		/* Move list */
	u64				room_id;		/* Room ID */
	RoomState		state;			/* Room state */
	u16				msg_id;			/* Message ID of the cli communication */
	u16				last_move_id_saved; /* Last move id saved */
} ChessRoom;

typedef struct s_chess_server {
	int			sockfd;			/* Server socket */
	SockaddrIn	addr;			/* Server address */
	RoomList	*room_lst;		/* Room list */
} ChessServer;

/* Global server pointer */
ChessServer *g_server = NULL;

#ifdef CHESS_WINDOWS_VERSION
	/* Define windows version of getimeofday */
	void gettimeofday(struct timeval *tv, void *tz) {
		LARGE_INTEGER count, freq;
		QueryPerformanceCounter(&count);
		QueryPerformanceFrequency(&freq);
		tv->tv_sec = count.QuadPart / freq.QuadPart;
		tv->tv_usec = (count.QuadPart % freq.QuadPart) * 1000000 / freq.QuadPart;
	}
#endif

/* @brief Update the chess game state structure 
 * @param r The room
 */
void update_chess_game_state(ChessRoom *r) {
	ft_memcpy(r->game_state.cliA_nickname, r->cliA.nickname, 8);
	ft_memcpy(r->game_state.cliB_nickname, r->cliB.nickname, 8);
	r->game_state.move_lst = r->move_lst;
	r->game_state.room_id = r->room_id;
	r->game_state.msg_id = r->msg_id;
	// printf(RED"GAME STATE UPDATED\n"RESET);
	// printf(ORANGE"time: %ld\n"RESET, r->game_state.cliA_timer);
	// printf(ORANGE"Cli A: %s -> [%d] %s\n"RESET, r->cliA.nickname, r->cliA.color, r->cliA.color == IS_WHITE ? "WHITE" : "BLACK");
	// printf(ORANGE"Cli B: %s -> [%d] %s\n"RESET, r->cliB.nickname, r->cliB.color, r->cliB.color == IS_WHITE ? "WHITE" : "BLACK");
	// printf(GREEN"STATE Cli A: %s -> [%d] %s\n"RESET, r->cliA.nickname, r->game_state.cliA_color, r->game_state.cliA_color == IS_WHITE ? "WHITE" : "BLACK");
	// printf(GREEN"STATE Cli B: %s -> [%d] %s\n"RESET, r->cliB.nickname, r->game_state.cliB_color, r->game_state.cliB_color == IS_WHITE ? "WHITE" : "BLACK");
}

/* @brief Store the move list in list
 * @param lst pointer to the head of the move list
 * @param msg The message buffer
 */
void server_store_movelist(ChessMoveList **lst, char *msg) { 
	ChessPiece piece_from = 0, piece_to = 0;
	ChessTile tile_from = 0, tile_to = 0;
	MsgType msg_type = msg[IDX_TYPE];

	if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION)  {
		/* Get the tile from */
		tile_from = msg[IDX_FROM];
		/* Get the tile to */
		tile_to = msg[IDX_TO];
			if (msg_type == MSG_TYPE_MOVE) {
				/* Get the piece from */
				piece_from = msg[IDX_PIECE];
				/* Get the piece to */
				piece_to = piece_from;
			} else {
				/* Get the piece from */
				piece_to = msg[IDX_PIECE];
				/* Get the piece to select pawn same color than piece to */
				piece_from = piece_to >= BLACK_PAWN ? BLACK_PAWN : WHITE_PAWN; 
			}
			move_save_add(lst, tile_from, tile_to, piece_from, piece_to);
	}
}


/* @brief Create a new room
 * @param id The room id
 * @return The new room
 */
ChessRoom *room_create(u32 id) {
	ChessRoom *room = ft_calloc(1, sizeof(ChessRoom));
	if (!room) {
		printf(RED"Error: alloc %s\n"RESET, __func__);
		return (NULL);
	}
	room->room_id = id;
	fast_bzero(&room->cliA, sizeof(ChessClient));
	fast_bzero(&room->cliB, sizeof(ChessClient));
	room->state = ROOM_STATE_WAITING;
	room->move_lst = NULL;
	room->msg_id = 0;
	room->last_move_id_saved = 0;
	return (room);
}

/* @brief Add a room to the list
 * @param lst The room list
 * @param room The room to add
 */
void room_list_add(RoomList **lst, ChessRoom *room) {
	t_list *new = ft_lstnew(room);
	if (!new) {
		printf(RED"Error: %s\n"RESET, __func__);
		return ;
	}
	ft_lstadd_back(lst, new);
}

/* @brief Compare two address (SockaddrIn structure)
 * @param client The client address
 * @param receive The receive address
 * @return TRUE if the address are the same, FALSE otherwise
 */
s8 addr_cmp(SockaddrIn *client, SockaddrIn *receive) {
	if (client->sin_addr.s_addr == receive->sin_addr.s_addr && client->sin_port == receive->sin_port && client->sin_family == receive->sin_family) {
		return (TRUE);
	}
	return (FALSE);
}


/* @brief Build a client message quit
 * @return The message
 */
char *build_client_msg_quit() {
	char *data = ft_calloc(1, MAGIC_SIZE + MSG_SIZE);
	if (!data) {
		printf(RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_STRING, MAGIC_SIZE);
	data[MAGIC_SIZE] = MSG_TYPE_QUIT;
	return (data);
}

/* @brief Send a quit message to the client and set the room state to wait reconnect
 * @param sockfd The socket file descriptor
 * @param r The room
 * @param client The client to send the message
 */
void send_quit_msg(int sockfd, ChessRoom *r, ChessClient *client) {
	char	*quit_msg = NULL; 
	size_t	len = MAGIC_SIZE + MSG_SIZE;

	if (client->connected) {
		quit_msg = build_client_msg_quit();
		if (!quit_msg) {
			printf(RED"Error: %s\n"RESET, __func__);
			return ;
		}
		sendto(sockfd, quit_msg, len, 0, (Sockaddr *)&client->addr, sizeof(client->addr));
		free(quit_msg);
	}

	if (r->state == ROOM_STATE_PLAYING) {
		r->state = ROOM_STATE_WAIT_RECONNECT;
	}
}

/* @brief Check if the message is a disconnect message and send a quit message to the client if it is
 * @param r The room
 * @param cliaddr The client address
 * @param buff The message buffer
 * @return TRUE if the message is a disconnect message, FALSE otherwise
 */
s8 client_disconnect_msg(ChessRoom *r, SockaddrIn *cliaddr, char *buff) {
	if (fast_strcmp(buff, DISCONNECT_MSG) == 0) {
		if (r->cliA.connected && addr_cmp(cliaddr, &r->cliA.addr)) {
			send_quit_msg(g_server->sockfd, r, &r->cliB);
			fast_bzero(&r->cliA, sizeof(ChessClient));
        } else if (r->cliB.connected && addr_cmp(cliaddr, &r->cliB.addr)) {
			send_quit_msg(g_server->sockfd, r, &r->cliA);
			fast_bzero(&r->cliB, sizeof(ChessClient));
        }
		printf(RED"Client disconnected: %s:%hu\n"RESET, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		if (!r->cliA.connected && !r->cliB.connected) {
			r->state = ROOM_STATE_WAITING;
		}
		return (TRUE);
    }
	return (FALSE);
}

/* @brief Display a brut packet
 * @param msg The message
 * @param packet_size The packet size
 * @param msg_type The message type
 */
void display_brut_packet(char *msg, u64 packet_size, char *msg_type) {
	printf(PINK"Packet %s: "ORANGE, msg_type);
	for (u64 i = 0; i < packet_size; i++) {
		printf("%d ", msg[i]);
	}
	printf("\n"RESET);
}


/* @brief Format a client message with adding the magic string
 * @param msg The message
 * @param message_size The message size
 * @return The formatted message
 */
char *format_client_message(char *msg, u64 message_size){
	char *data = ft_calloc(1, MAGIC_SIZE + message_size);
	if (!data) {
		printf(RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_STRING, MAGIC_SIZE);
	ft_memcpy(data + MAGIC_SIZE, msg, message_size);
	// display_brut_packet(data, MAGIC_SIZE + message_size, "Client Message");
	return (data);
}

/* @brief Format a connect packet with adding the magic connect string and the client state
 * @param msg The message
 * @param message_size The message size
 * @param player_state The player state
 * @return The formatted message
 */
char *format_connect_packet(char *msg, u64 message_size, s8 player_state){
	char *data = ft_calloc(1, CONNECT_PACKET_SIZE);
	if (!data) {
		printf(RED"Error: %s\n"RESET, __func__);
		return (NULL);
	}
	ft_memcpy(data, MAGIC_CONNECT_STR, MAGIC_SIZE);
	ft_memcpy(data + MAGIC_SIZE, msg, message_size);
	data[CONNECT_PACKET_SIZE - 1] = player_state;

	display_brut_packet(data, CONNECT_PACKET_SIZE, "Connect");
	return (data);
}

void send_reconnect_packet(ChessRoom *r, int sockfd, s8 last_connected) {
	char *reconnect_msg, *format_reconnect_msg;
	u64 my_timer = 0, enemy_timer = 0;
	u16 msg_size = 0;
	s8 color = -1;
	
	/* Get the right timer and color */
	if (last_connected == CLIENT_A) {
		my_timer = r->game_state.cliA_timer;
		enemy_timer = r->game_state.cliB_timer;
		color = r->game_state.cliB_color; /* need to send the reverse color */
	} else if (last_connected == CLIENT_B) {
		my_timer = r->game_state.cliB_timer;
		enemy_timer = r->game_state.cliA_timer;
		color = r->game_state.cliA_color; /* need to send the reverse color */
	}
	
	/* Build the reconnect message */
	reconnect_msg = build_reconnect_message(r->game_state.move_lst, &msg_size, my_timer, enemy_timer, r->game_state.msg_id, color);
	if (!reconnect_msg) {
		printf(RED"Error: %s\n"RESET, __func__);
		return ;
	}
	
	/* Format the reconnect message */
	format_reconnect_msg = format_client_message(reconnect_msg, msg_size);
	if (!format_reconnect_msg) {
		printf(RED"Error: %s\n"RESET, __func__);
		free(reconnect_msg);
		return ;
	}

	/* Send it */
	msg_size += MAGIC_SIZE;
	if (last_connected == CLIENT_A) {
		printf("Send reconnect packet to %s\n", r->cliA.nickname);
		sendto(sockfd, format_reconnect_msg, msg_size, 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	} else if (last_connected == CLIENT_B) {
		printf("Send reconnect packet to %s\n", r->cliB.nickname);
		sendto(sockfd, format_reconnect_msg, msg_size, 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));
	}
	free(reconnect_msg);
	free(format_reconnect_msg);
}

/* @brief Connect the client together set the room state to playing
 * @param sockfd The socket file descriptor
 * @param r The room
 */
void connect_client_together(int sockfd, ChessRoom *r, s8 last_connected) {

	char *dataClientA = format_connect_packet(r->cliA.nickname, 8, r->cliA.client_state);
	char *dataClientB = format_connect_packet(r->cliB.nickname, 8, r->cliB.client_state);

	if (!dataClientA || !dataClientB) {
		printf(RED"Error: %s\n"RESET, __func__);
		return ;
	}

	printf(PURPLE"Room is Ready send info:\nClientA : %s:%hu -> %s\nClientB : %s:%hu -> %s\n"RESET,
	 inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port), ClientState_to_str(r->cliA.client_state),
	 inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port), ClientState_to_str(r->cliB.client_state));

	/* Send information from B to A */
	sendto(sockfd, dataClientB, CONNECT_PACKET_SIZE, 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	/* Send information from A to B */
	sendto(sockfd, dataClientA, CONNECT_PACKET_SIZE, 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));

	free(dataClientA);
	free(dataClientB);

	// We need to check for reconnect here and send recconnect packet to the right client
	if (r->state == ROOM_STATE_WAIT_RECONNECT) {
		send_reconnect_packet(r, sockfd, last_connected);
	}

	r->state = ROOM_STATE_PLAYING;
}

/* @brief Set the first timer data
 * @param r The room
 * @param timer The timer
 */
void init_game_state_data(ChessRoom *r, u64 timer) {
	r->cliA.my_remain_time = timer;
	r->cliA.enemy_remain_time = timer;
	r->cliB.my_remain_time = timer;
	r->cliB.enemy_remain_time = timer;

	r->game_state.cliA_timer = timer;
	r->game_state.cliB_timer = timer;
	r->game_state.cliA_color = r->cliA.color;
	r->game_state.cliB_color = r->cliB.color;
}

s8 is_first_move(u32 lst_size, u16 msg_id, u16 last_move_id_saved) {
	printf("List size: %u, msg_id: %u, last_move_id_saved: %u\n", lst_size, msg_id, last_move_id_saved);
	return (lst_size == 0 && msg_id == 0 && last_move_id_saved == 0);
}

/* @brief Save the information of the client
 * @param r The room
 * @param msg The message buffer
 * @param is_client_a TRUE if the client is A, FALSE otherwise
 */
void server_save_info(ChessRoom *r, char *msg, s8 is_client_a) {
	s8 msg_type = msg[IDX_TYPE];
	u32 lst_size = ft_lstsize(r->move_lst);

	if (msg_type >= MSG_TYPE_COLOR && msg_type <= MSG_TYPE_PROMOTION) {
		r->msg_id = GET_MESSAGE_ID(msg);
		printf("Message ID rcv: %d in |%s|\n", r->msg_id, MsgType_to_str(msg_type));

		// if is message move and is last_msg_id + 1 or is the first move message
		if ((msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) \
			&& ((is_first_move(lst_size, r->msg_id, r->last_move_id_saved)) || (r->msg_id == r->last_move_id_saved + 1)))
		{
			server_store_movelist(&r->move_lst, msg);
			display_move_list(r->move_lst);
			r->last_move_id_saved = r->msg_id;
		} else if (msg_type == MSG_TYPE_COLOR) {
				if (is_client_a) {
					r->cliB.color = msg[IDX_FROM];
					r->cliA.color = !r->cliB.color;
					init_game_state_data(r, *(u64 *)&msg[IDX_TIMER]);
				} else {
					r->cliA.color = msg[IDX_FROM];
					r->cliB.color = !r->cliA.color;
					init_game_state_data(r, *(u64 *)&msg[IDX_TIMER]);
				}
				printf("Client A |%s| color: %s\n", r->cliA.nickname, r->cliA.color == IS_WHITE ? "WHITE" : "BLACK");
				printf("Client B |%s| color: %s\n", r->cliB.nickname, r->cliB.color == IS_WHITE ? "WHITE" : "BLACK");
		}
		if (msg_type != MSG_TYPE_COLOR) {
			if (is_client_a) {
				r->cliA.my_remain_time = *(u64 *)&msg[IDX_TIMER];
				r->cliB.enemy_remain_time = *(u64 *)&msg[IDX_TIMER];
			} else {
				r->cliB.my_remain_time = *(u64 *)&msg[IDX_TIMER];
				r->cliA.enemy_remain_time = *(u64 *)&msg[IDX_TIMER];
			}
		}
		update_chess_game_state(r);
		// printf("Client A |%s| remain time: %ld, enemy: %ld\n", r->cliA.nickname, r->cliA.my_remain_time, r->cliA.enemy_remain_time);
		// printf("Client B |%s| remain time: %ld, enemy: %ld\n", r->cliB.nickname, r->cliB.my_remain_time, r->cliB.enemy_remain_time);
	}
}

/* @brief Transmit a message to the other client
 * @param sockfd The socket file descriptor
 * @param r The room
 * @param addr_from The address of the sender
 * @param buffer The message buffer
 * @param msg_size The message size
 */
void transmit_message(int sockfd, ChessRoom *r, SockaddrIn *addr_from, char *buffer, ssize_t msg_size) {
	s8 is_client_a = addr_cmp(addr_from, &r->cliA.addr);
	s8 is_client_b = addr_cmp(addr_from, &r->cliB.addr);

	char *data = format_client_message(buffer, msg_size);
	if (!data) {
		printf(RED"Error: %s\n"RESET, __func__);
		return ;
	}
	if (is_client_a) {
		sendto(sockfd, data, msg_size + MAGIC_SIZE, 0, (Sockaddr *)&r->cliB.addr, sizeof(r->cliB.addr));
	} else if (is_client_b) {
		sendto(sockfd, data, msg_size + MAGIC_SIZE, 0, (Sockaddr *)&r->cliA.addr, sizeof(r->cliA.addr));
	}

	server_save_info(r, buffer, is_client_a); 



	free(data);
}

/* @brief Check if the message is an alive message and update the timer of last alive packet
 * @param r The room
 * @param addr The address of the sender
 * @param buffer The message buffer
 * @param msg_size The message size
 * @return TRUE if the message is an alive message, FALSE otherwise
 */
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

/* @brief Check if the client is timeout
 * @param last_alive The last alive packet
 * @return TRUE if the client is timeout, FALSE otherwise
 */
s8 client_timeout_alive(struct timeval *last_alive) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return ((now.tv_sec - last_alive->tv_sec) > CLIENT_NOT_ALIVE_TIMEOUT);
}

/* @brief Handle the client timeout
 * @param r The room
 */
void handle_client_timeout(ChessRoom *r) {
	if (r->cliA.connected && client_timeout_alive(&r->cliA.last_alive)) {
		printf(RED"Client A timeout: %s:%hu\n"RESET, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
		send_quit_msg(g_server->sockfd, r, &r->cliB);
		fast_bzero(&r->cliA, sizeof(ChessClient));
	} else if (r->cliB.connected && client_timeout_alive(&r->cliB.last_alive)) {
		printf(RED"Client B timeout: %s:%hu\n"RESET, inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
		send_quit_msg(g_server->sockfd, r, &r->cliA);
		fast_bzero(&r->cliB, sizeof(ChessClient));
	}
	if (!r->cliA.connected && !r->cliB.connected) {
		r->state = ROOM_STATE_WAITING;
		if (r->move_lst != NULL) {
			printf(ORANGE"Room reset to waiting\n"RESET);
			ft_lstclear(&r->move_lst, free);
			fast_bzero(r, sizeof(ChessRoom));
		}
	}
}

/* @brief Set the client data
 * @param cliaddr The client address
 * @param now The current time
 * @param connected The client connected
 * @return The client
 */
void set_client_data(ChessClient *client, SockaddrIn *cliaddr, struct timeval *now) {
	ft_memcpy(&client->addr, cliaddr, sizeof(SockaddrIn));
	ft_memcpy(&client->last_alive, now, sizeof(struct timeval));
	client->connected = TRUE;
	client->player_ready = TRUE;
}


/* @brief Handle the client connection to the server, handle the client state too
 * @param r The room
 * @param cliaddr The client address
 * @param sockfd The socket file descriptor
 */
void handle_client_connect(ChessRoom *r, SockaddrIn *cliaddr, int sockfd, char *nickname) {
	struct timeval now;
	s8 last_connected = INVALID_CLIENT;

	/* Check if the room is waiting to start or reconnect */
	if (r->state == ROOM_STATE_WAITING) {
		printf(ORANGE"Room is waiting to start\n"RESET);
		r->cliA.client_state = CLIENT_STATE_WAIT_COLOR;
		r->cliB.client_state = CLIENT_STATE_SEND_COLOR;
	} else if (r->state == ROOM_STATE_WAIT_RECONNECT) {
		printf(YELLOW"Room is waiting for reconnect\n"RESET);
		r->cliA.client_state = CLIENT_STATE_RECONNECT;
		r->cliB.client_state = CLIENT_STATE_RECONNECT;
	} else if (r->state == ROOM_STATE_END || r->state == ROOM_STATE_PLAYING) {
		printf(RED"Error: Room is end or in playing\n"RESET);
		return ;
	}

	gettimeofday(&now, NULL);
	if (!r->cliA.connected && !addr_cmp(cliaddr, &r->cliB.addr)) {
		set_client_data(&r->cliA, cliaddr, &now);
		ft_memcpy(r->cliA.nickname, nickname, 8);
		last_connected = CLIENT_A;
		printf(GREEN"Client A connected: |%s| -> %s:%hu\n"RESET, r->cliA.nickname, inet_ntoa(r->cliA.addr.sin_addr), ntohs(r->cliA.addr.sin_port));
	} else if (!r->cliB.connected && !addr_cmp(cliaddr, &r->cliA.addr)) {
		set_client_data(&r->cliB, cliaddr, &now);
		ft_memcpy(r->cliB.nickname, nickname, 8);
		last_connected = CLIENT_B;
		printf(GREEN"Client B connected: |%s| -> %s:%hu\n"RESET, r->cliB.nickname, inet_ntoa(r->cliB.addr.sin_addr), ntohs(r->cliB.addr.sin_port));
	}
	if (r->cliA.connected && r->cliB.connected && r->cliA.player_ready && r->cliB.player_ready) {
		connect_client_together(sockfd, r, last_connected);
	}
}

s8 is_end_game_message(char *buffer, ssize_t msg_size) {
	if (msg_size == GAME_END_LEN && ft_memcmp(buffer, GAME_END_MSG, GAME_END_LEN) == 0) {
		printf(RED"Game End Receive\n"RESET);
		return (TRUE);
	}
	return (FALSE);
}

/* @brief Handle the client message
 * @param sockfd The socket file descriptor
 * @param r The room to handle
 * @param cliaddr The client address
 * @param buffer The message buffer
 * @param msg_size The message size
 */
void handle_client_message(int sockfd, ChessRoom *r, SockaddrIn *cliaddr, char *buffer, ssize_t msg_size) {

	if (r->cliA.connected && r->cliB.connected && !addr_cmp(cliaddr, &r->cliA.addr) && !addr_cmp(cliaddr, &r->cliB.addr)) {
		printf(RED"Error: not a valid client: %s:%hu\n"RESET, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
		return ;
	}

	/* Check if the message is a hello message */
	if (ft_memcmp(buffer, CONNECT_STR, CONNECT_LEN) == 0 && msg_size == MSG_SIZE) {
		/* Handle client connection */
		handle_client_connect(r, cliaddr, sockfd, buffer + CONNECT_LEN);
		return ;
	}

	/* Check if the message is an alive message */
	if (is_alive_message(r, cliaddr, buffer, msg_size)) {
		return ;
	}

	/* Check if the message is an end game message */
	if (is_end_game_message(buffer, msg_size) && r->state == ROOM_STATE_PLAYING) {
		printf(RED"Game End Room Reset to Waiting\n"RESET);
		// Reset move list
		printf(ORANGE"Game end: Room reset to waiting\n"RESET);
		ft_lstclear(&r->move_lst, free);
		r->state = ROOM_STATE_WAITING;
		r->cliA.player_ready = FALSE;
		r->cliB.player_ready = FALSE;
		return ;
	}

	/* Check if the message is a disconnect message */
	if (client_disconnect_msg(r, cliaddr, buffer)) {
		return ;
	} else if (r->cliA.connected && r->cliB.connected) {
		/* Send message to the other client */
		transmit_message(sockfd, r, cliaddr, buffer, msg_size);
	}
}

/* @brief Setup the server
 * @return The server
 */
ChessServer *server_setup() {
	ChessServer		*server = NULL;
	struct timeval	timeout = {0, 100000};

	if (INIT_NETWORK() != 0) {
		printf(RED"Error: init network\n"RESET);
		return (NULL);
	}

	if (!(server = ft_calloc(1, sizeof(ChessServer)))) {
		printf(RED"Error: alloc %s\n"RESET, __func__);
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
    server->addr.sin_port = htons(SERVER_PORT);

	/* Bind the socket */
    if (bind(server->sockfd, (Sockaddr *)&server->addr, sizeof(server->addr)) < 0) {
        perror("Bind failed");
        CLOSE_SOCKET(server->sockfd);
		free(server);
		return (NULL);
    }

	if (SOCKET_NO_BLOCK(server->sockfd, timeout) < 0) {
		perror("Socket no block failed");
		CLOSE_SOCKET(server->sockfd);
		free(server);
		return (NULL);
	}
	return (server);
}

void room_destroy(void *room) {
	ChessRoom *r = room;

	if (r->move_lst) {
		ft_lstclear(&r->move_lst, free);
	}
	free(r);
}

/* @brief Destroy the server
 * @param server The server
 */
void server_destroy(ChessServer *server) {
	ft_lstclear(&server->room_lst, room_destroy);
	CLOSE_SOCKET(server->sockfd);
	free(server);
	CLEANUP_NETWORK();
}

/* @brief Server routine
 * @param server The server
 */
void server_routine(ChessServer *server) {
	SockaddrIn			cliaddr;
	SocketLen			addr_len = sizeof(cliaddr);
	char				buffer[4096];
	ssize_t				len = 0;

	fast_bzero(buffer, sizeof(buffer));
	fast_bzero(&cliaddr, sizeof(cliaddr));

	printf(ORANGE"Server waiting on port %d...\n"RESET, SERVER_PORT);

	ChessRoom *room = room_create(1);
	if (!room) {
		printf(RED"Error: %s\n"RESET, __func__);
		return ;
	}
	room_list_add(&server->room_lst, room);

	while (1) {
		len = recvfrom(server->sockfd, buffer, sizeof(buffer), 0, (Sockaddr *)&cliaddr, &addr_len);
		if (len > 0) {
			buffer[len] = '\0';
			// printf(CYAN"Server Received: %s from %s:%hu\n"RESET, MsgType_to_str(buffer[0]), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
			handle_client_message(server->sockfd, room, &cliaddr, buffer, len);
			fast_bzero(buffer, sizeof(buffer));
			fast_bzero(&cliaddr, sizeof(cliaddr));
		}
		/* Check if the client is timeout */
		handle_client_timeout(room);
	}
	server_destroy(server);
}

static void signal_handler_server(int signum) {
	printf(RED"\nSignal Catch: %d\n"RESET, signum);
	if (g_server) {
		server_destroy(g_server);
	}
	exit(signum);
}

int main() {
	INIT_SIGNAL_HANDLER(signal_handler_server);

	if (!(g_server = server_setup())) {
		return (1);
	}
	server_routine(g_server);
    return (0);
}
