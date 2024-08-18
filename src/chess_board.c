#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Update occupied bitboard */
void update_occupied(ChessBoard *b) {
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
}

void init_board(ChessBoard *b) {
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

	b->piece[WHITE_BISHOP] |= (1ULL << E4);
	b->piece[WHITE_QUEEN] |= (1ULL << E5);
	b->piece[WHITE_KING] |= (1ULL << E6);
	b->piece[WHITE_KNIGHT] = START_WHITE_KNIGHTS | START_WHITE_PAWNS;
	b->piece[BLACK_KNIGHT] = START_BLACK_KNIGHTS | START_BLACK_PAWNS;

	update_occupied(b);
}

/* Get piece from tile */
ChessPiece get_piece(ChessBoard *b, ChessTile tile) {
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

s8 isPossibleMove(Bitboard possible_moves, ChessTile tile) {
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
			if (((column + raw) & 1)) {
				color = BLACK_TILE;
			} else {
				color = WHITE_TILE;
			}
			colorTile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			
			/* Check if tile is possible move */
			if (isPossibleMove(handle->board->possible_moves, tile)) {
				color = RGBA_TO_UINT32(0, 0, 200, 100);
				colorTile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			}

			pieceIdx = get_piece(handle->board, tile);
			if (pieceIdx != EMPTY) {
				drawTextureTile(handle->renderer, handle->piece_texture[pieceIdx], tilePos, (iVec2){TILE_SIZE, TILE_SIZE});
			}
			tile++;
		}
	}
}

/* Display bitboard for debug */
void display_bitboard(Bitboard bitboard, const char *msg) {
	ft_printf_fd(1, "%s\n", msg);
	for (int i = 0; i < TILE_MAX; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (bitboard >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}