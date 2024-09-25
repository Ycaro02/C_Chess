#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"
#include "../include/FEN_notation.h"

FT_INLINE s8 is_left_click_down(SDL_Event event) {
	return (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT);
}

FT_INLINE s8 is_left_click_up(SDL_Event event) {
	return (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT);
}

FT_INLINE s8 promotion_enable(u32 flag) {
	return (has_flag(flag, FLAG_PROMOTION_SELECTION));
}

FT_INLINE s8 piece_in_range(SDLHandle *h, ChessPiece piece) {
	return (piece >= h->player_info.piece_start && piece <= h->player_info.piece_end);
}

FT_INLINE s8 is_locale_mode(u32 flag) {
	return (!has_flag(flag, FLAG_NETWORK));
}

FT_INLINE s8 is_key_pressed(SDL_Event event, s32 key) {
	return (event.type == SDL_KEYDOWN && event.key.keysym.sym == key);
}

FT_INLINE void handle_locale_turn(SDLHandle *h) {
	if (h->player_info.piece_start == WHITE_PAWN) {
		h->player_info.piece_start = BLACK_PAWN;
		h->player_info.piece_end = BLACK_KING;
		// h->player_info.color = IS_BLACK;
	} else {
		h->player_info.piece_start = WHITE_PAWN;
		h->player_info.piece_end = WHITE_KING;
		// h->player_info.color = IS_WHITE;
	}
}

/**
 * @brief Detect click tile on the board
 * @param handle The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
static ChessTile detect_tile_click(s32 x, s32 y, s32 tile_size, WinBand wb, s8 player_color) {
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s32 		column = 7;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			if (is_in_x_range(x, raw, tile_size, wb) && is_in_y_range(y, column, tile_size, wb)) {
				CHESS_LOG(LOG_DEBUG, "Click on "ORANGE"[%s]"RESET" -> "PINK"|%d|\n"RESET, ChessTile_to_str(tile), tile);
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


static void update_mouse_pos(SDLHandle *h, s32 x, s32 y) {
	h->mouse_pos.x = x;
	h->mouse_pos.y = y;
}

static void profile_button_handling(SDLHandle *h, SDL_Event event) {
	iVec2 				pos = {0, 0};
	ProfileFieldType	btn_click = PFT_INVALID;
	Profile				*p = h->menu.profile;


	SDL_GetMouseState(&pos.x, &pos.y);
	if (is_left_click_down(event)) {
		btn_click = detect_button_click(p->btn, 0, p->nb_field, pos);
		if (btn_click != PFT_INVALID && p->btn[btn_click].state != BTN_STATE_DISABLED) {
			p->btn[btn_click].state = BTN_STATE_PRESSED;
			p->current_btn_clicked = btn_click;
		}
	} else if (is_left_click_up(event)) {
		btn_click = detect_button_click(p->btn, 0, p->nb_field, pos);
		if (p->current_btn_clicked != PFT_INVALID) {
			p->btn[p->current_btn_clicked].state = BTN_STATE_RELEASED;
		}
		if (btn_click == p->current_btn_clicked && btn_click != PFT_INVALID) {
			if (p->btn[btn_click].func) {
				p->current_btn_clicked = PFT_INVALID;
				p->btn[btn_click].func(h);
			}
		}
	} else if (event.type == SDL_MOUSEMOTION) {
		update_mouse_pos(h, pos.x, pos.y);
		p->btn_hover = detect_button_click(p->btn, 0, p->nb_field, pos);
	} else if (ESCAPE_PRESSED(event)) {
		CHESS_LOG(LOG_INFO, "Close profile menu\n");
		unset_flag(&h->flag, FLAG_EDIT_PROFILE);
	}

	for (s32 i = 0; i < p->nb_field; i++) {
		if (p->tf[i]->is_active) {
			handle_text_input(h, &event, p->tf[i]);
		}
	}
}

static void button_event_handling(SDLHandle *h, SDL_Event event, s32 btn_start, s32 btn_nb) {
	iVec2 pos = {0, 0};
	BtnType btn_click = BTN_INVALID;

	SDL_GetMouseState(&pos.x, &pos.y);
	if (is_left_click_down(event)) {
		btn_click = detect_button_click(h->menu.btn, btn_start, btn_nb, pos);
		if (btn_click != BTN_INVALID && h->menu.btn[btn_click].state != BTN_STATE_DISABLED) {
			h->menu.btn[btn_click].state = BTN_STATE_PRESSED;
			h->menu.current_btn_clicked = btn_click;
		}
	} else if (is_left_click_up(event)) {
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
	} else if (btn_start == BTN_RESUME && ESCAPE_PRESSED(event)) {
		menu_close(&h->menu);
	} else if (event.type == SDL_MOUSEMOTION) {
		update_mouse_pos(h, pos.x, pos.y);
		h->menu.btn_hover = detect_button_click(h->menu.btn, btn_start, btn_nb, pos);
	}
}

static s32 call_move_piece_handling(SDLHandle *h, ChessBoard *b) {
	s32 ret = move_piece(h, b->selected_tile, b->last_clicked_tile, b->selected_piece);
	b->possible_moves = 0;
	h->over_piece_select = EMPTY;
	return (ret);
}

static void handle_move_piece_call(SDLHandle *h, ChessBoard *b) {
	s32	ret = FALSE;
	
	if (is_locale_mode(h->flag)) {
		call_move_piece_handling(h, b);
		handle_locale_turn(h);
	} else { /* Network mode */
		/* Build move message to the other player if is not pawn promotion or chess quit */
		if (h->player_info.turn == TRUE) {
			ret = call_move_piece_handling(h, b);
			update_graphic_board(h);
			if (ret != PAWN_PROMOTION) {
				h->player_info.turn = FALSE;
				build_message(h, h->player_info.msg_tosend, MSG_TYPE_MOVE, b->selected_tile, b->last_clicked_tile, b->selected_piece);
				safe_msg_send(h);
			}
		}
	}
}

