#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"

/* Typedef for Bitboard type just unsigned long long (64 bits) */
typedef u64 Bitboard;

/* Enum for chess tile */
enum e_chess_tile {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

/* Start white piece position */
#define START_WHITE_PAWNS 0x000000000000FF00ULL
#define START_WHITE_KNIGHTS (1ULL << B1 | 1ULL << G1)
#define START_WHITE_BISHOPS (1ULL << C1 | 1ULL << F1)
#define START_WHITE_ROOKS (1ULL << A1 | 1ULL << H1)
#define START_WHITE_QUEENS (1ULL << E1)
#define START_WHITE_KING (1ULL << D1)

/* Start black piece position */
#define START_BLACK_PAWNS 0x00FF000000000000ULL
#define START_BLACK_KNIGHTS (1ULL << B8 | 1ULL << G8)
#define START_BLACK_BISHOPS (1ULL << C8 | 1ULL << F8)
#define START_BLACK_ROOKS (1ULL << A8 | 1ULL << H8)
#define START_BLACK_QUEENS (1ULL << E8)
#define START_BLACK_KING (1ULL << D8)

/* Enum for chess piece */
enum e_chess_piece {
	EMPTY=-1,
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
	BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
	PIECE_MAX
};

/* ChessBoard struct */
struct s_chess_board {
	/* Board 1 for occupied, 0 for empty */
	Bitboard occupied;

	/* White pieces */
	Bitboard white_pawns;
	Bitboard white_knights;
	Bitboard white_bishops;
	Bitboard white_rooks;
	Bitboard white_queens;
	Bitboard white_king;

	/* Black pieces */
	Bitboard black_pawns;
	Bitboard black_knights;
	Bitboard black_bishops;
	Bitboard black_rooks;
	Bitboard black_queens;
	Bitboard black_king;
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