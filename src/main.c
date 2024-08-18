#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unloadTexture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
}


Bitboard get_piece_color_control(ChessBoard *b, s8 is_black) {
    ChessPiece enemy_piece_start = is_black ? BLACK_PAWN : WHITE_PAWN;
    ChessPiece enemy_piece_end = is_black ? PIECE_MAX : BLACK_PAWN;
	Bitboard control = 0, possible_moves = 0;

    for (ChessPiece type = enemy_piece_start; type < enemy_piece_end; type++) {
        Bitboard enemy_pieces = b->piece[type];

		/* For each enemy piece */
        while (enemy_pieces) {
			/* Get the first piece */
            Bitboard piece = enemy_pieces & -enemy_pieces; /* Get the first bit set */
            enemy_pieces &= enemy_pieces - 1;  /* Clear the first bit set */

			/* Get the possible moves */
            if (type == enemy_piece_start) {
				/* If the piece is a pawn, get only the pawn attacks moves */
				possible_moves = get_pawn_moves(piece, b->occupied, enemy_pieces, type == BLACK_PAWN, TRUE);
			} else {
				possible_moves = get_piece_move(b, piece, type);
			}
			/* Add the possible moves to the control bitboard */
			control |= possible_moves;
        }
    }
	return (control);
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

	display_bitboard(board->white, "White");
	display_bitboard(board->black, "Black");
	display_bitboard(board->white_control, "White Control");
	display_bitboard(board->black_control, "Black Control");
	

	// Bitboard enemy = 0, piece = 0;
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
			piece_type = get_piece_from_tile(board, tile_selected);
			board->possible_moves = get_piece_move(board, (1ULL << tile_selected), piece_type);
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
