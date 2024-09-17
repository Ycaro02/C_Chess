#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"
#include "../libft/parse_flag/parse_flag.h"
#include "chess_enum.h"
#include <time.h>
#include <stdio.h>

/* Typedef for Bitboard type just unsigned long long (64 bits) */
typedef u64 Bitboard;

/* Define for white and black boolean for different function using 'is_black' argument */
#define IS_WHITE 0
#define IS_BLACK 1

/* White and Black castle constant */
#define WHITE_KING_START_POS E1
#define WHITE_KING_ROOK_START_POS H1
#define WHITE_QUEEN_ROOK_START_POS A1
#define WHITE_KING_CASTLE_PATH ((1ULL << F1) | (1ULL << G1))
#define WHITE_QUEEN_CASTLE_PATH ((1ULL << B1) | (1ULL << C1) | (1ULL << D1))

#define BLACK_KING_START_POS E8
#define BLACK_KING_ROOK_START_POS H8
#define BLACK_QUEEN_ROOK_START_POS A8
#define BLACK_KING_CASTLE_PATH ((1ULL << F8) | (1ULL << G8))
#define BLACK_QUEEN_CASTLE_PATH ((1ULL << B8) | (1ULL << C8) | (1ULL << D8))


/* Start white piece position */
#define START_WHITE_PAWNS (1ULL << A2 | 1ULL << B2 | 1ULL << C2 | 1ULL << D2 | 1ULL << E2 | 1ULL << F2 | 1ULL << G2 | 1ULL << H2)
#define START_WHITE_KNIGHTS (1ULL << B1 | 1ULL << G1)
#define START_WHITE_BISHOPS (1ULL << C1 | 1ULL << F1)
#define START_WHITE_ROOKS (1ULL << A1 | 1ULL << H1)
#define START_WHITE_QUEENS (1ULL << D1)
#define START_WHITE_KING (1ULL << WHITE_KING_START_POS)

/* Start black piece position */
#define START_BLACK_PAWNS (1ULL << A7 | 1ULL << B7 | 1ULL << C7 | 1ULL << D7 | 1ULL << E7 | 1ULL << F7 | 1ULL << G7 | 1ULL << H7)
#define START_BLACK_KNIGHTS (1ULL << B8 | 1ULL << G8)
#define START_BLACK_BISHOPS (1ULL << C8 | 1ULL << F8)
#define START_BLACK_ROOKS (1ULL << A8 | 1ULL << H8)
#define START_BLACK_QUEENS (1ULL << D8)
#define START_BLACK_KING (1ULL << BLACK_KING_START_POS)


/* Mask for A, B, G and H columns */
#define FILE_A (1ULL << A1 | 1ULL << A2 | 1ULL << A3 | 1ULL << A4 | 1ULL << A5 | 1ULL << A6 | 1ULL << A7 | 1ULL << A8) 
#define FILE_B (1ULL << B1 | 1ULL << B2 | 1ULL << B3 | 1ULL << B4 | 1ULL << B5 | 1ULL << B6 | 1ULL << B7 | 1ULL << B8)
#define FILE_G (1ULL << G1 | 1ULL << G2 | 1ULL << G3 | 1ULL << G4 | 1ULL << G5 | 1ULL << G6 | 1ULL << G7 | 1ULL << G8)
#define FILE_H (1ULL << H1 | 1ULL << H2 | 1ULL << H3 | 1ULL << H4 | 1ULL << H5 | 1ULL << H6 | 1ULL << H7 | 1ULL << H8)
#define NOT_FILE_A (~FILE_A)
#define NOT_FILE_B (~FILE_B)
#define NOT_FILE_G (~FILE_G)
#define NOT_FILE_H (~FILE_H)


