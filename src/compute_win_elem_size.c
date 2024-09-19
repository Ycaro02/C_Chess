#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

/**
 * @brief Get the screen size
 * @param width The width of the screen (out)
 * @param height The height of the screen (out)
*/
static void get_screen_size(int *width, int *height) {
    SDL_DisplayMode mode;
    
	if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {
        SDL_Log("SDL_GetCurrentDisplayMode failed: %s", SDL_GetError());
        *width = 0;
        *height = 0;
    } else {
        *width = mode.w;
        *height = mode.h;
    }
}


/**
 * @brief Dynamic compute the window size
 * @param h The SDLHandle pointer
 * @note The window size is computed by removing 1/4 of the screen size on each dimension
 * @note The tile size is computed by dividing the minimum between the width and height by 8
*/
void pc_compute_win_size(SDLHandle *h) {
	s32 size_w=0, size_h=0, width = 0, height = 0;
	s32 minus, tile_size, band_w, band_h;

	get_screen_size(&size_w, &size_h);
	
	/* Adjust width and height by removing 1/4 of each dimension, for information (display coilumn and row) */
    width = size_w - (size_w >> 2);
    height = size_h - (size_h >> 2);

    /* Calculate tile size for the chessboard */
	minus = width < height ? width : height;
    
	/* Chess tile size == minus / 8 */
	tile_size = minus >> 3;

	h->tile_size.x = tile_size;
	h->tile_size.y = tile_size;

	/* Calculate band size */
	band_w = size_w >> 3;
	band_h = size_h >> 4;

	/* Calculate the band size bot and left for raw,column number display */
	h->band_size.bot = band_h;
	h->band_size.left = band_w >> 2;

	/* Detect minus between left and bot band size */
	minus = h->band_size.left < h->band_size.bot ? h->band_size.left : h->band_size.bot;

	/* Set the band size left and band to be the same size */
	h->band_size.left = minus;
	h->band_size.bot = minus;

	/* Set the band size right and top */
	h->band_size.right = band_w - h->band_size.left;
	h->band_size.top = 0;

	/* Set the band size height for the window */
	band_h = h->band_size.bot;

	/* Calculate the window size (tile_size * 8) + band */
	width = (tile_size << 3) + band_w;
	height = (tile_size << 3) + band_h;


	CHESS_LOG(LOG_INFO, "Width: %d, Height: %d, tile_size %d\n", width, height, tile_size);
	CHESS_LOG(LOG_INFO, "Band height: %d, Band width: %d\n", band_h, band_w);
	CHESS_LOG(LOG_INFO, "Top: %d, Bot: %d, Left: %d, Right: %d\n", h->band_size.top, h->band_size.bot, h->band_size.left, h->band_size.right);

	h->window_size.x = width;
	h->window_size.y = height;
}

/* In this we want to take the all width of the screen */
void android_compute_win_size(SDLHandle *h) {
	s32 size_w=0, size_h=0, width = 0, height = 0;
	s32 tile_size, band_w, band_h;

	get_screen_size(&size_w, &size_h);

	// tile_size are width / 8
	tile_size = size_w >> 3;
	h->tile_size.x = tile_size;
	h->tile_size.y = tile_size;

	band_w = 0;
	band_h = size_h - size_w;

	// get the band size
	h->band_size.bot = band_h >> 1;
	h->band_size.top = band_h >> 1;
	h->band_size.left = 0;
	h->band_size.right = 0;
	
	// take all the screen size
	width = size_w;
	height = size_h;
	CHESS_LOG(LOG_INFO, "Width: %d, Height: %d, tile_size %d\n", width, height, tile_size);
	CHESS_LOG(LOG_INFO, "Band height: %d, Band width: %d\n", band_h, band_w);
	CHESS_LOG(LOG_INFO, "Top: %d, Bot: %d, Left: %d, Right: %d\n", h->band_size.top, h->band_size.bot, h->band_size.left, h->band_size.right);
	h->window_size.x = width;
	h->window_size.y = height;
}


