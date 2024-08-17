#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Update occupied bitboard */
void update_occupied(ChessBoard *b) {
	b->occupied =	b->white_pawns | b->white_knights | b->white_bishops
						| b->white_rooks | b->white_queens | b->white_king
						| b->black_pawns | b->black_knights | b->black_bishops
						| b->black_rooks | b->black_queens | b->black_king;
}

void init_board(ChessBoard *b) {
	b->white_pawns = START_WHITE_PAWNS;
	b->white_knights = START_WHITE_KNIGHTS;
	b->white_bishops = START_WHITE_BISHOPS;
	b->white_rooks = START_WHITE_ROOKS;
	b->white_queens = START_WHITE_QUEENS;
	b->white_king = START_WHITE_KING;
	
	b->black_pawns = START_BLACK_PAWNS;
	b->black_knights = START_BLACK_KNIGHTS;
	b->black_bishops = START_BLACK_BISHOPS;
	b->black_rooks = START_BLACK_ROOKS;
	b->black_queens = START_BLACK_QUEENS;
	b->black_king = START_BLACK_KING;

	update_occupied(b);
}

/* Display bitboard */
void display_bitboard(Bitboard bitboard, const char *msg) {
	ft_printf_fd(1, "%s\n", msg);
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (bitboard >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}

/* Draw chess board */
void draw_empty_chess_board(SDLHandle *handle) {
	iVec2 tilePos;
	u32 color;
	for (int i = 0; i < 64; i++) {
		tilePos = (iVec2){i % 8, i / 8};
		if (i % 2 == 0) {
			color = RGBA_TO_UINT32(255, 255, 255, 255);
		} else {
			color = RGBA_TO_UINT32(0, 0, 0, 255);
		}
		colorTile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
	}
}