/* Mask for 1, 2, 3 and 8 rows */
#define RANK_1 (1ULL << A1 | 1ULL << B1 | 1ULL << C1 | 1ULL << D1 | 1ULL << E1 | 1ULL << F1 | 1ULL << G1 | 1ULL << H1)
#define RANK_2 (1ULL << A2 | 1ULL << B2 | 1ULL << C2 | 1ULL << D2 | 1ULL << E2 | 1ULL << F2 | 1ULL << G2 | 1ULL << H2)
#define RANK_7 (1ULL << A7 | 1ULL << B7 | 1ULL << C7 | 1ULL << D7 | 1ULL << E7 | 1ULL << F7 | 1ULL << G7 | 1ULL << H7)
#define RANK_8 (1ULL << A8 | 1ULL << B8 | 1ULL << C8 | 1ULL << D8 | 1ULL << E8 | 1ULL << F8 | 1ULL << G8 | 1ULL << H8)
#define NOT_RANK_1 (~RANK_1)
#define NOT_RANK_2 (~RANK_2)
#define NOT_RANK_7 (~RANK_7)
#define NOT_RANK_8 (~RANK_8)

/* Chess quit */
#define CHESS_QUIT -2

/* Struct for special info handling */
typedef struct s_special_info {
	ChessPiece		type;
	ChessBoolInfo	info_idx;
	ChessTile		tile_from;
} SpecialInfo;

/* Special info array size */
#define SPECIAL_INFO_SIZE 6

/* Struct for move save */
typedef struct s_move_save {
	ChessTile	tile_from;
	ChessTile	tile_to;
	ChessPiece	piece_from;
	ChessPiece	piece_to;
} MoveSave;

/* Typedef for t_list */
typedef t_list ChessMoveList;
typedef t_list ChessPieceList;

/* ChessBoard struct */
typedef struct s_chess_board {
	ChessMoveList	*lst;				/* List of MoveSave struct */
	ChessPieceList	*white_kill_lst;	/* List of white killed pieces */
	ChessPieceList	*black_kill_lst;	/* List of black killed pieces */
	/* 64 bitboard for each piece */
	Bitboard	piece[PIECE_MAX];

	/* Bitboard for occupied tile */
	Bitboard	occupied;

	/* Bitboard for white and black pieces, for eassy access to enemy pieces */
	Bitboard	white;
	Bitboard	black;

	/* Selected piece/tile handling */
	ChessTile	selected_tile;		/* Selected tile by last click */
	ChessPiece	selected_piece;		/* Selected piece by last click */
	Bitboard	possible_moves;		/* Possible moves for selected piece (on selected tile) */

	/* Special Bitboard for 'en passant' rule */
	Bitboard	en_passant;			/* Bitboard for en passant possible moves */
	ChessTile	en_passant_tile;	/* Tile of the pawn can be atatcked by en passant move */

	/* Bitboard for white and black control */
	Bitboard	white_control;		/* Bitboard for white control tile */
	Bitboard	black_control;		/* Bitboard for black control tile */

	/* Store last move for display it */
	ChessTile	last_tile_from;			/* Tile from */
	ChessTile	last_tile_to;			/* Tile to */

	/* Last clicked tile */
	ChessTile	last_clicked_tile;		/* Last clicked tile */

	/* Value of white and black pieces taken */
	s8			white_piece_val;		/* White piece value */
	s8			black_piece_val;		/* Black piece value */

	/* u8 used as 8 boolean info used as follow
	 * 0: white check
	 * 1: black check
	 * 2: white king moved
	 * 3: white king rook moved
	 * 4: white queen rook moved
	 * 5: black king moved
	 * 6: black king rook moved
	 * 7: black queen rook moved
	*/
	u8			info;
} ChessBoard;

/* @brief Function pointer typedef for get move functions 
 * @param b		ChessBoard struct pointer
 * @param piece	Bitboard of piece position to get move (1 bit set)
 * @param type	ChessPiece enum
 * @param is_black	1 for black piece, 0 for white piece
 * @param check_legal	1 for check legal move, 0 for not check legal move
 * @return Bitboard of possible moves
 */
