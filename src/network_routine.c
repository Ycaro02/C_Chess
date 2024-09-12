#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

static void piece_update_move(SDLHandle *h, ChessBoard *b, ChessTile last_tile_click) {
	if (h->over_piece_select != EMPTY) {
		b->selected_piece = get_piece_from_tile(b, last_tile_click);
		if (b->selected_piece >= h->player_info.piece_start && b->selected_piece <= h->player_info.piece_end) {
			b->selected_tile = last_tile_click;
			b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), b->selected_piece, TRUE);
			if (b->possible_moves == 0) {
				h->over_piece_select = EMPTY;
			}
			return ;
		} 
		reset_selected_tile(h);
		return ;
	} 
	reset_selected_tile(h);
}

/**
 * @brief Move piece logic for network game
 * @param h 
 * @param tile_selected 
 * @param piece_type 
 * @return s32 FALSE if we can't send the message, CHESS_QUIT if the player quit the game, TRUE otherwise 
 */
static s32 handle_player_event(SDLHandle *h, ChessTile last_tile_click) {
	ChessBoard	*b = h->board;
	s32			ret = FALSE;

	if (has_flag(h->flag, FLAG_PROMOTION_SELECTION)) {
		pawn_selection_event(h);
	} else if (is_selected_possible_move(b->possible_moves, last_tile_click)) {
		// piece_type = get_piece_from_tile(b, b->selected_tile);
		ret = move_piece(h, b->selected_tile, last_tile_click, b->selected_piece);
		b->possible_moves = 0;
		h->over_piece_select = EMPTY;
		
		update_graphic_board(h);


		/* Build move message to the other player if is not pawn promotion or chess quit */
		if (ret != PAWN_PROMOTION) {
			h->player_info.turn = FALSE;
			build_message(h, h->player_info.msg_tosend, MSG_TYPE_MOVE, b->selected_tile, last_tile_click, b->selected_piece);
			if (!safe_msg_send(h)) {
				return (ret);
			}
		}

		update_graphic_board(h);
		/* Send the message to the other player */

		reset_selected_tile(h);
		update_graphic_board(h);
		return (ret);
	} else { /* Update piece possible move and selected tile */
		piece_update_move(h, b, last_tile_click);
	}
	return (TRUE);
}

void send_alive_packet(NetworkInfo *info) {
	static		u64 last_alive_send = 0;
	u64			now = get_time_sec();
	u64			elapsed_time = now - last_alive_send;

	if (elapsed_time >= SEND_ALIVE_DELAY) {
		send_alive_to_server(info->sockfd, info->servaddr);
		last_alive_send = now;
	}
}

s8 reconnect_handling(SDLHandle *h) {
	center_text_string_set(h, "Wait peer reconnection", NULL);
	if (!has_flag(h->flag, FLAG_CENTER_TEXT_INPUT)) {
		set_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	}
	
	if (!has_flag(h->flag, FLAG_NETWORK)) {
		// instead of just return, we should display a new center text
		// to ask user if he really want to quit the application and destroy the game
		return (FALSE);
	}
 	if (wait_peer_info(h->player_info.nt_info, "Wait reconnect peer info")) {
		u16 msg_size = 0;
		char *buff = build_reconnect_message(h->board->lst, &msg_size, h->my_remaining_time, h->enemy_remaining_time, h->msg_id, h->player_info.color);
		chess_msg_send(h->player_info.nt_info, buff, msg_size);
		h->player_info.nt_info->peer_conected = TRUE;
		center_text_string_set(h, NULL, NULL);
		unset_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	}
	return (TRUE);
}

/**
 * @brief Network chess routine
 * @param h The SDLHandle pointer
 */
void network_chess_routine(SDLHandle *h) {
	s32			event = 0;
	s8			msg_recv = FALSE;
	
	/* Event handler */
	event = event_handler(h, h->player_info.color);
	
	/* If the quit button is pressed */
	if (event == CHESS_QUIT) { chess_destroy(h) ; }

	/* If the game is started */
	if (h->game_start && h->player_info.nt_info != NULL) {
		if (!h->player_info.nt_info->peer_conected) {
			if (!reconnect_handling(h)) { chess_destroy(h) ;}
		} else {
			/* If tile is selected and is player turn, try to send move piece and move it */
			if (h->player_info.turn == TRUE && h->board->last_clicked_tile != INVALID_TILE) {
				handle_player_event(h, h->board->last_clicked_tile);
			}
			/* Receive message from the other player */
			msg_recv = chess_msg_receive(h, h->player_info.nt_info, h->player_info.msg_receiv);
			if ((!h->player_info.turn && msg_recv) || (h->player_info.turn && msg_recv && h->player_info.msg_receiv[IDX_TYPE] == MSG_TYPE_QUIT)) {
				process_message_receive(h, h->player_info.msg_receiv);
			}
		}
	}

	if (h->player_info.nt_info == NULL) {
		CHESS_LOG(LOG_INFO, RED"Network info is NULL call Chess destroy\n"RESET);
		chess_destroy(h);
	}

	/* Draw logic */
	update_graphic_board(h);

	/* Send alive message to the server */
	send_alive_packet(h->player_info.nt_info);
	SDL_Delay(16);

}
