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
	Bitboard control = 0, possible_moves = 0, enemy_pieces = 0, piece = 0;
    ChessPiece enemy_piece_start = is_black ? BLACK_PAWN : WHITE_PAWN;
    ChessPiece enemy_piece_end = is_black ? PIECE_MAX : BLACK_PAWN;

    for (ChessPiece type = enemy_piece_start; type < enemy_piece_end; type++) {
        enemy_pieces = b->piece[type];

		/* For each enemy piece */
        while (enemy_pieces) {
			/* Get the first bit set */
            piece = enemy_pieces & -enemy_pieces;
            
			/* Clear the first bit set */
			enemy_pieces &= enemy_pieces - 1;

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
				ft_printf_fd(1, YELLOW"Move piece from [%s] TO [%s]\n"RESET, TILE_TO_STRING(board->selected_tile), TILE_TO_STRING(tile_selected));
				move_piece(board, board->selected_tile, tile_selected, piece_type);
				board->possible_moves = 0;
			} else {
				piece_type = get_piece_from_tile(board, tile_selected);
				board->selected_tile = tile_selected;
				ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));
				board->possible_moves = get_piece_move(board, (1ULL << board->selected_tile), piece_type);
			}
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