static void game_click_handling(SDLHandle *h, ChessBoard *b) {
	
	if (b->last_clicked_tile != INVALID_TILE) {
		if (is_selected_possible_move(b->possible_moves, b->last_clicked_tile)) {
			handle_move_piece_call(h, b);
		} else { /* if not possible move */
			b->selected_piece = get_piece_from_tile(b, b->last_clicked_tile);
			/* if peace is not in range reset and return */
			if (!piece_in_range(h, b->selected_piece)) { return (reset_selected_tile(h)); }
			b->selected_tile = b->last_clicked_tile;
			b->possible_moves = get_piece_move(b, (1ULL << b->selected_tile), b->selected_piece, TRUE);
			if (b->possible_moves == 0) { h->over_piece_select = EMPTY ; }
		}
	} else { // if invalid tile clicked
		reset_selected_tile(h);
	}
}

static void game_handle_left_click_down(SDLHandle *h, s32 x, s32 y, s8 player_color) {
	ChessBoard *b = h->board;
	ChessPiece piece_select = EMPTY;

	b->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
	piece_select = get_piece_from_tile(b, b->last_clicked_tile);

	if (!promotion_enable(h->flag) && piece_in_range(h, piece_select)) {
		h->over_piece_select = piece_select;
	}
	game_click_handling(h, b);
}

static void game_handle_left_click_up(SDLHandle *h, s32 x, s32 y, s8 player_color) {
	Bitboard aly_pos = h->over_piece_select >= BLACK_PAWN ? h->board->black : h->board->white;;
	
	if (!promotion_enable(h->flag) && h->over_piece_select != EMPTY) {
		h->board->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
		if (h->board->last_clicked_tile == h->board->selected_tile && h->over_piece_select != EMPTY) {
			h->over_piece_select = EMPTY;
		} else if (h->board->last_clicked_tile == INVALID_TILE \
			|| ((1ULL << h->board->last_clicked_tile) & aly_pos) != 0) {
			reset_selected_tile(h);
			h->board->last_clicked_tile = INVALID_TILE;
		}
		game_click_handling(h, h->board);
	}
}

#include "../include/chess_bot.h"

static void game_event_handling(SDLHandle *h, SDL_Event event, s8 player_color) {
	s32 x = 0, y = 0;

	if (ESCAPE_PRESSED(event)) {
		h->menu.is_open = TRUE;
	}

	if (is_key_pressed(event, SDLK_p)) {
		char *fen = build_FEN_notation(h);
		send_stockfish_fen(fen);
		free(fen);
	}

	if (h->player_info.turn == FALSE) { return ; }
	SDL_GetMouseState(&x, &y);
	if (is_left_click_down(event)) {
		game_handle_left_click_down(h, x, y, player_color);
	} else if (is_left_click_up(event)) {
		game_handle_left_click_up(h, x, y, player_color);
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
		} else if (has_flag(h->flag, FLAG_EDIT_PROFILE)) {
			profile_button_handling(h, event);
		} else {
			if (!h->menu.is_open) {
				game_event_handling(h, event, player_color);
			} else {
				if (h->menu.ip_field->is_active) {
					handle_text_input(h, &event, h->menu.ip_field);
				}
				button_event_handling(h, event, BTN_RESUME, h->menu.menu_btn + 1);
			}
		}
	}
	return (TRUE);
}