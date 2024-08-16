#include "../include/chess.h"

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
void display_bitboard(Bitboard board, const char *msg) {
	ft_printf_fd(1, "%s\n", msg);
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (board >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}