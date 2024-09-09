#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

/* @brief Write text in a rect
 * @param h The SDLHandle
 * @param str The string to write
 * @param font The font
 * @param pos The position of the text
 * @param color The color of the text
 */
void write_text_in_rect(SDLHandle *h, TTF_Font *font, SDL_Rect rect, char *str, RectTextPos align, u32 color) {
	// u32 color = RGBA_TO_UINT32(0, 0, 0, 255);
	iVec2 pos = {0, 0};
	iVec2 text_size = {0, 0};

	TTF_SizeText(font, str, &text_size.x, &text_size.y);
	pos.x = rect.x + (rect.w >> 1);
	pos.x -= (text_size.x >> 1);

	if (align == CENTER) {
		pos.y = rect.y + (rect.h >> 1);
		pos.y -= (text_size.y >> 1);
	} else if (align == TOP_CENTER) {
		pos.y = rect.y + (rect.h >> 4) - (text_size.y >> 1);
	} else if (align == BOT_CENTER) {
		pos.y = rect.y + (rect.h >> 1) + (rect.h >> 2);
	} else if (align == TOP) {
		pos.y = rect.y - (rect.h) + (text_size.y >> 1);
	}

	write_text(h, str, font, pos, color);
}

/* @brief Center the text in the left band
 * @param char_pos The position of the text
 * @param text The text to center
 * @param font The font
 * @param tile_size The size of the tile
 * @param band_left_size The size of the left band
 */
void left_band_center_text(iVec2 *char_pos, char *text, TTF_Font *font, s32 tile_size, s32 band_left_size) {
	iVec2 text_size = {0, 0};

	/* Get the text size */
	TTF_SizeText(font, text, &text_size.x, &text_size.y);

	/* Center the number in height */
	char_pos->y += (tile_size >> 1) - (text_size.y >> 1);

	/* Center the number width in the left band */
	char_pos->x -= (band_left_size >> 1);
	char_pos->x -= (text_size.x >> 1);
		
}

/* @brief Center the text in the bottom band
 * @param char_pos The position of the text
 * @param text The text to center
 * @param font The font
 * @param tile_size The size of the tile
 * @param band_bot_size The size of the bottom band
 */
void bot_band_center_text(iVec2 *char_pos, char *text, TTF_Font *font, s32 tile_size, s32 band_bot_size) {
	iVec2 text_size = {0, 0};

	/* Get the text size */
	TTF_SizeText(font, text, &text_size.x, &text_size.y);

	/* Center the number in width */
	char_pos->x += (tile_size >> 1) - (text_size.y >> 1);

	/* Center the number height in the left band */
	char_pos->y += (band_bot_size >> 1);
	char_pos->y -= (text_size.x >> 1);
}

/* Draw letter and number for raw/column */
void draw_letter_number(SDLHandle *handle, s8 player_color) {
	iVec2		pos = {0, 0}, char_pos = {0, 0};
	s32			tile_size = handle->tile_size.x;
	s32			column_raw = 7;
	char		letter = 'H', number = '1';
	char		text[2] = {0};

	if (player_color == IS_BLACK) {
		letter = 'A';
		number = '8';
	}

	while (column_raw >= 0) {
		/* Get lette tile_pos */
		pos = (iVec2){column_raw, 8};
		text[0] = letter;

		/* Get the pixel position en center it */
		// TILE_POSITION_TO_PIXEL(pos, char_pos.x, char_pos.y, tile_size, handle->band_size);
		tile_to_pixel_pos(&char_pos, pos, tile_size, handle->band_size);
		bot_band_center_text(&char_pos, text, handle->tile_font, tile_size, handle->band_size.bot);

		/* Draw the letter */
		write_text(handle, (char[]){letter, '\0'}, handle->tile_font, char_pos, RGBA_TO_UINT32(255, 165, 0, 255)); // orange color
		
		/* Get number tile_pos */
		pos = (iVec2){0, column_raw};
		text[0] = number;
		
		/* Get the pixel position en center it */
		// TILE_POSITION_TO_PIXEL(pos, char_pos.x, char_pos.y, tile_size, handle->band_size);
		tile_to_pixel_pos(&char_pos, pos, tile_size, handle->band_size);
		left_band_center_text(&char_pos, text, handle->tile_font, tile_size, handle->band_size.left);

		/* Draw the number */
		write_text(handle, (char[]){number, '\0'}, handle->tile_font, char_pos, RGBA_TO_UINT32(255, 165, 0, 255)); // orange color

		/* Increment or decrement letter and number */
		number = player_color == IS_BLACK ? number - 1 : number + 1;
		letter = player_color == IS_BLACK ? letter + 1 : letter - 1;
		column_raw--;
	}
}

