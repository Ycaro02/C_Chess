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

/* Get piece from tile */
ChessPiece get_piece(ChessBoard *b, ChessTile tile) {
	Bitboard mask = 1ULL << tile;
	ChessPiece piece = EMPTY;
	if (b->occupied & mask) {
		if (b->white_pawns & mask) {
			piece = WHITE_PAWN;
		} else if (b->white_knights & mask) {
			piece = WHITE_KNIGHT;
		} else if (b->white_bishops & mask) {
			piece = WHITE_BISHOP;
		} else if (b->white_rooks & mask) {
			piece = WHITE_ROOK;
		} else if (b->white_queens & mask) {
			piece = WHITE_QUEEN;
		} else if (b->white_king & mask) {
			piece = WHITE_KING;
		} else if (b->black_pawns & mask) {
			piece = BLACK_PAWN;
		} else if (b->black_knights & mask) {
			piece = BLACK_KNIGHT;
		} else if (b->black_bishops & mask) {
			piece = BLACK_BISHOP;
		} else if (b->black_rooks & mask) {
			piece = BLACK_ROOK;
		} else if (b->black_queens & mask) {
			piece = BLACK_QUEEN;
		} else if (b->black_king & mask) {
			piece = BLACK_KING;
		}
	}
	return (piece);
}

#define BLACK_TILE ((u32)(RGBA_TO_UINT32(0, 120, 0, 255)))
#define WHITE_TILE ((u32)(RGBA_TO_UINT32(255, 255, 255, 255)))

/* Draw chess board */
void draw_board(SDLHandle *handle) {
	iVec2 tilePos;
	u32 color;
	ChessTile tile = H8; // start from H8 for white player
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
			pieceIdx = get_piece(handle->board, tile);
			if (pieceIdx != EMPTY) {
				drawTextureTile(handle->renderer, handle->piece_texture[pieceIdx], tilePos, (iVec2){TILE_SIZE, TILE_SIZE});
			}
			tile--;
		}
	}
}


