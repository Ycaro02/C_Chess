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

	SDLHandle *handle = NULL;

	s32 w = 8 * TILE_SIZE + 9 * TILE_SPACING;
	s32 h = 8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT;

	handle = createSDLHandle(w, h, "Chess", board);
	if (!handle) {
		return (1);
	}
	while (windowIsOpen(handle->window)) {
		if (eventHandler(handle)) {
			destroy_sdl_handle(handle);
			windowClose(handle->window, handle->renderer);
			free(handle);
			break ;
		}
		// SDL_Event event;
		// while (SDL_PollEvent(&event)) {
		// 	if (event.type == SDL_QUIT \
		// 		|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
		// 		destroy_sdl_handle(handle);
		// 		windowClose(handle->window, handle->renderer);
		// 		free(handle);
		// 		return (0);
		// 	}
		// }
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

