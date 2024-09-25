#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"
#include "../include/android_macro.h"

/**
 * @brief Create a window with SDL2
 * @param width The width of the window
 * @param height The height of the window
 * @param title The title of the window
 * @return The window pointer
*/
SDL_Window* create_sdl_windows(SDLHandle *h, const char* title) {
	SDL_Window		*window = NULL;
	SDL_Renderer	*renderer = NULL;
	
	/* Init SDL2 and TTF */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_ERR_FUNC();
		return (NULL);
	}

	if (TTF_Init() != 0) {
		TTF_ERR_FUNC();
		SDL_Quit();
		return (NULL);
	}

	COMPUTE_WIN_BAND_SIZE(h);

	if (!init_menu(h, BTN_MAX)) {
		TTF_Quit();
		SDL_Quit();
		return (NULL);
	}

	h->center_text = center_text_init(h);


	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, h->window_size.x, h->window_size.y, SDL_WINDOW_SHOWN);
	if (!window) {
		SDL_ERR_FUNC();
		return (NULL);
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_ERR_FUNC();
		SDL_DestroyWindow(window);
		return (NULL);
	}

	/* Enable blending mode to handle alpha */
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return (window);
}

/**
 * @brief Load a texture with SDL2
 * @param renderer The renderer pointer
 * @param path The path of the texture
 * @return The texture pointer
*/
static SDL_Texture *safe_load_texture(SDL_Renderer *renderer, const char *path) {
	SDL_Texture *texture = BMP_TO_TEXTURE(renderer, path);
	if (!texture) {
		CHESS_LOG(LOG_ERROR, "%s: load_texture %s failed\n", __func__, path);
		return (NULL);
	}
	return (texture);
}


/**
 * @brief Load the piece texture
 * @param handle The SDLHandle pointer
 * @return TRUE if success, FALSE otherwise
*/
static s8 load_piece_texture(SDLHandle *handle) {
	handle->piece_texture = malloc(sizeof(SDL_Texture*) * PIECE_MAX);
	if (!handle->piece_texture) {
		CHESS_LOG(LOG_ERROR, "%s: malloc failed\n", __func__);
		return (FALSE);
	}
	
	/* Load black pieces */
	handle->piece_texture[BLACK_KING] = safe_load_texture(handle->renderer, BLACK_KING_TEXTURE);
	handle->piece_texture[BLACK_QUEEN] = safe_load_texture(handle->renderer, BLACK_QUEEN_TEXTURE);
	handle->piece_texture[BLACK_ROOK] = safe_load_texture(handle->renderer, BLACK_ROOK_TEXTURE);
	handle->piece_texture[BLACK_BISHOP] = safe_load_texture(handle->renderer, BLACK_BISHOP_TEXTURE);
	handle->piece_texture[BLACK_KNIGHT] = safe_load_texture(handle->renderer, BLACK_KNIGHT_TEXTURE);
	handle->piece_texture[BLACK_PAWN] = safe_load_texture(handle->renderer, BLACK_PAWN_TEXTURE);

	/* Load white pieces */
	handle->piece_texture[WHITE_KING] = safe_load_texture(handle->renderer, WHITE_KING_TEXTURE);
	handle->piece_texture[WHITE_QUEEN] = safe_load_texture(handle->renderer, WHITE_QUEEN_TEXTURE);
	handle->piece_texture[WHITE_ROOK] = safe_load_texture(handle->renderer, WHITE_ROOK_TEXTURE);
	handle->piece_texture[WHITE_BISHOP] = safe_load_texture(handle->renderer, WHITE_BISHOP_TEXTURE);
	handle->piece_texture[WHITE_KNIGHT] = safe_load_texture(handle->renderer, WHITE_KNIGHT_TEXTURE);
	handle->piece_texture[WHITE_PAWN] = safe_load_texture(handle->renderer, WHITE_PAWN_TEXTURE);

	for (s32 i = 0; i < PIECE_MAX; i++) {
		if (!handle->piece_texture[i]) {
			CHESS_LOG(LOG_ERROR, "%s: load_texture failed\n", __func__);
			for (s32 j = 0; j < i; j++) {
				unload_texture(handle->piece_texture[j]);
			}
			return (FALSE);
		}
	}
	return (TRUE);
}


static TTF_Font *safe_load_font(SDLHandle *h, const char *path, s32 fontSize) {
	TTF_Font *font = load_font(path, fontSize);
	if (!font) {
		SDL_DestroyRenderer(h->renderer);
		SDL_DestroyWindow(h->window);
		free(h);
		CHESS_LOG(LOG_ERROR, "%s: load_font %s failed\n", __func__, path);
		return (NULL);
	}
	return (font);
}