typedef Bitboard (*GetMoveFunc)(ChessBoard*, Bitboard, ChessPiece, s8, s8);

/* Struct for piece move */
typedef struct s_piece_move {
	ChessPiece	white_piece_type;
	ChessPiece	black_piece_type;
	GetMoveFunc get_move_func;
} PieceMove;

/* Piece Move Array size */
#define PIECE_MOVE_ARRAY_SIZE 6

/* @brief Display kill info
 * @param enemy_piece	ChessPiece enum
 * @param tile_to		ChessTile enum
*/
FT_INLINE void display_kill_info(ChessPiece enemy_piece, ChessTile tile_to) {
	printf(RED"Kill %s on [%s]\n"RESET, \
		ChessPiece_to_str(enemy_piece), ChessTile_to_str(tile_to));
}

/* Tile color */
#define BLACK_TILE ((u32)(RGBA_TO_UINT32(0, 120, 120, 255)))
#define WHITE_TILE ((u32)(RGBA_TO_UINT32(255, 255, 255, 255)))



/* Used in move piece to check if the move is a promotion and adapt message sending */
#define PAWN_PROMOTION 2


/* Forward declaration of NetworkInfo */
typedef struct s_network_info NetworkInfo;

/* Message max size */
#define	MSG_SIZE 16


/* Player info struct */
typedef struct s_player_info {
	NetworkInfo *nt_info;				/* Network info */
	char		msg_tosend[MSG_SIZE];	/* Message to send */
	char		msg_receiv[MSG_SIZE];	/* Message received */
	char		last_msg[MSG_SIZE];		/* Last message process */
	char		*name;					/* Player name */
	char		*dest_ip;				/* destination ip, server ip */
	u16			dest_port;				/* port server port */
	ChessPiece	piece_start;			/* ChessPiece color start */
	ChessPiece	piece_end;				/* ChessPiece color end */
	s8			color;					/* player color */
	s8			turn;					/* player turn */
} PlayerInfo;

FT_INLINE s8 random_player_color() {
	srand(time(NULL));
	return (rand() % 2 == 0 ? IS_WHITE : IS_BLACK);
}


#define HELP_MESSAGE "Usage: ./C_chess [OPTION]...\n\n" \
					"Chess game\n\n" \
					"Options:\n" \
					"  -l, --listen         Wait for connection (create a lobby)\n" \
					"  -j, --join           Try to join a lobby\n" \
					"  -p, --port <port>    Local port\n" \
					"  -i, --server-ip <ip>	Server ip\n" \
					"  -n, --network        Network mode\n" \
					"  -h, --help           Display this help\n" \
					"Example: network mode\n" \
					"  Listen mode:\n" \
					"  ./C_chess -n -l -p 8081 -i 127.0.0.1\n" \
					"  Join mode:\n" \
					"  ./C_chess -n -j -p 8081 -i 127.0.0.1\n" \
					"Example: local mode\n" \
					"  ./C_chess\n"

/* Typedef for flag context */
typedef t_flag_context ChessFlagContext;

#define LISTEN_OPT_CHAR		'l'
#define JOIN_OPT_CHAR		'j'
#define RECONNECT_OPT_CHAR	'r'
#define SERVER_IP_OPT_CHAR	'i'
#define NETWORK_OPT_CHAR	'n'
#define HELP_OPT_CHAR		'h'


#define LISTEN_STR		"listen"
#define JOIN_STR		"join"
#define RECONNECT_STR	"reconect"
#define SERVER_IP_STR	"server_ip"
#define NETWORK_STR		"network"
#define HELP_STR		"help"

#define DATA_SAVE_FILE "rsc/C_chess_data.txt"


#define GR_TRUE GREEN"TRUE"RESET
#define GR_FALSE GREEN"FALSE"RESET
#define RED_TRUE RED"TRUE"RESET
#define RED_FALSE RED"FALSE"RESET


/* Forward declarion for SDLHandle struct */
typedef struct s_sdl_handle SDLHandle;

