#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"

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