/**
 * @brief Create a SDLHandle
 * @param width The width of the window
 * @param height The height of the window
 * @param title The title of the window
 * @param board The chess board
 * @return The SDLHandle pointer
*/
SDLHandle *create_sdl_handle(const char* title) {
	SDLHandle *handle = ft_calloc(sizeof(SDLHandle),1);
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "%s : malloc failed\n", __func__);
		return (NULL);
	} else if (!(handle->window = create_sdl_windows(handle, title))) {
		free(handle);
		return (NULL);
	} else if (!(handle->renderer = SDL_GetRenderer(handle->window))) {
		SDL_ERR_FUNC();
		SDL_DestroyWindow(handle->window);
		free(handle);
		return (NULL);
	} else if (!load_piece_texture(handle)) {
		SDL_DestroyRenderer(handle->renderer);
		SDL_DestroyWindow(handle->window);
		free(handle);
		return (NULL);
	} else if (!(handle->tile_font = load_font(FONT_PATH, (handle->tile_size.x >> 2)))) {
		SDL_DestroyRenderer(handle->renderer);
		SDL_DestroyWindow(handle->window);
		free(handle);
		return (NULL);
	}

	handle->player_info.turn = FALSE;
	handle->player_info.color = IS_WHITE;
	handle->over_piece_select = EMPTY;
	handle->game_start = FALSE;

	/* Init timer rect and font */
	handle->timer_rect_bot = BUILD_TIMER_RECT(handle, TRUE);
	handle->timer_rect_top = BUILD_TIMER_RECT(handle, FALSE);
	if (!(handle->timer_font = safe_load_font(handle, FONT_PATH, (handle->timer_rect_bot.h >> 1)))) {
		return (NULL);
	}
	fast_bzero(handle->timer_str, TIME_STR_SIZE);
	handle->my_remaining_time = 60 * 30;
	handle->enemy_remaining_time = 60 * 30;

	/* Init name rect */
	handle->name_rect_bot = BUILD_NAME_RECT(handle, TRUE);
	handle->name_rect_top = BUILD_NAME_RECT(handle, FALSE);
	if (!(handle->name_font = safe_load_font(handle, FONT_PATH, (handle->name_rect_bot.h >> 1) + (handle->name_rect_bot.h >> 2)))) {
		return (NULL);
	}
	if (!(handle->piece_diff_font = safe_load_font(handle, FONT_PATH, (handle->tile_size.x >> 2))) ) {
		return (NULL);
	}

	window_clear(handle->renderer);
	return (handle);
}


/**
 * @brief Clear the window with SDL2
 * @param window The window pointers
*/
void window_clear(SDL_Renderer* renderer) {
	// SDL_SetRenderDrawColor(renderer, 0, 0, 120, 255);
	SDL_SetRenderDrawColor(renderer, CLEAR_COLOR);
	SDL_RenderClear(renderer);
}

/**
 * @brief Close the window with SDL2
 * @param window The window pointers
 * @note We need to destroy renderer before destroying the window
 */
void window_close(SDL_Window* window, SDL_Renderer *renderer) {

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
	}
}

/**
 * @brief Draw a color tile with SDL2
 * @param window The window pointers
 * @param tile_pos, The position of the tile
 * @param scale The scale of the tile
 * @note If scale.x/y are equal to TILE_SIZE, we use TILE_SPACING to space the tiles
*/
void draw_color_tile(SDLHandle *h, iVec2 tile_pos, iVec2 scale, u32 color) {
	SDL_Rect		tileRect = {0,0,0,0};
	iVec2			pixel_pos = {0,0};
	u8 				r, g, b, a;

	UINT32_TO_RGBA(color, r, g, b, a);

	/* Convert tile coordinates to pixel coordinates */
	if (scale.x == h->tile_size.x && scale.y == h->tile_size.y) {
		tile_to_pixel_pos(&pixel_pos, tile_pos, scale.x, h->band_size);
	} else {
		pixel_pos.x = tile_pos.x;
		pixel_pos.y = tile_pos.y;
	}

	tileRect.x = pixel_pos.x; tileRect.y = pixel_pos.y;
	tileRect.w = scale.x; tileRect.h = scale.y;

	/* Set the drawing color and draw the tile */
	SDL_SetRenderDrawColor(h->renderer, r, g, b, a);
	SDL_RenderFillRect(h->renderer, &tileRect);
}

/**
 * @brief Draw a texture tile with SDL2
 * @param window The window pointers
 * @param texture The texture pointer
 * @param tile_pos, The position of the tile
 * @param scale The scale of the tile
 * @note If the scale is equal to TILE_SIZE, the function will draw the tile at the right position
*/
void draw_texture_tile(SDLHandle *h, SDL_Texture *texture, iVec2 tile_pos, iVec2 scale) {
	SDL_Rect 	dstRect;
	iVec2		pixel_pos = {0,0};
	
	if (!texture || !h->renderer) {
		return;
	}
	/* Convert tile coordinates to pixel coordinates */
	if (scale.x == h->tile_size.x && scale.y == h->tile_size.y) {
		tile_to_pixel_pos(&pixel_pos, tile_pos, scale.x, h->band_size);
	} else {
		pixel_pos.x = tile_pos.x;
		pixel_pos.y = tile_pos.y;
	}

	dstRect.x = pixel_pos.x; dstRect.y = pixel_pos.y;
	dstRect.w = scale.x; dstRect.h = scale.y;
	SDL_RenderCopy(h->renderer, texture, NULL, &dstRect);
}


