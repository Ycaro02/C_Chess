#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Update control bitboard */
void update_piece_control(ChessBoard *b) {
	b->white_control = get_piece_color_control(b, IS_WHITE);
	b->black_control = get_piece_color_control(b, IS_BLACK);

	/* Check for king in check */
	b->info = u8ValueSet(b->info, WHITE_CHECK, (b->black_control & b->piece[WHITE_KING]) != 0);
	b->info = u8ValueSet(b->info, BLACK_CHECK, (b->white_control & b->piece[BLACK_KING]) != 0);
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

	/* Update control bitboard */
	update_piece_control(b);
}

void init_board(ChessBoard *b) {

	/* Set all pieces to 0 */
	fast_bzero(b, sizeof(ChessBoard));

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

	b->selected_piece = EMPTY;
	b->selected_tile = INVALID_TILE;
	b->last_tile_from = INVALID_TILE;
	b->last_tile_to = INVALID_TILE;
	b->possible_moves = 0;

	/* Update occupied and control bitboard */
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

/* @brief Get piece from mask
 * @param b		ChessBoard struct
 * @param mask	Bitboard mask (1ULL << tile), tile is the position of the piece
 * @return ChessPiece enum
 */
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