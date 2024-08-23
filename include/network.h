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
	MSG_TYPE_COLOR,
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
int			chess_msg_send(int sockfd, struct sockaddr_in peeraddr, socklen_t addr_len, char *msg);
char		*chess_msg_receive(int sockfd, struct sockaddr_in peeraddr, socklen_t addr_len);

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