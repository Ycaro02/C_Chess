#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"
#include "../libft/parse_flag/parse_flag.h"


/* Typedef for Bitboard type just unsigned long long (64 bits) */
typedef u64 Bitboard;

/* Define for white and black boolean for different function using 'is_black' argument */
#define IS_WHITE 0
#define IS_BLACK 1

/* Enum for chess tile */
enum e_chess_tile {
	INVALID_TILE=-1,
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	TILE_MAX
};

/* Typedef for chess tile enum */
typedef enum e_chess_tile ChessTile;

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

/* Enum for chess piece */
enum e_chess_piece {
	EMPTY=-1,
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
	BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
	PIECE_MAX
};

/* Typedef for chess piece enum */
typedef enum e_chess_piece ChessPiece;

/* Enum for chess boolean info */
enum chess_bool_info {
	WHITE_CHECK = 0,
	BLACK_CHECK,
	WHITE_KING_MOVED,
	WHITE_KING_ROOK_MOVED,
	WHITE_QUEEN_ROOK_MOVED,
	BLACK_KING_MOVED,
	BLACK_KING_ROOK_MOVED,
	BLACK_QUEEN_ROOK_MOVED,
};

/* Typedef for chess boolean info enum */
typedef enum chess_bool_info ChessBoolInfo;

/* Struct for special info handling */
struct s_special_info {
	ChessPiece		type;
	ChessBoolInfo	info_idx;
	ChessTile		tile_from;
};

/* Typedef for special info struct */
typedef struct s_special_info SpecialInfo;

/* Special info array size */
#define SPECIAL_INFO_SIZE 6

/* ChessBoard struct */
struct s_chess_board {
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
};

/* Typedef for ChessBoard struct and enum */
typedef struct s_chess_board ChessBoard;


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
struct s_piece_move {
	ChessPiece	white_piece_type;
	ChessPiece	black_piece_type;
	GetMoveFunc get_move_func;
};

/* Typedef for piece move structure */
typedef struct s_piece_move PieceMove;

/* Piece Move Array size */
#define PIECE_MOVE_ARRAY_SIZE 6

/* Inline function to convert ChessPiece to string */
FT_INLINE const char *chess_piece_to_string(ChessPiece piece) {
	static const char *piece_str[PIECE_MAX] = {
		"WHITE_PAWN", "WHITE_KNIGHT", "WHITE_BISHOP", "WHITE_ROOK", "WHITE_QUEEN", "WHITE_KING",
		"BLACK_PAWN", "BLACK_KNIGHT", "BLACK_BISHOP", "BLACK_ROOK", "BLACK_QUEEN", "BLACK_KING"
	};
	if (piece < 0 || piece >= PIECE_MAX) {
		return ("EMPTY");
	}
	return (piece_str[piece]);
}

/* Macro to convert tile to string */
#define TILE_TO_STRING(t) (char[3]){'A' + (t) % 8, '1' + (t) / 8, '\0'}


/* Enum for tile type, use in handle_occupied_tile */
enum tile_type {
	EMPTY_TILE = 0,
	ALLY_TILE,
	ENEMY_TILE
};

/* @brief Display kill info
 * @param enemy_piece	ChessPiece enum
 * @param tile_to		ChessTile enum
*/
FT_INLINE void display_kill_info(ChessPiece enemy_piece, ChessTile tile_to) {
	ft_printf_fd(1, RED"Kill %s on [%s]\n"RESET, \
		chess_piece_to_string(enemy_piece), TILE_TO_STRING(tile_to));
}

/* Tile color */
#define BLACK_TILE ((u32)(RGBA_TO_UINT32(0, 120, 0, 255)))
#define WHITE_TILE ((u32)(RGBA_TO_UINT32(255, 255, 255, 255)))


typedef struct s_network_info NetworkInfo;


/* Used in move piece to check if the move is a promotion and adapt message sending */
#define PAWN_PROMOTION 2

/* Player info struct */
struct s_player_info {
	NetworkInfo *nt_info;			/* Network info */
	char		msg_tosend[5];		/* Message to send */
	char		msg_receiv[5];		/* Message received */
	char		last_msg[5];		/* Last message process */
	char		*dest_ip;			/* destination ip, NULL if we are the listener */
	ChessPiece	piece_start;		/* ChessPiece color start */
	ChessPiece	piece_end;			/* ChessPiece color end */
	u16			running_port;		/* running port */
	s8			color;				/* player color */
	s8			turn;				/* player turn */
};

/* Typedef for player info struct */
typedef struct s_player_info PlayerInfo;


#include <time.h>

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
#define PORT_OPT_CHAR		'p'
#define SERVER_IP_OPT_CHAR	'i'
#define NETWORK_OPT_CHAR	'n'
#define HELP_OPT_CHAR		'h'

enum chess_flag_value {
	FLAG_LISTEN=1<<0,
	FLAG_JOIN=1<<1,
	FLAG_PORT=1<<2,
	FLAG_SERVER_IP=1<<3,
	FLAG_NETWORK=1<<4,
	FLAG_HELP=1<<5
};

#define LISTEN_STR		"listen"
#define JOIN_STR		"join"
#define PORT_STR		"port"
#define SERVER_IP_STR	"server_ip"
#define NETWORK_STR		"network"
#define HELP_STR		"help"
#define DEFAULT_PORT 	54321
#define MAX_PORT		65535

/* src/chess_flag.c */
u32 handle_chess_flag(int argc, char **argv, s8 *error, PlayerInfo *player_info);

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


/* Declarion for SDLHandle struct */
typedef struct s_sdl_handle SDLHandle;

/* src/generic_piece_move.c */
Bitboard 	get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type, s8 check_legal);
s32			move_piece(SDLHandle *handle, ChessTile tile_from, ChessTile tile_to, ChessPiece type);

/* src/handle_board.c */
void		draw_board(SDLHandle *handle, s8 player_color);
s32			event_handler(SDLHandle *h, s8 player_color);
s8			verify_check_and_mat(ChessBoard *b, s8 is_black);
Bitboard	get_piece_color_control(ChessBoard *b, s8 is_black);
s32			display_promotion_selection(SDLHandle *handle, ChessTile tile_to);
void 		update_graphic_board(SDLHandle *h);
void		draw_piece_over_board(SDLHandle *h, s32 x, s32 y);
void		reset_selected_tile(SDLHandle *h);

/* src/chess_network.c */
s32		network_move_piece(SDLHandle *h, ChessTile tile_selected);
void	network_chess_routine(SDLHandle *h);
s8		network_setup(SDLHandle *handle, u32 flag, PlayerInfo *player_info, char *server_ip);


/* src/handle_message.c */

#endif /* CHESS_H */