#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unloadTexture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
}

int main(void) {
	ChessBoard *board = ft_calloc(1, sizeof(ChessBoard));
	if (!board) {
		return (1);
	}

	init_board(board);
	display_bitboard(board->occupied, "Occupied bitboard");
	display_bitboard(board->white, "White bitboard");
	display_bitboard(board->black, "Black bitboard");

	SDLHandle *handle = NULL;

	s32 w = 8 * TILE_SIZE + 9 * TILE_SPACING;
	s32 h = 8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT;

	s32 selected = EMPTY; 

	handle = createSDLHandle(w, h, "Chess", board);
	if (!handle) {
		return (1);
	}
	while (windowIsOpen(handle->window)) {
		selected = eventHandler(handle);
		if (selected == CHESS_QUIT) {
			destroy_sdl_handle(handle);
			windowClose(handle->window, handle->renderer);
			free(handle);
			break ;
		}

		s8 piece_type = get_piece(board, selected);
		if (piece_type == WHITE_PAWN) {
			board->possible_moves = single_pawn_moves((1ULL << selected), board->occupied, board->black, TRUE);
			display_bitboard(board->possible_moves, "White pawn moves");
		} else if (piece_type == BLACK_PAWN) {
			board->possible_moves = single_pawn_moves((1ULL << selected), board->occupied, board->white, FALSE);
			display_bitboard(board->possible_moves, "Black pawn moves");
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}

/* Set and clear bit */
// void set_bit(Bitboard *board, int index) {
// 	*board |= 1ULL << index;
// }

// void clear_bit(Bitboard *board, int index) {
// 	*board &= ~(1ULL << index);
// }

