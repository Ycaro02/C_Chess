
#include "../libft/basic_define.h"

/*
 * This file is used to define enums that are used in the project.
 * The X-Macro technique is used to define the enums.
 * Make more easier to add new enums and to generate automatically 
 * the string conversion functions.
*/ 

#define CHESS_TILE_ENUM \
	X(INVALID_TILE, =-1) \
	X(A1, ) \
	X(B1, ) \
	X(C1, ) \
	X(D1, ) \
	X(E1, ) \
	X(F1, ) \
	X(G1, ) \
	X(H1, ) \
	X(A2, ) \
	X(B2, ) \
	X(C2, ) \
	X(D2, ) \
	X(E2, ) \
	X(F2, ) \
	X(G2, ) \
	X(H2, ) \
	X(A3, ) \
	X(B3, ) \
	X(C3, ) \
	X(D3, ) \
	X(E3, ) \
	X(F3, ) \
	X(G3, ) \
	X(H3, ) \
	X(A4, ) \
	X(B4, ) \
	X(C4, ) \
	X(D4, ) \
	X(E4, ) \
	X(F4, ) \
	X(G4, ) \
	X(H4, ) \
	X(A5, ) \
	X(B5, ) \
	X(C5, ) \
	X(D5, ) \
	X(E5, ) \
	X(F5, ) \
	X(G5, ) \
	X(H5, ) \
	X(A6, ) \
	X(B6, ) \
	X(C6, ) \
	X(D6, ) \
	X(E6, ) \
	X(F6, ) \
	X(G6, ) \
	X(H6, ) \
	X(A7, ) \
	X(B7, ) \
	X(C7, ) \
	X(D7, ) \
	X(E7, ) \
	X(F7, ) \
	X(G7, ) \
	X(H7, ) \
	X(A8, ) \
	X(B8, ) \
	X(C8, ) \
	X(D8, ) \
	X(E8, ) \
	X(F8, ) \
	X(G8, ) \
	X(H8, ) \
	X(TILE_MAX, ) \


#define TILE_TYPE_ENUM \
	X(EMPTY_TILE, =0) \
	X(ALLY_TILE, ) \
	X(ENEMY_TILE, ) \

#define CHESS_PIECE_ENUM \
	X(EMPTY, =-1) \
	X(WHITE_PAWN, ) \
	X(WHITE_KNIGHT, ) \
	X(WHITE_BISHOP, ) \
	X(WHITE_ROOK, ) \
	X(WHITE_QUEEN, ) \
	X(WHITE_KING, ) \
	X(BLACK_PAWN, ) \
	X(BLACK_KNIGHT, ) \
	X(BLACK_BISHOP, ) \
	X(BLACK_ROOK, ) \
	X(BLACK_QUEEN, ) \
	X(BLACK_KING, ) \
	X(PIECE_MAX, ) \


#define CHESS_BOOL_INFO_ENUM \
	X(WHITE_CHECK, =0) \
	X(BLACK_CHECK, ) \
	X(WHITE_KING_MOVED, ) \
	X(WHITE_KING_ROOK_MOVED, ) \
	X(WHITE_QUEEN_ROOK_MOVED, ) \
	X(BLACK_KING_MOVED, ) \
	X(BLACK_KING_ROOK_MOVED, ) \
	X(BLACK_QUEEN_ROOK_MOVED, ) \


#define BTN_STATE_ENUM \
	X(BTN_STATE_RELEASED, =0) \
	X(BTN_STATE_PRESSED, ) \
	X(BTN_STATE_DISABLED, ) \


#define BTN_TYPE_ENUM \
	X(BTN_INVALID, =-1) \
	X(BTN_RESUME, ) \
	X(BTN_PROFILE, ) \
	X(BTN_SEARCH, ) \
	X(BTN_RECONNECT, ) \
	X(BTN_QUIT, ) \
	X(BTN_SERVER_IP, ) \
	X(BTN_CENTER1, ) \
	X(BTN_CENTER2, ) \
	X(BTN_MAX, ) \

#define CLIENT_STATE_ENUM \
	X(CLIENT_STATE_INIT, =0) \
	X(CLIENT_STATE_SEND_COLOR, ) \
	X(CLIENT_STATE_WAIT_COLOR, ) \
	X(CLIENT_STATE_RECONNECT, ) \
	X(CLIENT_STATE_CONNECTED, ) \


#define ROOM_STATE_ENUM \
	X(ROOM_STATE_WAITING, =0) \
	X(ROOM_STATE_PLAYING, ) \
	X(ROOM_STATE_WAIT_RECONNECT, ) \
	X(ROOM_STATE_END, ) \


#define MSG_TYPE_ENUM \
	X(MSG_TYPE_COLOR, =1) \
	X(MSG_TYPE_MOVE, ) \
	X(MSG_TYPE_PROMOTION, ) \
	X(MSG_TYPE_RECONNECT, ) \
	X(MSG_TYPE_QUIT, ) \
	X(MSG_TYPE_ACK, ='A') \
	X(MSG_TYPE_HELLO, ='H') \
	X(MSG_TYPE_DISCONNECT, ='D') \
	X(MSG_TYPE_CLIENT_ALIVE, ='C') \
	X(MSG_TYPE_GAME_END, ='G') \

#define MSG_IDX_ENUM \
	X(IDX_TYPE, =0) \
	X(IDX_MSG_ID, ) \
	X(IDX_FROM, =3) \
	X(IDX_TO, =4) \
	X(IDX_PIECE, =5) \
	X(IDX_TIMER, =6) \
	X(IDX_TIMER_END, ) \

#define CHESS_FLAG_ENUM \
	X(FLAG_LISTEN, =1<<0) \
	X(FLAG_JOIN, =1<<1) \
	X(FLAG_RECONNECT, =1<<2) \
	X(FLAG_SERVER_IP, =1<<3) \
	X(FLAG_NETWORK, =1<<4) \
	X(FLAG_HELP, =1<<5) \
	X(FLAG_CENTER_TEXT_INPUT, =1<<6) \
	X(FLAG_PROMOTION_SELECTION, =1<<7) \
	X(FLAG_EDIT_PROFILE, =1<<8) \

