#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"

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

/* Macro to convert tile to string */
#define TILE_TO_STRING(t) (char[3]){'A' + (t) % 8, '1' + (t) / 8, '\0'}

/* Start white piece position */
#define START_WHITE_PAWNS (1ULL << A2 | 1ULL << B2 | 1ULL << C2 | 1ULL << D2 | 1ULL << E2 | 1ULL << F2 | 1ULL << G2 | 1ULL << H2)
#define START_WHITE_KNIGHTS (1ULL << B1 | 1ULL << G1)
#define START_WHITE_BISHOPS (1ULL << C1 | 1ULL << F1)
#define START_WHITE_ROOKS (1ULL << A1 | 1ULL << H1)
#define START_WHITE_QUEENS (1ULL << E1)
#define START_WHITE_KING (1ULL << D1)

/* Start black piece position */
#define START_BLACK_PAWNS (1ULL << A7 | 1ULL << B7 | 1ULL << C7 | 1ULL << D7 | 1ULL << E7 | 1ULL << F7 | 1ULL << G7 | 1ULL << H7)
#define START_BLACK_KNIGHTS (1ULL << B8 | 1ULL << G8)
#define START_BLACK_BISHOPS (1ULL << C8 | 1ULL << F8)
#define START_BLACK_ROOKS (1ULL << A8 | 1ULL << H8)
#define START_BLACK_QUEENS (1ULL << E8)
#define START_BLACK_KING (1ULL << D8)

/* Tile color */
#define BLACK_TILE ((u32)(RGBA_TO_UINT32(0, 120, 0, 255)))
#define WHITE_TILE ((u32)(RGBA_TO_UINT32(255, 255, 255, 255)))

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

typedef enum e_chess_tile ChessTile;
typedef enum e_chess_piece ChessPiece;

/* ChessBoard struct */
struct s_chess_board {
	/* Bitboard for each piece */
	Bitboard	piece[PIECE_MAX];

	/* Board 1 for occupied, 0 for empty */
	Bitboard	occupied;

	/* Bitboard for white and black pieces, for eassy access to enemy pieces */
	Bitboard	white;
	Bitboard	black;

	/* Bitboard for selected piece possible moves */
	ChessTile	selected_tile;
	Bitboard	possible_moves;

	/* Bitboard for white and black control */
	Bitboard	white_control;
	Bitboard	black_control;

	/* Boolean for white and black king check */
	s8			white_check;
	s8			black_check;
};

/* Typedef for ChessBoard struct and enum */
typedef struct s_chess_board ChessBoard;


/* Function pointer for get move functions */
typedef Bitboard (*GetMoveFunc)(Bitboard, Bitboard, Bitboard);

/* Struct for piece move */
struct s_piece_move {
	ChessPiece	white_piece_type;
	ChessPiece	black_piece_type;
	GetMoveFunc get_move_func;
};

/* Typedef for piece move */
typedef struct s_piece_move PieceMove;

/* Array of piece move struct */
#define PIECE_MOVE_ARRAY { \
	{WHITE_KNIGHT, BLACK_KNIGHT, get_knight_moves}, \
	{WHITE_BISHOP, BLACK_BISHOP, get_bishop_moves}, \
	{WHITE_ROOK, BLACK_ROOK, get_rook_moves}, \
	{WHITE_QUEEN, BLACK_QUEEN, get_queen_moves}, \
	{WHITE_KING, BLACK_KING, get_king_moves}, \
} 

/* Piece Move Array size */
#define PIECE_MOVE_ARRAY_SIZE 5

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

// main
Bitboard get_piece_color_control(ChessBoard *b, s8 is_black);

/* src/chess_board.c */
void init_board(ChessBoard *board);
void update_piece_state(ChessBoard *b);
void display_bitboard(Bitboard board, const char *msg);

s8 is_selected_possible_move(Bitboard possible_moves, ChessTile tile);

ChessPiece get_piece_from_tile(ChessBoard *b, ChessTile tile);

/* src/chess_piece_moves.c */
Bitboard 	get_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_black, s8 only_attacks);
Bitboard 	get_bishop_moves(Bitboard bishop, Bitboard occupied, Bitboard enemy);
Bitboard 	get_rook_moves(Bitboard rook, Bitboard occupied, Bitboard enemy);
Bitboard 	get_queen_moves(Bitboard queen, Bitboard occupied, Bitboard enemy);
Bitboard 	get_king_moves(Bitboard king, Bitboard occupied, Bitboard enemy);
Bitboard 	get_knight_moves(Bitboard knight, Bitboard occupied, Bitboard enemy);
Bitboard 	get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type);

void		move_piece(ChessBoard *board, ChessTile tile_from, ChessTile tile_to, ChessPiece type);

#endif /* CHESS_H */