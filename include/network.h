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

NetworkInfo	*init_network(char *server_ip, int local_port, struct timeval timeout);
void		send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr);

s8 chess_msg_receive(NetworkInfo *info, char *rcv_buffer, char *last_msg_processed);
s8 chess_msg_send(NetworkInfo *info, char *msg);
void build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);
//main
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


FT_INLINE void display_message(char *msg) {
	MsgType msg_type = msg[0];
	ChessTile tile_from = 0, tile_to = 0;
	ChessPiece piece_type = EMPTY;


	ft_printf_fd(1, YELLOW"Message type: %s: "RESET, message_type_to_str(msg_type));

	if (msg_type == MSG_TYPE_COLOR) {
		ft_printf_fd(1, "brut data: |%d||%d| ->", msg[0], msg[1]);
		ft_printf_fd(1, "Color: %s\n", (msg[1] - 1) == IS_WHITE ? "WHITE" : "BLACK");
		return ;
	} else if (msg_type == MSG_TYPE_QUIT) {
		ft_printf_fd(1, "Opponent quit the game, msg type %d\n", msg[0]);
		return ;
	}	
	
	/* We need to decrement all value cause we send with +1 can't send 0, interpreted like '\0' */
	tile_from = msg[1] - 1;
	tile_to = msg[2] - 1;
	piece_type = msg[3] - 1;
	ft_printf_fd(1, PURPLE"brut data: |%d||%d||%d||%d|\n"RESET, msg[0], msg[1], msg[2], msg[3]);
	if (msg_type == MSG_TYPE_MOVE) {
		ft_printf_fd(1, ORANGE"Move from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} else if (msg_type == MSG_TYPE_PROMOTION) {
		ft_printf_fd(1, ORANGE"Promotion from %s to %s with piece %s\n"RESET, TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to), chess_piece_to_string(piece_type));
	} 
	// else {
	// 	ft_printf_fd(1, RED"Unknown message type\n"RESET);
	// }
}


#define MAX_ITER 50

void build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);


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