/**
 * @brief Build the timer rectangle
 * @param h The SDLHandle pointer
 * @param is_bot_band A flag to indicate if the timer is in the bot band
 * @return SDL_Rect The timer rectangle
*/
SDL_Rect pc_build_timer_rect(SDLHandle *h, s8 is_bot_band) {
	SDL_Rect timer_rect = {0}; 

	/* Window end width - size band right - (size_band_right / 16)*/
	s32 timer_padding = h->band_size.right >> 4;
	s32 timer_rect_start = (h->window_size.x - h->band_size.right) + timer_padding;
	s32 timer_rect_end = h->window_size.x - timer_padding;
	s32 timer_height = h->tile_size.x;

	timer_rect.x = timer_rect_start;

	if (is_bot_band) {
		/* Start to the bot of window - band_size_bot - timer_height */
		timer_rect.y = h->window_size.y - (h->band_size.bot) - (timer_height);
	} else {
		timer_rect.y = h->band_size.top;
	}
	

	timer_rect.w = timer_rect_end - timer_rect_start;
	timer_rect.h = timer_height;

	CHESS_LOG(LOG_INFO, "Timer rect start: %d, end: %d, height: %d, width: %d\n", timer_rect_start, timer_rect_end, timer_height, timer_rect.w);
	return (timer_rect);
}

SDL_Rect android_build_timer_rect(SDLHandle *h, s8 is_bot_band) {
	SDL_Rect timer_rect = {0}; 

	s32 height_pad = (h->band_size.bot >> 2) + (h->band_size.bot >> 3);
	s32 width = (h->window_size.x >> 1) - (h->window_size.x >> 2);
	s32 start_x = 0;
	s32 height = h->tile_size.x;


	if (is_bot_band) {
		start_x = h->window_size.x - width - (h->window_size.x >> 4);
		/* Start to the bot of window - band_size_bot - height_pad */
		timer_rect.y = h->window_size.y - h->band_size.bot + height_pad - (height);
	} else {
		start_x = (h->window_size.x >> 4);
		// timer_rect.y = (height << 1);
		timer_rect.y = 0 + h->band_size.top - height_pad;
	}
	
	timer_rect.x = start_x;
	

	timer_rect.w = width;
	timer_rect.h = height;

	CHESS_LOG(LOG_INFO, "Timer rect start: %d, height: %d, width: %d\n", start_x, height, timer_rect.w);
	return (timer_rect);
}

/**
 * @brief Build the name rectangle
 * @param h The SDLHandle pointer
 * @param is_bot_band A flag to indicate if the name is in the bot band
 * @return SDL_Rect The name rectangle
*/
SDL_Rect pc_build_name_rect(SDLHandle *h, s8 is_bot_band) {
	SDL_Rect name_rect = {0}; 

	/* Window end width - size band right - (size_band_right / 16)*/
	s32 name_padding = h->band_size.right >> 4;
	
	s32 start_x = (h->window_size.x - h->band_size.right) + name_padding;
	s32 rect_end_x = h->window_size.x - name_padding;

	s32 rect_height = h->tile_size.x >> 1;

	name_rect.x = start_x;

	if (is_bot_band) {
		/* Start to the bot of window - band_size_bot - rect_height */
		name_rect.y = h->window_size.y - (h->band_size.bot) - (rect_height) - h->timer_rect_bot.h - (h->timer_rect_bot.h >> 2);
	} else {
		name_rect.y = h->band_size.top + h->timer_rect_top.h + (h->timer_rect_top.h >> 2);
	}
	

	name_rect.w = rect_end_x - start_x;
	name_rect.h = rect_height;

	CHESS_LOG(LOG_INFO, ORANGE"Name rect start: %d, end: %d, height: %d, width: %d\n"RESET, start_x, rect_end_x, rect_height, name_rect.w);
	return (name_rect);
}

SDL_Rect android_build_name_rect(SDLHandle *h, s8 is_bot_band) {
	SDL_Rect name_rect = {0}; 

	// s32 height_pad = (h->band_size.bot >> 2) + (h->band_size.bot >> 3);
	s32 width = (h->window_size.x >> 1) - (h->window_size.x >> 2);
	s32 height = h->tile_size.x >> 1;


	s32 pad = height + (height >> 1);

	if (is_bot_band) {
		name_rect.x = h->timer_rect_bot.x;
		name_rect.y = h->timer_rect_bot.y + h->timer_rect_bot.h + pad - height;
	} else {
		name_rect.x = h->timer_rect_top.x;
		name_rect.y = h->timer_rect_top.y - (pad);
	}
	

	name_rect.w = width;
	name_rect.h = height;

	CHESS_LOG(LOG_INFO, "Name rect start: %d, height: %d, width: %d\n", name_rect.x, height, name_rect.w);
	return (name_rect);
}