/* src/main.c */
/* Singleton to get the sdl pointer */
SDLHandle	*get_SDL_handle();
void		local_chess_routine();
void		reset_local_board(SDLHandle *h);
void		chess_destroy(SDLHandle *h);

/* src/chess_flag.c */
u32			handle_chess_flag(int argc, char **argv, s8 *error, PlayerInfo *player_info);

/* src/chess_board.c */
void		init_board(ChessBoard *board);
void		update_piece_state(ChessBoard *b);
void		display_bitboard(Bitboard board, const char *msg);
s8			is_selected_possible_move(Bitboard possible_moves, ChessTile tile);
s8			is_en_passant_move(ChessBoard *b, ChessTile tile);
ChessPiece	get_piece_from_tile(ChessBoard *b, ChessTile tile);
ChessPiece	get_piece_from_mask(ChessBoard *b, Bitboard mask);

/* src/chess_piece_moves.c */
Bitboard	get_pawn_moves(ChessBoard *b, Bitboard pawn, ChessPiece type, s8 is_black, s8 check_legal);
Bitboard	get_bishop_moves(ChessBoard *b, Bitboard bishop, ChessPiece type, s8 is_black, s8 check_legal);
Bitboard	get_rook_moves(ChessBoard *b, Bitboard rook, ChessPiece type, s8 is_black, s8 check_legal);
Bitboard	get_queen_moves(ChessBoard *b, Bitboard queen, ChessPiece type, s8 is_black, s8 check_legal);
Bitboard	get_king_moves(ChessBoard *b, Bitboard king, ChessPiece type, s8 is_black, s8 check_legal);
Bitboard	get_knight_moves(ChessBoard *b, Bitboard knight, ChessPiece type, s8 is_black, s8 check_legal);


/* src/generic_piece_move.c */
Bitboard 	get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type, s8 check_legal);
s32			move_piece(SDLHandle *handle, ChessTile tile_from, ChessTile tile_to, ChessPiece type);
void		handle_enemy_piece_kill(ChessBoard *b, ChessPiece type, Bitboard mask_to);

/* src/handle_board.c */
s32			event_handler(SDLHandle *h, s8 player_color);
void		reset_selected_tile(SDLHandle *h);

/* src/draw_baord.c */
s8			is_selected_possible_move(Bitboard possible_moves, ChessTile tile);
s8			is_en_passant_move(ChessBoard *b, ChessTile tile);
ChessTile	handle_tile(ChessTile tile, s32 player_color);
s8			verify_check_and_mat(ChessBoard *b, s8 is_black);
Bitboard	get_piece_color_control(ChessBoard *b, s8 is_black);
void 		update_graphic_board(SDLHandle *h);

/* src/pawn_promotion.c */
s8			check_pawn_promotion(SDLHandle *handle, ChessPiece type, ChessTile tile_to);
void		do_promotion_move(SDLHandle *h, ChessTile tile_from, ChessTile tile_to, ChessPiece new_piece_type, s8 add_list);
void		display_promotion_selection(SDLHandle *h);
void		pawn_selection_event(SDLHandle *h);

/* src/move_save.c */
s8			move_save_add(ChessMoveList **lst, ChessTile tile_from, ChessTile tile_to, ChessPiece piece_from, ChessPiece piece_to);
void		display_move_list(ChessMoveList *lst);
void		add_kill_lst(ChessBoard *b, ChessPiece killed_piece);
void		compute_piece_value(ChessBoard *b);

/* src/parse_message_receive.c */
s8 			ignore_msg(SDLHandle *h, char *buffer);

/* src/timer.c */
u64 		get_time_sec();
void		draw_timer_rect(SDLHandle *h);

/* nickname.c */
// char		*get_nickname_in_file();
char		*get_file_data(char *path, char *keyword, u32 line_idx, int max_size);
void		register_data(char *path, char *nickname, char *server_ip);
#endif /* CHESS_H */