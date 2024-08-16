#include "../include/chess.h"

/* Mask for A and H columns */
// #define FILE_A = 0x0101010101010101ULL;
// #define FILE_H = 0x8080808080808080ULL;

int main(void) {
	ChessBoard board = {0};

	init_board(&board);
	display_bitboard(board.occupied, "Occupied");
	// display_bitboard(board.white_pawns, "White Pawns");
	// display_bitboard(board.black_pawns, "Black Pawns");
}

/* Set and clear bit */
// void set_bit(Bitboard *board, int index) {
// 	*board |= 1ULL << index;
// }

// void clear_bit(Bitboard *board, int index) {
// 	*board &= ~(1ULL << index);
// }

