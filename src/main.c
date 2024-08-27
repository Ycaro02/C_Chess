#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/handle_signal.h"


SDLHandle *init_game() {
	SDLHandle	*handle = NULL;

	handle = create_sdl_handle(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
	if (!handle) {
		ft_printf_fd(2, "Error %s: create_sdl_handle failed\n", __func__);
		return (NULL);
	}
	handle->board = ft_calloc(1, sizeof(ChessBoard));
	if (!handle->board) {
		ft_printf_fd(2, "Error %s : malloc failed\n", __func__);
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
	// ChessTile	tile_selected = INVALID_TILE;
	ChessBoard	*b = h->board;
	s32			ret = TRUE;
	s32			event = 0;
	
	while (1) {
		event = event_handler(h, h->player_info.color);
		/* If the quit button is pressed */
		if (event == CHESS_QUIT) { break ; }
		
		/* If tile is selected */
		if (b->last_clicked_tile != INVALID_TILE) {
			/* If a piece is selected and the tile selected is a possible move */
			if (is_selected_possible_move(b->possible_moves, b->last_clicked_tile)) {
				ret = move_piece(h, b->selected_tile, b->last_clicked_tile, b->selected_piece);
				b->possible_moves = 0;
				h->over_piece_select = EMPTY;
				if (ret == CHESS_QUIT) { break ; }
			} 
			else { /* Update piece possible move and selected tile */
				if (h->over_piece_select != EMPTY) {
					b->selected_piece = get_piece_from_tile(b, b->last_clicked_tile);
					b->selected_tile = b->last_clicked_tile;
					b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), b->selected_piece, TRUE);
					if (b->possible_moves == 0) { h->over_piece_select = EMPTY ; }
				} else { /* if over piece select is empty */
					reset_selected_tile(h);
				}
			}
		} /* End if invalid tile */

		/* Draw logic */
		update_graphic_board(h);
	}
}



void chess_destroy(SDLHandle *h) {
	ft_printf_fd(1, RED"Destroy chess game%s\n", RESET);
	destroy_sdl_handle(h);
	free(h);
	CLEANUP_NETWORK();
}

/* Singleton to get the SDL handle pointer structure */
SDLHandle *get_SDL_handle() {
	static SDLHandle *stat = NULL;

	if (!stat) {
		stat = init_game();
	}

	return (stat);
}

void chess_game(SDLHandle *h) {
	
	INIT_SIGNAL_HANDLER();
	
	update_graphic_board(h);

	if (has_flag(h->flag, FLAG_NETWORK)) {
		ft_printf_fd(1, ORANGE"Try to connect to Server at : %s:%d\n"RESET, h->player_info.dest_ip, SERVER_PORT);
		network_setup(h, h->flag, &h->player_info, h->player_info.dest_ip);
		network_chess_routine(h);
	} else {
		h->player_info.turn = TRUE;
		h->player_info.piece_start = WHITE_PAWN;
		h->player_info.piece_end = BLACK_KING;
		chess_routine(h);
	}
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

	handle = get_SDL_handle();
	if (!handle) {
		ft_printf_fd(2, "Error %s: get_SDL_handle failed init\n", __func__);
		return (1);
	}

	handle->flag = flag;
	handle->player_info = player_info;

	chess_game(handle);

	/* Free memory */
	chess_destroy(handle);
	return (0);
}



// ft_printf_fd(1, YELLOW"Move piece from [%s](%d) TO [%s](%d)\n"RESET, TILE_TO_STRING(b->selected_tile), b->selected_tile, TILE_TO_STRING(tile_selected), tile_selected);
// ft_printf_fd(1, GREEN"Select piece in [%s]"RESET" -> "ORANGE"%s\n"RESET, TILE_TO_STRING(tile_selected), chess_piece_to_string(piece_type));



