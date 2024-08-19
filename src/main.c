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
	SDLHandle	*handle = NULL;
	if (!board) {
		return (1);
	}

	init_board(board);
	handle = createSDLHandle(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess", board);
	if (!handle) {
		return (1);
	}

	ChessTile tile_selected = INVALID_TILE;
	ChessPiece piece_type = EMPTY;

	while (windowIsOpen(handle->window)) {
		tile_selected = eventHandler();
		if (tile_selected == CHESS_QUIT) {
			destroy_sdl_handle(handle);
			windowClose(handle->window, handle->renderer);
			free(handle);
			break ;
		}
		if (tile_selected != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (is_selected_possible_move(board->possible_moves, tile_selected)) {
				ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(board->selected_tile), board->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
				move_piece(board, board->selected_tile, tile_selected, piece_type);
				board->possible_moves = 0;
			} else {
				piece_type = get_piece_from_tile(board, tile_selected);
				board->selected_tile = tile_selected;
				ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));
				board->possible_moves = get_piece_move(board, (1ULL << board->selected_tile), piece_type, TRUE);
			}
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
