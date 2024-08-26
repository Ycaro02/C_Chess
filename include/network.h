#ifndef CHESS_NETWORK_H
#define CHESS_NETWORK_H

#ifdef CHESS_WINDOWS_VERSION
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	typedef SOCKET Socket;
	typedef int SocketLen; // Define SocketLen for Windows
	#define CLOSE_SOCKET closesocket
	#define INIT_NETWORK() init_network_windows()
	#define CLEANUP_NETWORK() cleanup_network_windows()
	#define ft_printf_fd(fd, format, ...) printf(format, __VA_ARGS__)
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/time.h>
	typedef int Socket;
	typedef socklen_t SocketLen; // Define SocketLen for Unix
	#define CLOSE_SOCKET close
	#define INIT_NETWORK() init_network_posix()
	#define CLEANUP_NETWORK() cleanup_network_posix()
#endif


int init_network_posix();
void cleanup_network_posix();
int init_network_windows();
void cleanup_network_windows();

#include <stdio.h> // For perror
#include "chess.h"

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

typedef struct sockaddr_in SockaddrIn;

struct s_network_info {
    Socket	sockfd;
    SockaddrIn	localaddr;
    SockaddrIn	servaddr;
    SockaddrIn	peeraddr;
    SocketLen	addr_len;
	fd_set		readfds;
	struct		timeval timeout;
};


// struct s_network_info {
// 	struct sockaddr_in	localaddr;
// 	struct sockaddr_in	servaddr;
// 	struct sockaddr_in	peeraddr;
// 	socklen_t			addr_len;
// 	int					sockfd;
// };

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

/* src/chess_network.c */
NetworkInfo	*init_network(char *server_ip, int local_port, struct timeval timeout);
s8			network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip);
void		send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr);

/* src/handle_message.c */
void	process_message_receive(SDLHandle *handle, char *msg);
void	display_message(char *msg);
void	build_message(char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type, s32 turn);
s8		chess_msg_receive(SDLHandle *h, NetworkInfo *info, char *rcv_buffer, char *last_msg_processed);
s8		chess_msg_send(NetworkInfo *info, char *msg);
s8		safe_msg_send(SDLHandle *h);

/* src/network_routine.c */
void	network_chess_routine(SDLHandle *h);

#endif /* CHESS_NETWORK_H */