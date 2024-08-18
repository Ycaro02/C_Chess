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
	ChessTile tile = A1; // start from A1 for white player
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

// Bitboard single_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_white) {
    
// 	(void)is_white;
// 	/* One step, if pawn is white, it moves up, if black, it moves down */
// 	Bitboard one_step = (pawn >> 8) & ~occupied;
// 	s8 one_step_free = ((pawn >> 8) & ~occupied) == (pawn >> 8);

//     Bitboard two_steps = 0;
	
// 	if (one_step_free) {
// 		two_steps = ((pawn & START_WHITE_PAWNS) >> 16) & ~occupied;
// 	}

// 	/* Compute attacks left and right, and avoid out of bound */
//     Bitboard attacks_left = (pawn >> 7) & ~FILE_H & enemy;
//     Bitboard attacks_right = (pawn >> 9) & ~FILE_A & enemy;

//     return (one_step | two_steps | attacks_left | attacks_right);
// }


Bitboard single_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_black) {
    Bitboard one_step, two_steps, attacks_left, attacks_right;
	/* One step, if pawn is white, it moves up, if black, it moves down */
    s8 direction = is_black ? 8 : -8;

    one_step = (is_black ? (pawn >> direction) : (pawn << -direction)) & ~occupied;
    
    s8 one_step_free = one_step != 0;

	/* Compute two steps if pawn is in starting position */
    if (one_step_free) {
        two_steps = (is_black ? ((pawn & START_BLACK_PAWNS) >> 2*direction) : ((pawn & START_WHITE_PAWNS) << 2*-direction)) & ~occupied;
    } else {
        two_steps = 0;
    }

	/* Compute attacks left and right, and avoid out of bound */
    attacks_left = (is_black ? (pawn >> (direction - 1)) : (pawn << -(direction - 1))) & ~FILE_H & enemy;
    attacks_right = (is_black ? (pawn >> (direction + 1)) : (pawn << -(direction + 1))) & ~FILE_A & enemy;

    return (one_step | two_steps | attacks_left | attacks_right);
}


// Bitboard single_bishop_moves(Bitboard bishop, Bitboard occupied, Bitboard enemy) {
//     Bitboard attacks = 0, oob_column, oob_raw, move;
//     s32 bishop_dir[4] = {7, 9, -7, -9};
//     for (s32 i = 0; i < 4; i++) {
//         s32 direction = bishop_dir[i];
//         for (s32 j = 1; j < 8; j++) {
//             oob_column = (direction == 7 || direction == -9) ? FILE_H : FILE_A;
//             oob_raw = (direction == 7 || direction == 9) ? RANK_8 : RANK_1;
//             s32 dir_shift = direction * j;
//             if (dir_shift < -63 || dir_shift > 63) {
//                 // fprintf(stderr, "Out of bound %d\n", dir_shift);
//                 break;
//             }
//             move = (bishop >> dir_shift) & ~occupied & ~oob_column & ~oob_raw;
//             if (move == 0) {
//                 break;
//             }
//             attacks |= move;
//             if (move & enemy) {
//                 break;
//             }
// 			ft_printf_fd(1, "Bishop move %d\n", j);
//         }
//     }
//     return attacks;
// }

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