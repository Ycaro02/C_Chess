#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/handle_signal.h"
#include "../include/chess_log.h"

#ifdef _EMSCRIPTEN_VERSION_
	#include <emscripten.h>
	
	void	set_local_info(SDLHandle *h);
	void	local_chess_routine();

	void emscripten_setup() {
		SDLHandle *h = get_SDL_handle();

		set_local_info(h);
		emscripten_set_main_loop(local_chess_routine, 0, 1);
	}
#endif


void set_local_info(SDLHandle *h) {
	h->player_info.turn = TRUE;
	h->game_start = TRUE;
	h->player_info.piece_start = WHITE_PAWN;
	h->player_info.piece_end = BLACK_KING;
}


void chess_signal_handler(int signum)
{
	SDLHandle *stat = get_SDL_handle();

	CHESS_LOG(LOG_INFO, RED"\nSignal Catch: %d\n"RESET, signum);
	chess_destroy(stat);
	exit(signum);
}

SDLHandle *init_game() {
	SDLHandle	*handle = NULL;
	handle = create_sdl_handle("C_Chess");
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "create_sdl_handle failed\n");
		return (NULL);
	}
	handle->board = ft_calloc(1, sizeof(ChessBoard));
	if (!handle->board) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		free(handle);
		return (NULL);
	}
	init_board(handle->board);

	return (handle);
}

/*
 * @brief Main chess routine
 * @note This function is called in a loop
*/
void local_chess_routine() {
	SDLHandle	*h = get_SDL_handle();
	ChessBoard	*b = h->board;
	s32			event = 0;
	
	event = event_handler(h, h->player_info.color);
	/* If the quit button is pressed */
	if (event == CHESS_QUIT) { chess_destroy(h) ; }
	
	if (has_flag(h->flag, FLAG_PROMOTION_SELECTION)) {
		pawn_selection_event(h);
	} else if (b->last_clicked_tile != INVALID_TILE) {
		/* If a piece is selected and the tile selected is a possible move */
		if (is_selected_possible_move(b->possible_moves, b->last_clicked_tile)) {
			move_piece(h, b->selected_tile, b->last_clicked_tile, b->selected_piece);
			b->possible_moves = 0;
			h->over_piece_select = EMPTY;
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
	} /* End if not invalid tile */

	/* Draw logic */
	update_graphic_board(h);
}

void chess_destroy(SDLHandle *h) {
	CHESS_LOG(LOG_INFO, RED"Destroy chess game%s\n", RESET);

	// print_call_stack();

	register_data(DATA_SAVE_FILE, h->player_info.name, h->player_info.dest_ip);

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

void chess_game(SDLHandle *h) {
	struct timeval	timeout = {0, 10000}; /* 10000 microseconds = 0.01 seconds */
	
	INIT_SIGNAL_HANDLER(chess_signal_handler);
	update_graphic_board(h);

	if (has_flag(h->flag, FLAG_NETWORK)) {
		CHESS_LOG(LOG_INFO, ORANGE"Try to connect to Server at : %s:%d\n"RESET, h->player_info.dest_ip, SERVER_PORT);
		// network_setup(h, h->flag, &h->player_info, h->player_info.dest_ip);
		h->player_info.nt_info = init_network(h->player_info.dest_ip, h->player_info.name, timeout);
		handle_network_client_state(h, h->flag, &h->player_info);
		h->game_start = TRUE;
		while (1) {
			network_chess_routine(h);
		}
	} else {
		set_local_info(h);
		while (1) {
			local_chess_routine();
		}
	}
}

void update_data_from_file(SDLHandle *h) {
	char *nickname = get_file_data(DATA_SAVE_FILE, "Nickname", 0, 8);
	if (nickname) {
		h->player_info.name = nickname;
	} else {
		h->player_info.name = ft_strdup("Default");
	}
	if (h->menu.profile->tf[PFT_NAME]->text) {
		free(h->menu.profile->tf[PFT_NAME]->text);
		h->menu.profile->tf[PFT_NAME]->text = ft_strdup(h->player_info.name);
	}
	char *ip = get_file_data(DATA_SAVE_FILE, "Server", 1, 15);
	if (ip) {
		CHESS_LOG(LOG_INFO, "Get In File Server IP: %s\n", ip);
		h->player_info.dest_ip = ip;
		if (h->menu.ip_field->text) {
			free(h->menu.ip_field->text);
			h->menu.ip_field->text = ft_strdup(h->player_info.dest_ip);
		}
	}
}

// int SDL_main(int argc, char **argv) {
int main(int argc, char **argv) {
	SDLHandle	*handle = NULL;
	PlayerInfo	player_info = {0};
	u32			flag = 0;
	s8			error = 0;


	// set_log_level(LOG_DEBUG);
	// set_log_level(LOG_INFO);
	// set_log_level(LOG_ERROR);
	set_log_level(LOG_NONE);

	flag = handle_chess_flag(argc, argv, &error, &player_info);
	if (error == -1) {
		return (1);
	}


	handle = get_SDL_handle();
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "%s: get_SDL_handle failed init\n", __func__);
		return (1);
	}
	handle->flag = flag;
	handle->player_info = player_info;
	update_data_from_file(handle);

	#ifdef _EMSCRIPTEN_VERSION_
		emscripten_setup();
	#else
		chess_game(handle);	
	#endif
	

	/* Free memory */
	chess_destroy(handle);
	return (0);
}