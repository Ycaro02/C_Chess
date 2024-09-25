#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/handle_signal.h"
#include "../include/chess_log.h"

#ifdef _EMSCRIPTEN_VERSION_
	#include <emscripten.h>
	
	void	set_local_info(SDLHandle *h);

	void emscripten_setup() {
		SDLHandle *h = get_SDL_handle();

		set_local_info(h);
		emscripten_set_main_loop(local_chess_routine, 0, 1);
	}
#endif

void chess_start_program(void);

void set_local_info(SDLHandle *h) {
	h->player_info.turn = TRUE;
	h->game_start = TRUE;
	h->player_info.color = IS_WHITE;
	h->player_info.piece_start = WHITE_PAWN;
	h->player_info.piece_end = WHITE_KING;
}


void reset_board(SDLHandle *h) {
	set_local_info(h);
	init_board(h->board, &h->flag);
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
	init_board(handle->board, &handle->flag);

	return (handle);
}

/*
 * @brief Main chess routine
 * @note This function is called in a loop
*/
void local_chess_routine() {
	SDLHandle	*h = get_SDL_handle();
	s32			event = 0;
	
	event = event_handler(h, h->player_info.color);
	/* If the quit button is pressed */
	if (event == CHESS_QUIT) { chess_destroy(h) ; }
	
	if (has_flag(h->flag, FLAG_PROMOTION_SELECTION)) {
		pawn_selection_event(h);
	} 
	/* Draw logic */
	update_graphic_board(h);
}



void chess_destroy(SDLHandle *h) {
	CHESS_LOG(LOG_INFO, RED"Destroy chess game%s\n", RESET);

	register_data(h, DATA_SAVE_FILE);

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

	/* Set local info and local chess routine func */
	set_local_info(h);
	h->routine_func = local_chess_routine;

	if (has_flag(h->flag, FLAG_NETWORK)) {
		CHESS_LOG(LOG_INFO, ORANGE"Try to connect to Server at : %s:%d\n"RESET, h->player_info.dest_ip, SERVER_PORT);
		center_text_string_set(h, "Reconnect game on:", h->player_info.dest_ip);
		/* Init network and player state */
		h->player_info.nt_info = init_network(h->player_info.dest_ip, h->player_info.name, timeout);
		/* Wait for player */
		if (wait_player_handling(h)) {
			start_network_game(h);
		}
	} 

	while (1) {
		h->routine_func();
	}


}

void update_data_from_file(SDLHandle *h) {
	
	/* handle nickname */
	char *nickname = get_file_data(DATA_SAVE_FILE, "Nickname", 0, 8);
	if (nickname) {
		h->player_info.name = nickname;
	} else {
		h->player_info.name = ft_strdup("Default");
	}
	if (h->menu.profile->tf[PFT_NAME]->text) {
		fast_bzero(h->menu.profile->tf[PFT_NAME]->text, NICKNAME_MAX_LEN);
		ft_memcpy(h->menu.profile->tf[PFT_NAME]->text, h->player_info.name, fast_strlen(h->player_info.name));
	}

	/* handle server ip */
	char *ip = get_file_data(DATA_SAVE_FILE, "Server", 1, 15);
	if (ip) {
		CHESS_LOG(LOG_INFO, "Get In File Server IP: %s\n", ip);
		h->player_info.dest_ip = ip;
	} else {
		h->player_info.dest_ip = ft_strdup("127.0.0.1");
	}
	if (h->menu.ip_field->text) {
		ft_memcpy(h->menu.ip_field->text, h->player_info.dest_ip, fast_strlen(h->player_info.dest_ip));
	}
	/* handle auto reconnect */
	char *network_pause = get_file_data(DATA_SAVE_FILE, "NetworkPause", 2, 2);
	if (network_pause && network_pause[0] == '1') {
		CHESS_LOG(LOG_INFO, CYAN"NetworkPause: %s\n"RESET, network_pause);
		set_flag(&h->flag, FLAG_NETWORK);
		set_flag(&h->flag, FLAG_RECONNECT);
	}
	free(network_pause);
}

void chess_start_program() {
	SDLHandle	*h = NULL;
	PlayerInfo	player_info = {0};

	// set_log_level(LOG_DEBUG);
	set_log_level(LOG_INFO);
	// set_log_level(LOG_ERROR);
	// set_log_level(LOG_NONE);

	h = get_SDL_handle();
	if (!h) {
		CHESS_LOG(LOG_ERROR, "%s: get_SDL_handle failed init\n", __func__);
		return ;
	}
	fast_bzero(&player_info, sizeof(PlayerInfo));
	player_info.dest_port = SERVER_PORT;
	h->flag = 0;
	h->player_info = player_info;
	update_data_from_file(h);

	#ifdef _EMSCRIPTEN_VERSION_
		emscripten_setup();
	#else
		chess_game(h);	
	#endif

	chess_destroy(h);
}

int main(int argc, char **argv) {
	// s8			error = 0;
	// u32			flag = 0;
	// flag = handle_chess_flag(argc, argv, &error, &player_info);
	// if (error == -1) {
	// 	return (1);
	// }

	(void)argc, (void)argv;
	chess_start_program();
	return (0);
}