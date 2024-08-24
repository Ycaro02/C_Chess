#ifndef CHESS_NETWORK_H
#define CHESS_NETWORK_H

#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "chess.h"

#include <stdio.h> // For perror

/* For testing */
#define TEST_MSG_NB 3
#define TIMEOUT_SEC 2
#define SENDER 1
#define RECEIVER 0

/* Contant server port and nb attemps max */
#define SERVER_PORT 24242
#define MAX_ATTEMPTS 10

/* Message disconect */
#define DISCONNECT_MSG "DISCONNECT"

enum e_msg_type {
	MSG_TYPE_COLOR=1,
	MSG_TYPE_MOVE,
	MSG_TYPE_PROMOTION,
	MSG_TYPE_QUIT,
}; 

struct s_network_info {
	struct sockaddr_in	localaddr;
	struct sockaddr_in	servaddr;
	struct sockaddr_in	peeraddr;
	socklen_t			addr_len;
	int					sockfd;
};

typedef enum e_msg_type MsgType;
typedef struct s_network_info NetworkInfo;

FT_INLINE char *message_type_to_str(MsgType msg_type) {
	if (msg_type == MSG_TYPE_COLOR) {
		return ("MSG_TYPE_COLOR");
	} else if (msg_type == MSG_TYPE_MOVE) {
		return ("MSG_TYPE_MOVE");
	} else if (msg_type == MSG_TYPE_PROMOTION) {
		return ("MSG_TYPE_PROMOTION");
	} else if (msg_type == MSG_TYPE_QUIT) {
		return ("MSG_TYPE_QUIT");
	}
	return ("UNKNOWN");
}

/* Max iteration for sending message */
#define	MAX_ITER 50

/* Message max size */
#define	MSG_SIZE 5

/* src/chess_network.c */
NetworkInfo	*init_network(char *server_ip, int local_port, struct timeval timeout);
void		send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr);
void		build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);
void		process_message_receive(SDLHandle *handle, char *msg);

/* src/chess_client.c */
s8			chess_msg_receive(NetworkInfo *info, char *rcv_buffer, char *last_msg_processed);
s8			chess_msg_send(NetworkInfo *info, char *msg);


/* src/handle_message.c */
void	process_message_receive(SDLHandle *handle, char *msg);
void	build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);
void	display_message(char *msg);

#endif /* CHESS_NETWORK_H */