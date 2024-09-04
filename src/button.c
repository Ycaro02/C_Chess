#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

/**
 * @brief Detect if a button is clicked
 * @param btn The button
 * @param btn_start The start index of the button
 * @param nb_btn The number of button
 * @param mouse_pos The mouse position
 * @return The index of the button clicked
*/
s32 detect_button_click(Button *btn, s32 btn_start, s32 nb_btn, iVec2 mouse_pos) {
	s32 idx = 0;
	
	for (s32 i = 0; i < nb_btn; i++) {
		idx = btn_start + i;
		if (mouse_pos.x >= btn[idx].start.x && mouse_pos.x <= btn[idx].end.x &&
			mouse_pos.y >= btn[idx].start.y && mouse_pos.y <= btn[idx].end.y) {
			// CHESS_LOG(LOG_INFO, "Button %d clicked\n", idx);
			return (idx);
		}
	}
	return (BTN_INVALID);
}

s8 wait_player_handling(SDLHandle *h) {
	set_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	wait_for_player(h);
	unset_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	if (!has_flag(h->flag, FLAG_NETWORK)) {
		return (FALSE);
	}
	return (TRUE);
}

/**
 * @brief Wrapper to handle the button click
 * @param h The SDLHandle
 * @param mouse_pos The mouse position
*/
void search_game(SDLHandle *h) {
	(void)h;
	
	u32 client_flag = FLAG_LISTEN;
	struct timeval	timeout = {0, 10000}; /* 10000 microseconds = 0.01 seconds */

	menu_close(&h->menu);
	/* Reset board */
	init_board(h->board);
	set_info_str(h, "Searching game on :", h->player_info.dest_ip);
	update_graphic_board(h);

	if (!has_flag(h->flag, FLAG_NETWORK)) {
		set_flag(&h->flag, FLAG_NETWORK);
		
		/* Init network and player state */
		h->player_info.nt_info = init_network(h->player_info.dest_ip, timeout);

		/* Wait for player */
		if (!wait_player_handling(h)) {
			return ;
		}
		CHESS_LOG(LOG_INFO, "After wait player: %s\n", clientstate_to_str(h->player_info.nt_info->client_state));
		if (h->player_info.nt_info->client_state == CLIENT_STATE_WAIT_COLOR) {
			client_flag = FLAG_JOIN;
		} else if (h->player_info.nt_info->client_state == CLIENT_STATE_RECONNECT) {
			client_flag = FLAG_RECONNECT;
		}
		set_flag(&h->flag, client_flag);

		handle_network_client_state(h, h->flag, &h->player_info);
		set_info_str(h, NULL, NULL);
		network_chess_routine(h);
		chess_destroy(h);
	}
}

void reconnect_game(SDLHandle *h) {
	(void)h;
	struct timeval	timeout = {0, 10000}; /* 10000 microseconds = 0.01 seconds */

	menu_close(&h->menu);
	CHESS_LOG(LOG_INFO, "Reconnect to game\n");
	/* Reset board */
	init_board(h->board);
	set_info_str(h, "Reconnect game on:", h->player_info.dest_ip);
	update_graphic_board(h);

	if (!has_flag(h->flag, FLAG_NETWORK)) {
		set_flag(&h->flag, FLAG_NETWORK);
		set_flag(&h->flag, FLAG_RECONNECT);
		h->player_info.nt_info = init_network(h->player_info.dest_ip, timeout);

		/* Wait for player */
		if (!wait_player_handling(h)) {
			return ;
		}
		handle_network_client_state(h, h->flag, &h->player_info);
		set_info_str(h, NULL, NULL);
		network_chess_routine(h);
		chess_destroy(h);
	}
}

void quit_game(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Quit game\n");
	chess_destroy(h);
}

void change_ip_click(SDLHandle *h) {
	(void)h;
	h->menu.ip_field.is_active = TRUE;
	CHESS_LOG(LOG_INFO, "Change ip\n");
}

/**
 * @brief Set the button text and function
 * @param h The SDLHandle
 * @param idx The index of the button
 * @param type The type of the button
*/
void set_btn_text_func(SDLHandle *h, s32 idx, BtnType type) {
	
	if (type == BTN_RESUME) {
		h->menu.btn[idx].text = ft_strdup("Resume");
		h->menu.btn[idx].func = NULL;
	} else if (type == BTN_SEARCH) {
		h->menu.btn[idx].text = ft_strdup("Search game");
		h->menu.btn[idx].func = search_game;
	} else if (type == BTN_RECONNECT) {
		h->menu.btn[idx].text = ft_strdup("Reconnect");
		h->menu.btn[idx].func = reconnect_game;
	} else if (type == BTN_QUIT) {
		h->menu.btn[idx].text = ft_strdup("Quit");
		h->menu.btn[idx].func = quit_game;
	}
}

