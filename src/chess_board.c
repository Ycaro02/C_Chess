#include "../include/chess.h"
#include "../include/handle_sdl.h"

// void update_tmp_piece(ChessBoard *b) {
// 	for (s32 i = 0; i < PIECE_MAX; i++) {
// 		b->tmp_piece[i] = b->piece[i];
// 	}
// }

/* Update control bitboard */
void update_piece_control(ChessBoard *b) {
	b->white_control = get_piece_color_control(b, IS_WHITE);
	b->black_control = get_piece_color_control(b, IS_BLACK);

	/* Check for king in check */
	b->white_check = (b->black_control & b->piece[WHITE_KING]) != 0;
	b->black_check = (b->white_control & b->piece[BLACK_KING]) != 0;

	if (b->white_check) {
		ft_printf_fd(1, CYAN"White King in check\n"RESET);
	}
	if (b->black_check) {
		ft_printf_fd(1, ORANGE"Black King in check\n"RESET);
	}
}

/* Update occupied bitboard */
void update_piece_state(ChessBoard *b) {
	b->occupied = 0;
	b->white = 0;
	b->black = 0;
	for (s32 i = 0; i < PIECE_MAX; i++) {
		b->occupied |= b->piece[i];
		if (i < BLACK_PAWN) {
			b->white |= b->piece[i];
		} else {
			b->black |= b->piece[i];
		}
	}

	// update_tmp_piece(b);

	/* Update control bitboard */
	update_piece_control(b);
}

void init_board(ChessBoard *b) {

	/* Set all pieces to 0 */
	ft_bzero(b, sizeof(ChessBoard));

	/* Set start for white and black piece */
	b->piece[WHITE_PAWN] = START_WHITE_PAWNS;
	b->piece[WHITE_KNIGHT] = START_WHITE_KNIGHTS;
	b->piece[WHITE_BISHOP] = START_WHITE_BISHOPS;
	b->piece[WHITE_ROOK] = START_WHITE_ROOKS;
	b->piece[WHITE_QUEEN] = START_WHITE_QUEENS;
	b->piece[WHITE_KING] = START_WHITE_KING;

	b->piece[BLACK_PAWN] = START_BLACK_PAWNS;
	b->piece[BLACK_KNIGHT] = START_BLACK_KNIGHTS;
	b->piece[BLACK_BISHOP] = START_BLACK_BISHOPS;
	b->piece[BLACK_ROOK] = START_BLACK_ROOKS;
	b->piece[BLACK_QUEEN] = START_BLACK_QUEENS;
	b->piece[BLACK_KING] = START_BLACK_KING;

	// b->piece[WHITE_BISHOP] |= (1ULL << E4);
	// b->piece[WHITE_QUEEN] |= (1ULL << E5);
	// b->piece[WHITE_KING] |= (1ULL << E6);
	// b->piece[WHITE_KNIGHT] = START_WHITE_KNIGHTS | START_WHITE_PAWNS;
	// b->piece[BLACK_KNIGHT] = START_BLACK_KNIGHTS | START_BLACK_PAWNS;
	// b->piece[BLACK_PAWN] |= (1ULL << B3);

	update_piece_state(b);
}

/* @brief Get piece from tile
 * @param b		ChessBoard struct
 * @param tile	ChessTile enum
 * @return ChessPiece enum
 */
ChessPiece get_piece_from_tile(ChessBoard *b, ChessTile tile) {
	Bitboard mask = 1ULL << tile;
	ChessPiece piece = EMPTY;
	if (b->occupied & mask) {
		for (s32 i = 0; i < PIECE_MAX; i++) {
			if (b->piece[i] & mask) {
				piece = i;
				break;
			}
		}
	}
	return (piece);
}

ChessPiece get_piece_from_mask(ChessBoard *b, Bitboard mask) {
	ChessPiece piece = EMPTY;
	if (b->occupied & mask) {
		for (s32 i = 0; i < PIECE_MAX; i++) {
			if (b->piece[i] & mask) {
				piece = i;
				break;
			}
		}
	}
	return (piece);
}

s8 is_selected_possible_move(Bitboard possible_moves, ChessTile tile) {
    return ((possible_moves & (1ULL << tile)) != 0);
}


/* Draw chess board */
void draw_board(SDLHandle *handle) {
	iVec2 tilePos;
	u32 color;
	ChessTile tile = A1; // start from A1 for white player corner top left
	ChessPiece pieceIdx = EMPTY;
	for (s32 column = 0; column < 8; column++) {
		for (s32 raw = 0; raw < 8; raw++) {
			tilePos = (iVec2){raw, column};
			
			color = (column + raw) & 1 ? BLACK_TILE : WHITE_TILE;
			
			draw_color_tile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			
			/* Check if tile is current selected possible move */
			if (is_selected_possible_move(handle->board->possible_moves, tile)) {
				color = RGBA_TO_UINT32(0, 0, 200, 100);
				draw_color_tile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			}

			pieceIdx = get_piece_from_tile(handle->board, tile);
			if (pieceIdx != EMPTY) {
				draw_texture_tile(handle->renderer, handle->piece_texture[pieceIdx], tilePos, (iVec2){TILE_SIZE, TILE_SIZE});
			}
			tile++;
		}
	}
}

/* Display bitboard for debug */
void display_bitboard(Bitboard bitboard, const char *msg) {
	ft_printf_fd(1, "%s", msg);
	for (int i = 0; i < TILE_MAX; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (bitboard >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}