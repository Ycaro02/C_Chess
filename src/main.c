#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/handle_signal.h"
#include "../include/chess_log.h"


SDLHandle *init_game() {
	SDLHandle	*handle = NULL;
	handle = create_sdl_handle("C_Chess");
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "Error %s: create_sdl_handle failed\n", __func__);
		return (NULL);
	}
	handle->board = ft_calloc(1, sizeof(ChessBoard));
	if (!handle->board) {
		CHESS_LOG(LOG_ERROR, "Error %s : malloc failed\n", __func__);
		free(handle);
		return (NULL);
	}
	init_board(handle->board);
	handle->board->turn = 1;

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
	CHESS_LOG(LOG_INFO, RED"Destroy chess game%s\n", RESET);

	// if (h->player_info.nt_info && h->player_info.nt_info->peer_conected) {
	// 	build_message(h, h->player_info.msg_tosend, MSG_TYPE_QUIT, 0, 0, 0);
	// 	chess_msg_send(h->player_info.nt_info, h->player_info.msg_tosend, MSG_SIZE);
	// }
	if (h->board->lst) {
		ft_lstclear(&h->board->lst, free);
	}

	destroy_menu(h);
	destroy_sdl_handle(h);
	free(h);
	CLEANUP_NETWORK();
	exit(0);
}

/* Singleton to get the SDL handle pointer structure */
SDLHandle *get_SDL_handle() {
	static SDLHandle *stat = NULL;

	if (!stat) {
		stat = init_game();
	}

	return (stat);
}

/**
 * When player click on search game button : we enable the flag network (if not already set)
 *	- We need to send a message to the server to search for a game
 *	- We need to wait for the server to send us if we are the first player or the second player
 * 		- If he is the first player, he will wait for the second player (flag_listen)
 * 		- If he is the second player, the first player will send him the color (flag_join)
 
 *	When player click on reconnect game button : we enable the flag network (if not already set)
 *	- We enable the reconnect flag too
 *	- Then we send a message to the server to get the game state from the first player
 */





void chess_game(SDLHandle *h) {
	struct timeval	timeout = {0, 10000}; /* 10000 microseconds = 0.01 seconds */
	
	INIT_SIGNAL_HANDLER();
	update_graphic_board(h);

	if (has_flag(h->flag, FLAG_NETWORK)) {
		CHESS_LOG(LOG_INFO, ORANGE"Try to connect to Server at : %s:%d\n"RESET, h->player_info.dest_ip, SERVER_PORT);
		// network_setup(h, h->flag, &h->player_info, h->player_info.dest_ip);
		h->player_info.nt_info = init_network(h->player_info.dest_ip, timeout);
		handle_network_client_state(h, h->flag, &h->player_info);
		network_chess_routine(h);
	} else {
		h->player_info.turn = TRUE;
		h->game_start = TRUE;
		h->player_info.piece_start = WHITE_PAWN;
		h->player_info.piece_end = BLACK_KING;
		chess_routine(h);
	}
}

#include "../include/chess_log.h"

int main(int argc, char **argv) {
	SDLHandle	*handle = NULL;
	PlayerInfo	player_info = {0};
	u32			flag = 0;
	s8			error = 0;


	// set_log_level(LOG_ERROR);

	set_log_level(LOG_INFO);


	flag = handle_chess_flag(argc, argv, &error, &player_info);
	if (error == -1) {
		return (1);
	}
	

	handle = get_SDL_handle();
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "Error %s: get_SDL_handle failed init\n", __func__);
		return (1);
	}

	handle->flag = flag;
	handle->player_info = player_info;

	chess_game(handle);

	/* Free memory */
	chess_destroy(handle);
	return (0);
}