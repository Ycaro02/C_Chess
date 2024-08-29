#include "../include/network.h"
#include "../include/handle_sdl.h"

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
static s32 network_move_piece(SDLHandle *h, ChessTile last_tile_click) {
	ChessBoard	*b = h->board;
	s32			ret = FALSE;

	/* If a piece is selected and the tile selected is a possible move */
	if (is_selected_possible_move(b->possible_moves, last_tile_click)) {
		// piece_type = get_piece_from_tile(b, b->selected_tile);
		ret = move_piece(h, b->selected_tile, last_tile_click, b->selected_piece);
		b->possible_moves = 0;
		h->over_piece_select = EMPTY;
		
		update_graphic_board(h);

		/* Build move message to the other player if is not pawn promotion or chess quit */
		if (ret == TRUE) {
			build_message(h, h->player_info.msg_tosend, MSG_TYPE_MOVE, b->selected_tile, last_tile_click, b->selected_piece);
		}

		h->player_info.turn = FALSE;
		update_graphic_board(h);

		/* Send the message to the other player */
		if (!safe_msg_send(h)) {
			return (ret);
		}

		reset_selected_tile(h);
		update_graphic_board(h);
		return (ret);
	} else { /* Update piece possible move and selected tile */
		piece_update_move(h, b, last_tile_click);
	}
	return (TRUE);
}

/**
 * @brief Network chess routine
 * @param h The SDLHandle pointer
 */
void network_chess_routine(SDLHandle *h) {
	s32			ret = FALSE, event = 0;
	s8			msg_recv = FALSE;
	
	h->game_start = TRUE;

	while (1) {

		/* Event handler */
		event = event_handler(h, h->player_info.color);
		/* If the quit button is pressed */
		if (event == CHESS_QUIT) { break ; } // Send quit message to the other player
		
		if (h->player_info.nt_info->peer_conected == FALSE) {
			msg_recv = wait_peer_info(h->player_info.nt_info, "Wait reconnect peer info");
			if (msg_recv) {
				/* Here we need to build MSG_TYPE_RECONECT and while on the list of move to give them to the reconnected client */
				build_message(h, h->player_info.msg_tosend, MSG_TYPE_COLOR, !h->player_info.color, 0, 0);
				u16 msg_size = 0;
				char *buff = build_reconnect_message(h, &msg_size);
				chess_msg_send(h->player_info.nt_info, buff, msg_size);
				h->player_info.nt_info->peer_conected = TRUE;
			}
		} else {
			/* If tile is selected and is player turn, try to send move piece and move it */
			if (h->player_info.turn == TRUE && h->board->last_clicked_tile != INVALID_TILE) {
				ret = network_move_piece(h, h->board->last_clicked_tile);
				if (ret == CHESS_QUIT || ret == FALSE) { break ; }
			}

			/* Receive message from the other player */
			msg_recv = chess_msg_receive(h, h->player_info.nt_info, h->player_info.msg_receiv, h->player_info.last_msg);
			if ((!h->player_info.turn && msg_recv) || (h->player_info.turn && msg_recv && h->player_info.msg_receiv[IDX_TYPE] == MSG_TYPE_QUIT)) {
				process_message_receive(h, h->player_info.msg_receiv);
			}
		}

		/* Draw logic */
		update_graphic_board(h);

		SDL_Delay(16);

	}
}
