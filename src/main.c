#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Mask for A and H columns */
// #define FILE_A = 0x0101010101010101ULL;
// #define FILE_H = 0x8080808080808080ULL;

int main(void) {
	ChessBoard board = {0};

	init_board(&board);
	display_bitboard(board.occupied, "Occupied");

	SDLHandle *handle = NULL;

	handle = createSDLHandle(800, 600, "Chess");
	if (!handle) {
		return (1);
	}
	while (windowIsOpen(handle->window)) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				SDL_DestroyWindow(handle->window);
				SDL_Quit();
				TTF_Quit();
				return (0);
			}
		}
		windowClear(handle->renderer);
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

