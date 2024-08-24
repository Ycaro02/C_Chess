#ifndef CHESS_NETWORK_H
#define CHESS_NETWORK_H

#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>

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

NetworkInfo	*init_network(char *server_ip, int local_port, struct timeval timeout);
void		send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr);
int			chess_msg_send(NetworkInfo *info, char *msg);
char		*chess_msg_receive(NetworkInfo *info);

//main
char *build_message(s32 msg_size, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);
void process_message_receive(SDLHandle *handle, char *msg);


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

/**
 * Packet format 4 char
 * -	1: msg_type
 * if (msg_type == MSG_TYPE_MOVE)
 * - 	2: tile_from
 * - 	3: tile_to
 * - 	4: piece_type
 * if (msg_type == MSG_TYPE_COLOR)
 * -	2: color
 * if (msg_type == MSG_TYPE_PROMOTION)
 * -	2: tile_from
 * -	3: tile_to
 * -	4: NEW_piece_type (QUEEN, ROOK, BISHOP, KNIGHT)
 * -	@note The piece type is the new piece type, not the pawn type (WHITE_PAWN, BLACK_PAWN)
 * 
 */

#endif /* CHESS_NETWORK_H */