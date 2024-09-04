#ifndef CHESS_NETWORK_H
#define CHESS_NETWORK_H

#include <stdio.h> // For perror
#include "chess.h"

#ifdef CHESS_WINDOWS_VERSION
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	typedef SOCKET Socket;
	typedef int SocketLen; // Define SocketLen for Windows
	#define CLOSE_SOCKET closesocket
	#define INIT_NETWORK() init_network_windows()
	#define CLEANUP_NETWORK() cleanup_network_windows()
	#define SOCKET_NO_BLOCK(socket, timeout) socket_no_block_windows(socket, timeout)
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
	#define SOCKET_NO_BLOCK(socket, timeout) socket_no_block_posix(socket, timeout)
#endif


/* src/network_os.c */
/* Function prototypes posix */
int		init_network_posix();
void	cleanup_network_posix();
s8		socket_no_block_posix(Socket sockfd, struct timeval timeout);

/* Function prototypes windows */
int		init_network_windows();
void	cleanup_network_windows();
s8		socket_no_block_windows(Socket sockfd, struct timeval timeout);

/* Contant server port and nb attemps max */
#define SERVER_PORT 24242
#define MAX_ATTEMPTS 10

/* Message disconect */
#define DISCONNECT_MSG "DISCONNECT"

/* Message alive */
#define ALIVE_MSG "CLIENT_ALIVE"
#define ALIVE_LEN 12

/* Message ack */
#define ACK_STR "ACK"
#define ACK_LEN 3

/* Message hello */
#define HELLO_STR "Hello"
#define HELLO_LEN 5


/* Magic string for sending addr */
#define MAGIC_STRING ((const char[]){0x7F, 0x42, 'C', 'H', 'E', 'S', 'S', 'M', 'A', 'G', 'I', 'C', 0x7A, 0x7B, 0x42, 0x7F})
#define MAGIC_CONNECT_STR ((const char[]){0x42, 0x7B, 'C', 'H', 'E', 'S', 'S', 'C', 'O', 'N', 'N', 'E', 'C', 'T', 0x7B, 0x42})
#define MAGIC_SIZE 16ULL

/* Connect packet size */
#define CONNECT_PACKET_SIZE (MAGIC_SIZE + sizeof(struct sockaddr_in) + 1ULL)

typedef struct sockaddr_in SockaddrIn;
typedef struct sockaddr Sockaddr;

struct s_network_info {
    Socket		sockfd;
    SockaddrIn	localaddr;
    SockaddrIn	servaddr;
    SockaddrIn	peeraddr;
    SocketLen	addr_len;
	ClientState	client_state;			/* Client state (reconnect, waiter/lister color)*/
	s8			peer_conected;			/* Peer connected */
};

typedef struct s_network_info NetworkInfo;

/* Timeout for reveive */
#define TIMEVAL_TIMEOUT ((struct timeval){0, 10000})

/* Max iteration for sending message */
#define	MAX_ITER 50

/* Alive packet sent to server delay (in seconde)*/
#define SEND_ALIVE_DELAY 5ULL

/* Time before server disconect client for timeout ( in seconde )*/
#define CLIENT_NOT_ALIVE_TIMEOUT 12L

/* src/chess_network.c */
NetworkInfo	*init_network(char *server_ip, struct timeval timeout);
void handle_network_client_state(SDLHandle *handle, u32 flag, PlayerInfo *player_info);
// s8			network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip);
void		send_disconnect_to_server(int sockfd, struct sockaddr_in servaddr);
void		send_alive_to_server(int sockfd, struct sockaddr_in servaddr);
s8			wait_peer_info(NetworkInfo *info, const char *msg);
s8			check_magic_value(char *buff);
void		destroy_network_info(SDLHandle *h);
void 		wait_for_player(SDLHandle *h);

/* src/handle_message.c */
void	process_message_receive(SDLHandle *handle, char *msg);
void	display_message(char *msg);
void	build_message(SDLHandle *h, char *msg, MsgType msg_type, ChessTile tile_from_or_color, ChessTile tile_to, ChessPiece piece_type);
char	*build_reconnect_message(SDLHandle *h, u16 *msg_size);
s8		chess_msg_receive(SDLHandle *h, NetworkInfo *info, char *rcv_buffer);
s8		chess_msg_send(NetworkInfo *info, char *msg, u16 msg_size);
s8		safe_msg_send(SDLHandle *h);

/* src/network_routine.c */
void	network_chess_routine(SDLHandle *h);
void 	send_alive_packet(NetworkInfo *info);

#endif /* CHESS_NETWORK_H */