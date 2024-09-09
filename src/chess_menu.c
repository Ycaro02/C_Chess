#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

void menu_close(ChessMenu *menu) {
	menu->is_open = FALSE;
	menu->btn_hover = BTN_INVALID;
	menu->ip_field->is_active = FALSE;
}

/**
 * @brief Destroy the menu
 * @param h The SDLHandle
*/
void destroy_menu(SDLHandle *h) {
	destroy_text_field(h->menu.ip_field);
	for (s32 i = 0; i < h->menu.total_btn; i++) {
		if (h->menu.btn[i].text) {
			free(h->menu.btn[i].text);
		}
	}
	free(h->menu.btn);
	unload_font(h->menu.btn_text_font);
}

/**
 * @brief Set the menu size
 * @param h The SDLHandle
 * @param nb_btn The number of button
*/
s8 init_menu(SDLHandle *h, s32 total_btn) {
	
	/* The last button is the server ip button */
	h->menu.menu_btn = BTN_QUIT + 1;

	h->menu.is_open = FALSE;
	h->menu.btn_hover = BTN_INVALID;

	/* Set menu rect data */
	h->menu.start.x = h->band_size.left + (h->tile_size.x << 1);
	h->menu.start.y = h->band_size.top + (h->tile_size.x << 1) - (h->tile_size.x >> 1);
	h->menu.width = h->tile_size.x << 2;
	h->menu.height = (h->tile_size.x << 2) + h->tile_size.x;
	h->menu.end.x = h->menu.start.x + h->menu.width;
	h->menu.end.y = h->menu.start.y + h->menu.height;

	/* Set server info rect */
	h->menu.server_info.x = h->menu.start.x;
	h->menu.server_info.y = h->menu.start.y - ((h->tile_size.x >> 2)) - h->tile_size.x;
	h->menu.server_info.w = h->menu.width;
	h->menu.server_info.h = h->tile_size.x;

	/* Set server info string position */
	s32 server_info_pad = (h->tile_size.x >> 3);
	h->menu.server_info_str_pos.x = h->menu.server_info.x + (h->tile_size.x >> 2);
	h->menu.server_info_str_pos.y = h->menu.server_info.y + server_info_pad;


	/* Set main button info */
	h->menu.total_btn = total_btn;
	h->menu.btn = ft_calloc(sizeof(Button), total_btn);
	if (h->menu.btn == NULL) {
		CHESS_LOG(LOG_ERROR, "Failed to allocate memory for buttons\n");
		return (FALSE);
	}
	init_button(h, &h->menu, h->menu.menu_btn);


	/* Set server ip button info */
	Button *server_ip_btn = &h->menu.btn[BTN_SERVER_IP];

	server_ip_btn->width = h->tile_size.x << 1;
	server_ip_btn->height = (h->tile_size.x >> 1) - server_info_pad;
	server_ip_btn->start.x = h->menu.server_info.x + (h->menu.server_info.w >> 1) - (server_ip_btn->width >> 1);
	server_ip_btn->start.y = h->menu.server_info_str_pos.y + (h->tile_size.x >> 2) + server_info_pad;
	server_ip_btn->end.x = server_ip_btn->start.x + server_ip_btn->width;
	server_ip_btn->end.y = server_ip_btn->start.y + server_ip_btn->height;
	
	server_ip_btn->text = ft_strdup("Edit");
	center_btn_text(server_ip_btn, h->menu.btn_text_font);
	server_ip_btn->state = BTN_STATE_RELEASED;
	server_ip_btn->func = edit_ip_click;


	/* Set server ip position */
	iVec2 text_size = {0, 0};
	TTF_SizeText(h->menu.btn_text_font, SERVER_INFO_STR, &text_size.x, &text_size.y);

	SDL_Rect rect = {0,0,0,0};
	rect.x = h->menu.server_info_str_pos.x + text_size.x;
	rect.y = h->menu.server_info_str_pos.y;
	rect.w = (h->menu.width >> 1) + (h->menu.width >> 4);
	rect.h = text_size.y;

	h->menu.ip_field = init_text_field(rect, IP_INPUT_SIZE, h->menu.btn_text_font, "127.0.0.1");
	return (TRUE);
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
	SDL_SetRenderDrawColor(h->renderer, MENU_BG_COLOR);
	SDL_RenderFillRect(h->renderer, &rect);

	/* Draw the server info rect */
	SDL_SetRenderDrawColor(h->renderer, CLEAR_COLOR);
	SDL_RenderFillRect(h->renderer, &h->menu.server_info);

	/* Draw the server info string */
	write_text(h, SERVER_INFO_STR, h->menu.btn_text_font, h->menu.server_info_str_pos, RGBA_TO_UINT32(255, 255, 255, 255));
	
	SDL_Color bg_color = {CLEAR_COLOR};
	/* Draw the server ip text input */
	if (h->menu.ip_field->is_active) {
		bg_color = (SDL_Color){WHITE_COLOR};
	}
	render_text_field(h->renderer, h->menu.ip_field, (SDL_Color){255, 0, 0, 255}, bg_color);

	/* Update the button state */
	update_btn_disabled(h, h->menu.btn);

	/* Draw the menu button +1 for the server_ip btn */
	draw_multiple_button(h, BTN_RESUME, h->menu.menu_btn + 1);
}

