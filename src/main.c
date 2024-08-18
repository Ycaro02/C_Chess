#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unloadTexture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
}

#define WINDOW_WIDTH (8 * TILE_SIZE + 9 * TILE_SPACING)
#define WINDOW_HEIGHT (8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT)

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

	Bitboard enemy = 0;
	ChessTile tile_selected = INVALID_TILE;
	ChessPiece piece_type = EMPTY; 

	while (windowIsOpen(handle->window)) {
		tile_selected = eventHandler(handle);
		if (tile_selected == CHESS_QUIT) {
			destroy_sdl_handle(handle);
			windowClose(handle->window, handle->renderer);
			free(handle);
			break ;
		}
		if (tile_selected != INVALID_TILE) {
			piece_type = get_piece(board, tile_selected);
			if (piece_type == WHITE_PAWN || piece_type == BLACK_PAWN) {
				enemy = (piece_type == BLACK_PAWN) ? board->white : board->black;
				board->possible_moves = single_pawn_moves((1ULL << tile_selected), board->occupied, enemy, piece_type == BLACK_PAWN);
			} 
			else if (piece_type == BLACK_BISHOP || piece_type == WHITE_BISHOP) {
				enemy = (piece_type == BLACK_BISHOP) ? board->white : board->black;
				board->possible_moves = single_bishop_moves((1ULL << tile_selected), board->occupied, enemy);
			} 
			else if (piece_type == BLACK_ROOK || piece_type == WHITE_ROOK) {
				enemy = (piece_type == BLACK_ROOK) ? board->white : board->black;
				board->possible_moves = single_rook_move((1ULL << tile_selected), board->occupied, enemy);
			}
			else {
				board->possible_moves = 0;
			}
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
