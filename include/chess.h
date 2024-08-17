#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"

/* Typedef for Bitboard type just unsigned long long (64 bits) */
typedef u64 Bitboard;

/* Enum for chess tile */
enum e_chess_tile {
	H8, G8, F8, E8, D8, C8, B8, A8,
	H7, G7, F7, E7, D7, C7, B7, A7,
	H6, G6, F6, E6, D6, C6, B6, A6,
	H5, G5, F5, E5, D5, C5, B5, A5,
	H4, G4, F4, E4, D4, C4, B4, A4,
	H3, G3, F3, E3, D3, C3, B3, A3,
	H2, G2, F2, E2, D2, C2, B2, A2,
	H1, G1, F1, E1, D1, C1, B1, A1,
	TILE_MAX
};


/* Start white piece position */
#define START_WHITE_PAWNS 0x00FF000000000000ULL
#define START_WHITE_KNIGHTS (1ULL << B1 | 1ULL << G1)
#define START_WHITE_BISHOPS (1ULL << C1 | 1ULL << F1)
#define START_WHITE_ROOKS (1ULL << A1 | 1ULL << H1)
#define START_WHITE_QUEENS (1ULL << E1)
#define START_WHITE_KING (1ULL << D1)

/* Start black piece position */
#define START_BLACK_PAWNS 0x000000000000FF00ULL
#define START_BLACK_KNIGHTS (1ULL << B8 | 1ULL << G8)
#define START_BLACK_BISHOPS (1ULL << C8 | 1ULL << F8)
#define START_BLACK_ROOKS (1ULL << A8 | 1ULL << H8)
#define START_BLACK_QUEENS (1ULL << E8)
#define START_BLACK_KING (1ULL << D8)

/* Tile color */
#define BLACK_TILE ((u32)(RGBA_TO_UINT32(0, 120, 0, 255)))
#define WHITE_TILE ((u32)(RGBA_TO_UINT32(255, 255, 255, 255)))

/* Enum for chess piece */
enum e_chess_piece {
	EMPTY=-1,
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
	BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
	PIECE_MAX
};

/* ChessBoard struct */
struct s_chess_board {
	/* Bitboard for each piece */
	Bitboard piece[PIECE_MAX];

	/* Board 1 for occupied, 0 for empty */
	Bitboard occupied;
};

/* Typedef for ChessBoard struct */
typedef struct s_chess_board ChessBoard;
typedef enum e_chess_tile ChessTile;
typedef enum e_chess_piece ChessPiece;

/* src/chess_board.c */
void init_board(ChessBoard *board);
void update_occupied(ChessBoard *board);
void display_bitboard(Bitboard board, const char *msg);



#endif /* CHESS_H */