/**
 * @brief Draw a texture with SDL2
 * @param window The window pointers
 * @param texture The texture pointer
 * @param pos The position of the texture
 * @param scale The scale of the texture
*/
void draw_texure(SDLHandle *handle, SDL_Texture *texture, iVec2 pos, iVec2 scale) {
	SDL_Rect 	dstRect = { .x = pos.x, .y = pos.y, .w = scale.x, .h = scale.y };	

	SDL_RenderCopy(handle->renderer, texture, NULL, &dstRect);
}

/**
 * @brief Load a texture from a path with SDL2
 * @param window The window pointers
 * @param path The path of the texture
 * @return The texture pointer
*/
SDL_Texture *pc_load_texture(SDL_Renderer *renderer, const char* path) {
	SDL_Texture		*texture = NULL;
	SDL_Surface		*surface = NULL;

	if (!renderer) {
		return (NULL);
	}

	// surface = BMP_TO_SURFACE(path);
	surface = SDL_LoadBMP(path);
	if (!surface) {
		SDL_ERR_FUNC();
		return (NULL);
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) {
		SDL_ERR_FUNC();
		return (NULL);
	}
	SDL_FreeSurface(surface);
	return (texture);
}

/**
 * @brief Unload a texture with SDL2
 * @param texture The texture pointer
*/
void unload_texture(SDL_Texture *texture) {
	if (!texture) {
		return ;
	}
	SDL_DestroyTexture(texture);
}

void destroy_sdl_handle(SDLHandle *handle) {
	if (!handle) {
		return ;
	}

	/* free board */
	if (handle->board) {
		free(handle->board);
	}
	
	/* Unload texture */
	if (handle->piece_texture) {
		for (int i = 0; i < PIECE_MAX; i++) {
			unload_texture(handle->piece_texture[i]);
		}
		free(handle->piece_texture);
	}


	/* unload font */
	if (handle->tile_font) {
		unload_font(handle->tile_font);
	}
	if (handle->timer_font) {
		unload_font(handle->timer_font);
	}
	if (handle->name_font) {
		unload_font(handle->name_font);
	}
	if (handle->piece_diff_font) {
		unload_font(handle->piece_diff_font);
	}

	/* free center_text */
	center_text_destroy(handle->center_text);

	/* Close window */
	window_close(handle->window, handle->renderer);

	/* Free ip addr */
	if (handle->player_info.dest_ip) {
		free(handle->player_info.dest_ip);
	}

	/* Free player name */
	if (handle->player_info.name) {
		free(handle->player_info.name);
	}


	/* Free network info and send disconnect to server */
	destroy_network_info(handle);
}


/**
 * @brief Load a font with SDL2
 * @param path The path of the font
 * @return The font pointer
*/
TTF_Font *load_font(const char *path, s32 fontSize) {
	TTF_Font *font = TTF_OpenFont(path, fontSize);
	if (!font) {
		return (NULL);
	}
	return (font);
}

/**
 * @brief Unload a font with SDL2
 * @param font The font pointer
*/
void unload_font(TTF_Font *font) {
	if (font) {
		TTF_CloseFont(font);
	}
}

/**
 * @brief Write a text with SDL2
 * @param window The window pointers
 * @param font The font pointer
 * @param text The text to write
 * @param pos The position of the text
 * @param fontSize The size of the text
 * @param color The color of the text
*/
void write_text(SDLHandle *h, char *text, TTF_Font *font, iVec2 pos, u32 color) {
	SDL_Surface		*surface = NULL;
	SDL_Texture		*texture = NULL;
	SDL_Rect		textRect = {0,0,0,0};
	u8 				r, g, b, a;

	if (!h->window || !font) {
		return;
	}

	UINT32_TO_RGBA(color, r, g, b, a);

	surface = TTF_RenderText_Solid(font, text, (SDL_Color){r, g, b, a});
	if (!surface) {
		CHESS_LOG(LOG_ERROR, "%s\n", SDL_GetError());
		return;
	}
	texture = SDL_CreateTextureFromSurface(h->renderer, surface);
	if (!texture) {
		CHESS_LOG(LOG_ERROR, "%s\n", SDL_GetError());
		SDL_FreeSurface(surface);
		return;
	}

	textRect.x = pos.x;
	textRect.y = pos.y;
	textRect.w = surface->w;
	textRect.h = surface->h;
	SDL_RenderCopy(h->renderer, texture, NULL, &textRect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

/* Function to draw a filled circle */
void draw_filled_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

/* Function to draw a circle outline */
void draw_circle_outline(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            int distance_squared = dx * dx + dy * dy;
            if (distance_squared <= (radius * radius) && distance_squared >= ((radius - 5) * (radius - 5))) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}
