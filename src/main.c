#include "../include/chess.h"

/* Mask for A and H columns */
// #define FILE_A = 0x0101010101010101ULL;
// #define FILE_H = 0x8080808080808080ULL;

/* Update occupied bitboard */
void update_occupied(ChessBoard *board) {
	board->occupied =	board->white_pawns | board->white_knights | board->white_bishops
						| board->white_rooks | board->white_queens | board->white_king
						| board->black_pawns | board->black_knights | board->black_bishops
						| board->black_rooks | board->black_queens | board->black_king;
}

void init_board(ChessBoard *board) {
	board->white_pawns = START_WHITE_PAWNS;
	board->white_knights = START_WHITE_KNIGHTS;
	board->white_bishops = START_WHITE_BISHOPS;
	board->white_rooks = START_WHITE_ROOKS;
	board->white_queens = START_WHITE_QUEENS;
	board->white_king = START_WHITE_KING;
	
	board->black_pawns = START_BLACK_PAWNS;
	board->black_knights = START_BLACK_KNIGHTS;
	board->black_bishops = START_BLACK_BISHOPS;
	board->black_rooks = START_BLACK_ROOKS;
	board->black_queens = START_BLACK_QUEENS;
	board->black_king = START_BLACK_KING;

	update_occupied(board);
}

/* Display bitboard */
void display_bitboard(Bitboard board) {
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (board >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}

int main(void) {
	ChessBoard board = {0};

	init_board(&board);

	display_bitboard(board.occupied);

	ft_printf_fd(1, "Hello From C_Chess\n");
}

/* Set and clear bit */
// void set_bit(Bitboard *board, int index) {
// 	*board |= 1ULL << index;
// }

// void clear_bit(Bitboard *board, int index) {
// 	*board &= ~(1ULL << index);
// }

