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

// s32 detect_menu_btn_click(SDLHandle *h, iVec2 mouse_pos) {
// 	s32 btn_idx = detect_button_click(h->menu.btn, h->menu.nb_btn, mouse_pos);
	
// 	if (btn_idx == BTN_INVALID) {
// 		btn_idx = detect_button_click(&h->menu.server_ip_btn, 1, mouse_pos);
// 		if (btn_idx != BTN_INVALID) {
// 			btn_idx = BTN_SERVER_IP;
// 		}
// 	} 
// 	return (btn_idx);
// }

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
		menu->btn[i].state = BTN_RELEASED;
		button_center_text(&menu->btn[i], menu->btn_text_font);
	}
}

#define SERVER_INFO_STR "Server IP: "

/**
 * @brief Set the menu size
 * @param h The SDLHandle
 * @param nb_btn The number of button
*/
void init_menu(SDLHandle *h, s32 total_btn) {
	
	/* The last button is the server ip button */
	s32 menu_btn = total_btn - 1;

	/* Set menu rect data */
	h->menu.start.x = h->band_size.left + (h->tile_size.x << 1);
	h->menu.start.y = h->band_size.top + (h->tile_size.x << 1);
	h->menu.width = h->tile_size.x << 2;
	h->menu.height = h->tile_size.x << 2;
	h->menu.end.x = h->menu.start.x + h->menu.width;
	h->menu.end.y = h->menu.start.y + h->menu.height;

	/* Set server info rect */
	h->menu.server_info.x = h->menu.start.x;
	h->menu.server_info.y = h->menu.start.y - ((h->tile_size.x >> 1) + h->tile_size.x);
	h->menu.server_info.w = h->menu.width;
	h->menu.server_info.h = h->tile_size.x;

	/* Set server info string position */
	s32 server_info_pad = (h->tile_size.x >> 3);
	h->menu.server_info_str_pos.x = h->menu.server_info.x + (h->tile_size.x >> 2);
	h->menu.server_info_str_pos.y = h->menu.server_info.y + server_info_pad;


	/* Set main button info */
	h->menu.nb_btn = total_btn;
	h->menu.btn = ft_calloc(sizeof(Button), total_btn);
	if (h->menu.btn == NULL) {
		CHESS_LOG(LOG_ERROR, "Failed to allocate memory for buttons\n");
		return ;
	}
	init_button(h, &h->menu, menu_btn);


	/* Set server ip button info */
	Button *server_ip_btn = &h->menu.btn[BTN_SERVER_IP];

	server_ip_btn->width = h->tile_size.x << 1;
	server_ip_btn->height = (h->tile_size.x >> 1) - server_info_pad;
	server_ip_btn->start.x = h->menu.server_info.x + (h->menu.server_info.w >> 1) - (server_ip_btn->width >> 1);
	server_ip_btn->start.y = h->menu.server_info_str_pos.y + (h->tile_size.x >> 2) + server_info_pad;
	server_ip_btn->end.x = server_ip_btn->start.x + server_ip_btn->width;
	server_ip_btn->end.y = server_ip_btn->start.y + server_ip_btn->height;
	
	server_ip_btn->text = ft_strdup("Change");
	button_center_text(server_ip_btn, h->menu.btn_text_font);
	server_ip_btn->state = BTN_RELEASED;
	server_ip_btn->func = change_ip_click;


	/* Set server ip position */
	iVec2 text_size = {0, 0};
	TTF_SizeText(h->menu.btn_text_font, SERVER_INFO_STR, &text_size.x, &text_size.y);

	SDL_Rect rect = {0,0,0,0};
	rect.x = h->menu.server_info_str_pos.x + text_size.x;
	rect.y = h->menu.server_info_str_pos.y;
	rect.w = (h->menu.width >> 1) + (h->menu.width >> 4);
	rect.h = text_size.y;

	h->menu.ip_field = init_text_field(rect, TEXT_INPUT_SIZE, h->menu.btn_text_font, "127.0.0.1");
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

	/* Draw the menu rect */
	SDL_SetRenderDrawColor(h->renderer, 70,70,70,220);
	SDL_RenderFillRect(h->renderer, &rect);

	/* Draw the server info rect */
	SDL_SetRenderDrawColor(h->renderer, 70,70,70,255);
	SDL_RenderFillRect(h->renderer, &h->menu.server_info);

	/* Draw the server info string */
	write_text(h, SERVER_INFO_STR, h->menu.btn_text_font, h->menu.server_info_str_pos, RGBA_TO_UINT32(255, 255, 255, 255));
	
	SDL_Color bg_color = {70, 70, 70, 255};
	/* Draw the server ip text input */
	if (h->menu.ip_field.is_active) {
		bg_color = (SDL_Color){255, 255, 255, 255};
	}
	render_text_field(h->renderer, &h->menu.ip_field, (SDL_Color){255, 0, 0, 255}, bg_color);

	/* Draw the menu button */
	for (s32 i = 0; i < h->menu.nb_btn; i++) {
		draw_button(h, h->menu.btn[i], h->menu.btn[i].state);
	}
}