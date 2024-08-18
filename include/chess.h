#ifndef CHESS_H
#define CHESS_H

#include "../libft/libft.h"

/* Typedef for Bitboard type just unsigned long long (64 bits) */
typedef u64 Bitboard;

/* Enum for chess tile */
// enum e_chess_tile {
// 	INVALID_TILE=-1,
// 	H8, G8, F8, E8, D8, C8, B8, A8,
// 	H7, G7, F7, E7, D7, C7, B7, A7,
// 	H6, G6, F6, E6, D6, C6, B6, A6,
// 	H5, G5, F5, E5, D5, C5, B5, A5,
// 	H4, G4, F4, E4, D4, C4, B4, A4,
// 	H3, G3, F3, E3, D3, C3, B3, A3,
// 	H2, G2, F2, E2, D2, C2, B2, A2,
// 	H1, G1, F1, E1, D1, C1, B1, A1,
// 	TILE_MAX
// };


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

/* Mask for A and H columns */
#define FILE_A (0x0101010101010101ULL)
#define FILE_B (0x0202020202020202ULL)
#define FILE_G (0x4040404040404040ULL)
#define FILE_H (0x8080808080808080ULL)


/* Mask for 1 and 8 rows */
#define RANK_1 (1ULL << A1 | 1ULL << B1 | 1ULL << C1 | 1ULL << D1 | 1ULL << E1 | 1ULL << F1 | 1ULL << G1 | 1ULL << H1)
#define RANK_2 (1ULL << A2 | 1ULL << B2 | 1ULL << C2 | 1ULL << D2 | 1ULL << E2 | 1ULL << F2 | 1ULL << G2 | 1ULL << H2)
#define RANK_7 (1ULL << A7 | 1ULL << B7 | 1ULL << C7 | 1ULL << D7 | 1ULL << E7 | 1ULL << F7 | 1ULL << G7 | 1ULL << H7)
#define RANK_8 (1ULL << A8 | 1ULL << B8 | 1ULL << C8 | 1ULL << D8 | 1ULL << E8 | 1ULL << F8 | 1ULL << G8 | 1ULL << H8)


/* Mask for not A and H columns, same for ran 1 and 8 */
#define NOT_FILE_A (~FILE_A)
#define NOT_FILE_B (~FILE_B)
#define NOT_FILE_G (~FILE_G)
#define NOT_FILE_H (~FILE_H)

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

/* ChessBoard struct */
struct s_chess_board {
	/* Bitboard for each piece */
	Bitboard piece[PIECE_MAX];

	/* Board 1 for occupied, 0 for empty */
	Bitboard occupied;
	Bitboard white;
	Bitboard black;
	Bitboard possible_moves;
};

/* Typedef for ChessBoard struct */
typedef struct s_chess_board ChessBoard;
typedef enum e_chess_tile ChessTile;
typedef enum e_chess_piece ChessPiece;

/* src/chess_board.c */
void init_board(ChessBoard *board);
void update_occupied(ChessBoard *board);
void display_bitboard(Bitboard board, const char *msg);
ChessPiece get_piece(ChessBoard *b, ChessTile tile);

/* src/chess_get_moves.c */
Bitboard get_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_black);
Bitboard get_bishop_moves(Bitboard bishop, Bitboard occupied, Bitboard enemy);
Bitboard get_rook_moves(Bitboard rook, Bitboard occupied, Bitboard enemy);
Bitboard get_queen_moves(Bitboard queen, Bitboard occupied, Bitboard enemy);
Bitboard get_king_moves(Bitboard king, Bitboard occupied, Bitboard enemy);
Bitboard get_knight_moves(Bitboard knight, Bitboard occupied, Bitboard enemy);

#endif /* CHESS_H */