/* Draw the given text in the center of the board (in a rect) */
void center_text_draw(SDLHandle *h, CenterText *ct) {
    iVec2 text_size = {0, 0};
    iVec2 text_pos = {0, 0};

    SDL_SetRenderDrawColor(h->renderer, MENU_BG_COLOR);
    SDL_RenderFillRect(h->renderer, &ct->rect);

    // Calculer la taille et la position de la première chaîne
    TTF_SizeText(ct->font, ct->str, &text_size.x, &text_size.y);
    text_pos.x = ct->rect.x + (ct->rect.w >> 1) - (text_size.x >> 1);
    text_pos.y = ct->rect.y + (ct->rect.h >> 1) - (text_size.y >> 1);

    // Ajuster la position si str2 est présente
    if (ct->str2) {
        text_pos.y -= text_size.y; // Remonter la première chaîne de quelques pixels
    }

    write_text(h, ct->str, ct->font, text_pos, U32_BLACK_COLOR);

    // Afficher la seconde chaîne si elle est présente
    if (ct->str2) {
        iVec2 text_size2 = {0, 0};
        iVec2 text_pos2 = {0, 0};

        TTF_SizeText(ct->font, ct->str2, &text_size2.x, &text_size2.y);
        text_pos2.x = ct->rect.x + (ct->rect.w >> 1) - (text_size2.x >> 1);
        text_pos2.y = text_pos.y + (text_size.y << 1); // Positionner la seconde chaîne en dessous de la première

        write_text(h, ct->str2, ct->font, text_pos2, U32_BLACK_COLOR);
    }

	draw_multiple_button(h, h->menu.btn_text_font, h->menu.btn, h->menu.btn_hover, BTN_CENTER1, ct->curent_btn_enable);
}

void cancel_search_func(SDLHandle *h) {
	(void)h;
	center_text_string_set(h, NULL, NULL);
	if (has_flag(h->flag, FLAG_NETWORK)) {
		unset_flag(&h->flag, FLAG_NETWORK);
		destroy_network_info(h);
	}
	// unset_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	CHESS_LOG(LOG_INFO, "BtnCenter1: Cancel search/reconnect\n");
}

typedef enum {
	CENTER_POS,
	LEFT_POS,
	RIGHT_POS,
} CenterBtnPos;



void  get_center_btn_pos(CenterText *ct, iVec2 *btn_start, iVec2 *btn_size, s8 pos) {
	btn_size->x = ct->rect.w >> 2;
	btn_size->y = ct->rect.h >> 2;

	btn_start->y = ct->rect.y + ct->rect.h + (ct->rect.h >> 4);

	if (pos == CENTER_POS) {
		btn_start->x = ct->rect.x + (ct->rect.w >> 1) - (btn_size->x >> 1);
	} else if (pos == LEFT_POS) {
		btn_start->x = ct->rect.x + (ct->rect.w >> 2) - (btn_size->x >> 1);
	} else if (pos == RIGHT_POS) {
		btn_start->x = ct->rect.x + (ct->rect.w >> 1) + (ct->rect.w >> 2) - (btn_size->x >> 1);
	}
}

CenterText *center_text_init(SDLHandle *h) {
    CenterText *ct = ft_calloc(1, sizeof(CenterText));

    if (!ct) {
        CHESS_LOG(LOG_ERROR, "Failed to allocate memory\n");
        return (NULL);
    }

    ct->str = NULL;
    ct->str2 = NULL;

    if (ct->rect.x == 0) {
        ct->rect.w = h->tile_size.x * 4;
        ct->rect.h = h->tile_size.x * 2;
        ct->rect.x = h->band_size.left + (h->tile_size.x * 2);
        ct->rect.y = h->band_size.top + (h->tile_size.x * 3);
    }

    ct->font = load_font(FONT_PATH, (ct->rect.h >> 2) - (ct->rect.h >> 4));
    if (!ct->font) {
        CHESS_LOG(LOG_ERROR, "Failed to load font\n");
        free(ct);
        return (NULL);
    }

	center_text_function_set(h, ct, (BtnCenterText){"Cancel", cancel_search_func}, (BtnCenterText){NULL, NULL});
    return (ct);
}

void center_text_function_set(SDLHandle *h, CenterText *ct, BtnCenterText btn1, BtnCenterText btn2) {
	iVec2 btn_start = {0, 0};
	iVec2 btn_size = {0, 0};

	if (!btn2.func) {
		ct->curent_btn_enable = 1;
		get_center_btn_pos(ct, &btn_start, &btn_size, CENTER_POS);
		set_btn_info(h, BTN_CENTER1, btn_start, btn_size, btn1.str, btn1.func);
		return ;
	}

	ct->curent_btn_enable = 2;

	get_center_btn_pos(ct, &btn_start, &btn_size, LEFT_POS);
	set_btn_info(h, BTN_CENTER1, btn_start, btn_size, btn1.str, btn1.func);

	get_center_btn_pos(ct, &btn_start, &btn_size, RIGHT_POS);
	set_btn_info(h, BTN_CENTER2, btn_start, btn_size, btn2.str, btn2.func);
}

void center_text_string_set(SDLHandle *h, char *str, char *str2) {
    if (h->center_text->str) {
        free(h->center_text->str);
        h->center_text->str = NULL;
    }
    if (h->center_text->str2) {
        free(h->center_text->str2);
        h->center_text->str2 = NULL;
    }
    if (str) {
        h->center_text->str = ft_strdup(str);
    }
    if (str2) {
        h->center_text->str2 = ft_strdup(str2);
    }
}

void center_text_destroy(CenterText *ct) {
	if (!ct) {
		return ;
	}
	if (ct->str) {
		free(ct->str);
		ct->str = NULL;
	}
	if (ct->str2) {
		free(ct->str2);
		ct->str2 = NULL;
	}
	if (ct->font) {
		unload_font(ct->font);
		ct->font = NULL;
	}
	free(ct);
}