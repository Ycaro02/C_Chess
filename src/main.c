#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"


SDLHandle *init_game() {
	SDLHandle	*handle = NULL;

	handle = create_sdl_handle(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
	if (!handle) {
		ft_printf_fd(2, "Error: create_sdl_handle failed\n");
		return (NULL);
	}
	handle->board = ft_calloc(1, sizeof(ChessBoard));
	if (!handle->board) {
		ft_printf_fd(2, "Error: malloc failed\n");
		free(handle);
		return (NULL);
	}
	init_board(handle->board);
	handle->player_info.color = IS_WHITE;
	handle->player_info.turn = FALSE;
	handle->over_piece_select = EMPTY;
	return (handle);
}

void chess_routine(SDLHandle *h){
	ChessTile	tile_selected = INVALID_TILE;
	ChessBoard	*b = h->board;
	s32			ret = TRUE;
	
	while (1) {
		tile_selected = event_handler(h, h->player_info.color);
		/* If the quit button is pressed */
		if (tile_selected == CHESS_QUIT) { break ; }
		
		/* If tile is selected */
		if (tile_selected != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (is_selected_possible_move(b->possible_moves, tile_selected)) {
				ret = move_piece(h, b->selected_tile, tile_selected, b->selected_piece);
				b->possible_moves = 0;
				h->over_piece_select = EMPTY;
				if (ret == CHESS_QUIT) { break ; }
			} 
			else { /* Update piece possible move and selected tile */
				if (h->over_piece_select != EMPTY) {
					b->selected_piece = get_piece_from_tile(b, tile_selected);
					b->selected_tile = tile_selected;
					b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), b->selected_piece, TRUE);
					if (b->possible_moves == 0) {
						h->over_piece_select = EMPTY;
					}
				} else { /* if over piece select is empty */
					reset_selected_tile(h);
				}
			}
		} /* End if invalid tile */

		/* Draw logic */
		update_graphic_board(h);
	}

	/* Free memory */
	destroy_sdl_handle(h);
	window_close(h->window, h->renderer);
	free(h);
}


int main(int argc, char **argv) {
	SDLHandle	*handle = NULL;
	PlayerInfo	player_info = {0};
	u32			flag = 0;
	s8			error = 0;

	flag = handle_chess_flag(argc, argv, &error, &player_info);
	if (error == -1) {
		return (1);
	}
	
	handle = init_game();
	if (!handle) {
		return (1);
	}
	handle->player_info = player_info;

	if (has_flag(flag, FLAG_NETWORK)) {
		ft_printf_fd(1, ORANGE"Try to connect to Server at : %s:%d\n"RESET, player_info.dest_ip, SERVER_PORT);
		network_setup(handle, flag, &handle->player_info, player_info.dest_ip);
		network_chess_routine(handle);
	} else {
		handle->player_info.turn = TRUE;
		handle->player_info.piece_start = WHITE_PAWN;
		handle->player_info.piece_end = BLACK_KING;
		chess_routine(handle);
	}
	return (0);
}



// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));



