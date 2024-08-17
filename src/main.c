#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Mask for A and H columns */
// #define FILE_A = 0x0101010101010101ULL;
// #define FILE_H = 0x8080808080808080ULL;

int main(void) {
	ChessBoard board = {0};

	init_board(&board);
	display_bitboard(board.occupied, "Occupied bitboard");

	SDLHandle *handle = NULL;

	s32 w = 8 * TILE_SIZE + 9 * TILE_SPACING;
	s32 h = 8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT;

	handle = createSDLHandle(w, h, "Chess");
	if (!handle) {
		return (1);
	}
	while (windowIsOpen(handle->window)) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT \
				|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
				SDL_DestroyWindow(handle->window);
				SDL_Quit();
				TTF_Quit();
				return (0);
			}
		}
		windowClear(handle->renderer);
		draw_empty_chess_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
}

/* Set and clear bit */
// void set_bit(Bitboard *board, int index) {
// 	*board |= 1ULL << index;
// }

// void clear_bit(Bitboard *board, int index) {
// 	*board &= ~(1ULL << index);
// }

