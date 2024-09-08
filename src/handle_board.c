#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

/**
 * @brief Detect click tile on the board
 * @param handle The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
ChessTile detect_tile_click(s32 x, s32 y, s32 tile_size, WinBand wb, s8 player_color) {
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s32 		column = 7;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			if (is_in_x_range(x, raw, tile_size, wb) && is_in_y_range(y, column, tile_size, wb)) {
				CHESS_LOG(LOG_INFO, "Click on "ORANGE"[%s]"RESET" -> "PINK"|%d|\n"RESET, ChessTile_to_str(tile), tile);
				return (tile);
			}
			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
	}
	return (INVALID_TILE);
}

void reset_selected_tile(SDLHandle *h) {
	h->board->selected_tile = INVALID_TILE;
	h->board->selected_piece = EMPTY;
	h->board->possible_moves = 0;
	h->over_piece_select = EMPTY;
}


void update_mouse_pos(SDLHandle *h, s32 x, s32 y) {
	h->mouse_pos.x = x;
	h->mouse_pos.y = y;
}

void button_event_handling(SDLHandle *h, SDL_Event event, s32 btn_start, s32 btn_nb) {
	iVec2 pos = {0, 0};
	BtnType btn_click = BTN_INVALID;

	SDL_GetMouseState(&pos.x, &pos.y);
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		btn_click = detect_button_click(h->menu.btn, btn_start, btn_nb, pos);
		if (btn_click != BTN_INVALID && h->menu.btn[btn_click].state != BTN_STATE_DISABLED) {
			h->menu.btn[btn_click].state = BTN_STATE_PRESSED;
			h->menu.current_btn_clicked = btn_click;
		}
	} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
		btn_click = detect_button_click(h->menu.btn, btn_start, btn_nb, pos);
		if (h->menu.current_btn_clicked != BTN_INVALID) {
			h->menu.btn[h->menu.current_btn_clicked].state = BTN_STATE_RELEASED;
		}
		if (btn_click == h->menu.current_btn_clicked && btn_click != BTN_INVALID) {
			if (h->menu.btn[btn_click].func) {
				h->menu.current_btn_clicked = BTN_INVALID;
				h->menu.btn[btn_click].func(h);
			}
			if (btn_start == BTN_RESUME && btn_click != BTN_SERVER_IP) {
				CHESS_LOG(LOG_INFO, "Button %d clicked close menu\n", btn_click);
				menu_close(&h->menu);
			}
		}
	} else if (btn_start == BTN_RESUME && event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_p || event.key.keysym.sym == SDLK_ESCAPE)) {
		menu_close(&h->menu);
	} else if (event.type == SDL_MOUSEMOTION) {
		update_mouse_pos(h, pos.x, pos.y);
		h->menu.btn_hover = detect_button_click(h->menu.btn, btn_start, btn_nb, pos);
	}
}

void game_event_handling(SDLHandle *h, SDL_Event event, s8 player_color) {
	ChessPiece piece_select = EMPTY;
	Bitboard aly_pos = h->over_piece_select >= BLACK_PAWN ? h->board->black : h->board->white;;
	s32 x = 0, y = 0;

	
	/* For local mode, aly is all occupied tile */
	if (!has_flag(h->flag, FLAG_NETWORK)) {
		aly_pos = h->board->occupied;
	}

	if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_p || event.key.keysym.sym == SDLK_ESCAPE)) {
		h->menu.is_open = TRUE;
	}

	if (h->player_info.turn == FALSE) { return ; }
	SDL_GetMouseState(&x, &y);
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		h->board->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
		piece_select = get_piece_from_tile(h->board, h->board->last_clicked_tile);
		if (!has_flag(h->flag, FLAG_PROMOTION_SELECTION) \
			&& (piece_select >= h->player_info.piece_start && piece_select <= h->player_info.piece_end)) {
			h->over_piece_select = piece_select;
		}
	} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
		if (!has_flag(h->flag, FLAG_PROMOTION_SELECTION) && h->over_piece_select != EMPTY) {
			h->board->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
			
			if (h->board->last_clicked_tile == INVALID_TILE) {
				h->over_piece_select = EMPTY;
				h->board->possible_moves = 0;
			} 
			else if (h->board->last_clicked_tile == h->board->selected_tile || ((1ULL << h->board->last_clicked_tile) & aly_pos) != 0) {
				h->over_piece_select = EMPTY;
			} 
		}
	} else if (event.type == SDL_MOUSEMOTION) {
		update_mouse_pos(h, x, y);
	}
}

/**
 * @brief Chess event handler
 * @return The tile clicked, or CHESS_QUIT if the user want to quit
 * @note Return the clicled tile or INVALID_TILE if no tile is clicked
*/
s32 event_handler(SDLHandle *h, s8 player_color) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return (CHESS_QUIT);
		}
		if (has_flag(h->flag, FLAG_CENTER_TEXT_INPUT)) {
			button_event_handling(h, event, BTN_CENTER1, h->center_text->curent_btn_enable);
			// center_text_event_handling(h, event);
		} else {
			if (!h->menu.is_open) {
				game_event_handling(h, event, player_color);
			} else {
				if (h->menu.ip_field.is_active) {
					handle_text_input(h, &event);
				}
				button_event_handling(h, event, BTN_RESUME, h->menu.menu_btn + 1);
				// menu_event_handling(h, event);
			}
		}
	}
	return (TRUE);
}