/**
 * @brief Center the text in the button
 * @param btn The button
 * @param font The font
*/
void center_btn_text(Button *btn, TTF_Font *font) {
	iVec2		text_size = {0, 0};
	iVec2		text_pos = {0, 0};

	TTF_SizeText(font, btn->text, &text_size.x, &text_size.y);

	text_pos.x = btn->start.x + (btn->width >> 1) - (text_size.x >> 1);
	text_pos.y = btn->start.y + (btn->height >> 1) - (text_size.y >> 1);

	btn->text_pos = text_pos;
}

/**
 * @brief Initialize the button
 * @param h The SDLHandle
 * @param nb_btn The number of button
*/
void init_button(SDLHandle *h, ChessMenu *menu, s32 nb_btn) {
	s32 btn_pad = menu->height / 10;
	s32 btn_width = menu->width >> 1;
	s32 btn_height = (menu->height - (btn_pad * (nb_btn + 1))) / nb_btn;

	menu->btn_text_font = TTF_OpenFont(FONT_PATH, btn_height >> 1);

	for (s32 i = 0; i < nb_btn; i++) {
		menu->btn[i].start.x = menu->start.x + (menu->width >> 2);
		menu->btn[i].start.y = menu->start.y + ((i * btn_pad) + btn_pad) + (i * btn_height);
		menu->btn[i].width = btn_width;
		menu->btn[i].height = btn_height;
		menu->btn[i].end.x = menu->btn[i].start.x + btn_width;
		menu->btn[i].end.y = menu->btn[i].start.y + btn_height;
		set_btn_text_func(h, i, i);
		menu->btn[i].state = BTN_STATE_RELEASED;
		center_btn_text(&menu->btn[i], menu->btn_text_font);
	}
}

/**
 * @brief Draw a button
 * @param h The SDLHandle
 * @param btn The button to draw
 * @param state The state of the button
*/
void draw_button(SDLHandle *h, Button btn, SDL_Color c) {
	SDL_Rect		rect = {0,0,0,0};

	rect.x = btn.start.x;
	rect.y = btn.start.y;
	rect.w = btn.width;
	rect.h = btn.height;

	SDL_SetRenderDrawColor(h->renderer, c.r, c.g, c.b, c.a);
	SDL_RenderFillRect(h->renderer, &rect);
	write_text(h, btn.text, h->menu.btn_text_font, btn.text_pos, RGBA_TO_UINT32(255, 255, 255, 255));
}


void update_btn_disabled(SDLHandle *h, Button *btn) {
	u32 flag = h->flag;

	if (has_flag(flag, FLAG_NETWORK)) {
		btn[BTN_SEARCH].state = BTN_STATE_DISABLED;
		btn[BTN_RECONNECT].state = BTN_STATE_DISABLED;
	} 
}

/**
 * @brief Set the button info
 * @param h The SDLHandle
 * @param btn_idx The index of the button
 * @param start The start position
 * @param size The size of the button
 * @param text The text of the button
 * @param func The function of the button
*/
void set_btn_info(SDLHandle *h, s32 btn_idx, iVec2 start, iVec2 size, char *text, ButtonFunc func) {
	h->menu.btn[btn_idx].start = start;
	h->menu.btn[btn_idx].width = size.x;
	h->menu.btn[btn_idx].height = size.y;
	h->menu.btn[btn_idx].end = (iVec2){start.x + size.x, start.y + size.y};
	if (h->menu.btn[btn_idx].text) {
		free(h->menu.btn[btn_idx].text);
		h->menu.btn[btn_idx].text = NULL;
	}
	h->menu.btn[btn_idx].text = ft_strdup(text);
	h->menu.btn[btn_idx].func = func;
	h->menu.btn[btn_idx].state = BTN_STATE_RELEASED;
	center_btn_text(&h->menu.btn[btn_idx], h->menu.btn_text_font);
}

/**
 * @brief Draw multiple button
 * @param h The SDLHandle
 * @param btn_start The start index of the button
 * @param nb_btn The number of button
*/
void draw_multiple_button(SDLHandle *h, s32 btn_start, s32 nb_btn) {
	SDL_Color btn_color;
	s32 idx = 0;

	for (s32 i = 0; i < nb_btn; i++) {
		idx = btn_start + i;
		// printf("In draw multiple button: idx: %d Hover: %d\n", idx, h->menu.btn_hover);
		if (idx == h->menu.btn_hover) {
			btn_color = BTN_HOVER_COLOR;
		} else {
			btn_color = BTN_BASIC_COLOR;
		}
		if (h->menu.btn[idx].state == BTN_STATE_DISABLED) {
			btn_color = (SDL_Color){CLEAR_COLOR};
		}
		draw_button(h, h->menu.btn[idx], btn_color);
	}

}