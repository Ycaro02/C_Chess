#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


/**
 * @brief Destroy the menu
 * @param h The SDLHandle
*/
void destroy_menu(SDLHandle *h) {
	for (s32 i = 0; i < h->menu.nb_btn; i++) {
		free(h->menu.btn[i].text);
	}
	free(h->menu.btn);
	unload_font(h->menu.btn_text_font);
}

/**
 * @brief Detect if a button is clicked
 * @param btn The button
 * @param nb_btn The number of button
 * @param mouse_pos The mouse position
 * @return The index of the button clicked
*/
s32 detect_button_click(Button *btn, s32 nb_btn, iVec2 mouse_pos) {
	for (s32 i = 0; i < nb_btn; i++) {
		if (mouse_pos.x >= btn[i].start.x && mouse_pos.x <= btn[i].end.x &&
			mouse_pos.y >= btn[i].start.y && mouse_pos.y <= btn[i].end.y) {
			CHESS_LOG(LOG_INFO, "Button %d clicked\n", i);
			return (i);
		}
	}
	CHESS_LOG(LOG_INFO, "No button clicked\n");	
	return (BTN_INVALID);
}

/**
 * @brief Wrapper to handle the button click
 * @param h The SDLHandle
 * @param mouse_pos The mouse position
*/
void search_game(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Search game\n");
}

void reconnect_game(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Reconnect to game\n");
	if (!has_flag(h->flag, FLAG_NETWORK)) {
		set_flag(&h->flag, FLAG_NETWORK);
		set_flag(&h->flag, FLAG_RECONNECT);
		h->menu.is_open = FALSE;
		h->player_info.dest_ip = ft_strdup("127.0.0.1");
		network_setup(h, h->flag, &h->player_info, h->player_info.dest_ip);
		network_chess_routine(h);
	}
	chess_destroy(h);
}

void quit_game(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Quit game\n");
	chess_destroy(h);
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
void button_center_text(Button *btn, TTF_Font *font) {
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
void init_button(SDLHandle *h, s32 nb_btn) {
	s32 btn_pad = h->menu.height / 10;
	s32 btn_width = h->menu.width >> 1;
	s32 btn_height = (h->menu.height - (btn_pad * (nb_btn + 1))) / nb_btn;

	h->menu.btn_text_font = TTF_OpenFont(FONT_PATH, btn_height >> 1);

	for (s32 i = 0; i < nb_btn; i++) {
		h->menu.btn[i].start.x = h->menu.start.x + (h->menu.width >> 2);
		h->menu.btn[i].start.y = h->menu.start.y + ((i * btn_pad) + btn_pad) + (i * btn_height);
		h->menu.btn[i].width = btn_width;
		h->menu.btn[i].height = btn_height;
		h->menu.btn[i].end.x = h->menu.btn[i].start.x + btn_width;
		h->menu.btn[i].end.y = h->menu.btn[i].start.y + btn_height;
		set_btn_text_func(h, i, i);
		h->menu.btn[i].state = BTN_RELEASED;
		button_center_text(&h->menu.btn[i], h->menu.btn_text_font);
	}
}

/**
 * @brief Set the menu size
 * @param h The SDLHandle
 * @param nb_btn The number of button
*/
void init_menu(SDLHandle *h, s32 nb_btn) {
	h->menu.start.x = h->band_size.left + (h->tile_size.x << 1);
	h->menu.start.y = h->band_size.top + (h->tile_size.x << 1);
	h->menu.width = h->tile_size.x << 2;
	h->menu.height = h->tile_size.x << 2;
	h->menu.end.x = h->menu.start.x + h->menu.width;
	h->menu.end.y = h->menu.start.y + h->menu.height;

	/* Set main button info */
	h->menu.nb_btn = nb_btn;
	h->menu.btn = ft_calloc(sizeof(Button), nb_btn);
	if (h->menu.btn == NULL) {
		CHESS_LOG(LOG_ERROR, "Failed to allocate memory for buttons\n");
		return ;
	}
	init_button(h, nb_btn);
}

/**
 * @brief Draw a button
 * @param h The SDLHandle
 * @param btn The button to draw
 * @param state The state of the button
*/
void draw_button(SDLHandle *h, Button btn, s8 state) {
	SDL_Rect		rect = {0,0,0,0};

	rect.x = btn.start.x;
	rect.y = btn.start.y;
	rect.w = btn.width;
	rect.h = btn.height;

	if (state == BTN_PRESSED) {
		SDL_SetRenderDrawColor(h->renderer, 100, 10, 10, 255);
	} else {
		SDL_SetRenderDrawColor(h->renderer, 10, 10, 10, 255);
	}
	SDL_RenderFillRect(h->renderer, &rect);

	write_text(h, btn.text, h->menu.btn_text_font, btn.text_pos, RGBA_TO_UINT32(255, 255, 255, 255));
	
}


/**
 * @brief Draw the menu
 * @param h The SDLHandle
*/
void draw_menu(SDLHandle *h) {
	SDL_Rect		rect = {0,0,0,0};
	iVec2			start = h->menu.start;

	rect.x = start.x;
	rect.y = start.y;
	rect.w = h->menu.width;
	rect.h = h->menu.height;

	SDL_SetRenderDrawColor(h->renderer, 70,70,70,220);
	SDL_RenderFillRect(h->renderer, &rect);

	for (s32 i = 0; i < h->menu.nb_btn; i++) {
		draw_button(h, h->menu.btn[i], h->menu.btn[i].state);
	}
}