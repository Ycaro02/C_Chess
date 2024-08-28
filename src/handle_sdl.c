#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


void get_screen_size(int *width, int *height) {
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
 * @brief Create a window with SDL2
 * @param width The width of the window
 * @param height The height of the window
 * @param title The title of the window
 * @return The window pointer
*/
SDL_Window* createWindow(SDLHandle * h, const char* title) {
	SDL_Window		*window = NULL;
	SDL_Renderer	*renderer = NULL;
	
	/* Init SDL2 and TTF */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_ERR_FUNC();
		return (NULL);
	}
	// if (TTF_Init() != 0) {
	// 	TTF_ERR_FUNC();
	// 	SDL_Quit();
	// 	return (NULL);
	// }

	s32 size_w=0, size_h=0, width = 0, height = 0;

	get_screen_size(&size_w, &size_h);
	
	
	// Adjust width and height by removing 1/4 of each dimension, for information (display coilumn and row)
    width = size_w - (size_w / 4);
    height = size_h - (size_h / 4);

    // Calculate tile size for the chessboard
	int minus = width < height ? width : height;
    int tile_size = minus / 8;

	h->tile_size.x = tile_size;
	h->tile_size.y = tile_size;

    printf("Tile size: %d\n", tile_size);

	// Calculate band size
	int band_w = (size_w / 8);
	int band_h = (size_h / 16);


	// Calculate the band size bot and left for raw,column number display
	h->band_size.bot = band_h;
	h->band_size.left = band_w / 4;

	// Detect minus between left and bot band size
	minus = h->band_size.left < h->band_size.bot ? h->band_size.left : h->band_size.bot;

	// Set the band size
	h->band_size.left = minus;
	h->band_size.bot = minus;

	// Set the band size right and top
	h->band_size.right = band_w - h->band_size.left;
	h->band_size.top = 0;

	// Set the band size height for the window
	band_h = h->band_size.bot;

	// Calculate the window size
	width = (8 * tile_size) + band_w;
	height = (8 * tile_size) + band_h;


	printf("Width: %d, Height: %d\n", width, height);

	printf("Band height: %d\n", band_h);
	printf("Band width: %d\n", band_w);

	printf("Top band: %d, Bot band: %d, Left band: %d, Right band: %d\n", h->band_size.top, h->band_size.bot, h->band_size.left, h->band_size.right);

	h->window_size.x = width;
	h->window_size.y = height;




	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
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

static SDL_Texture *safe_load_texture(SDL_Renderer *renderer, const char *path) {
	SDL_Texture *texture = load_texture(renderer, path);
	if (!texture) {
		CHESS_LOG(LOG_ERROR, "%s: load_texture %s failed\n", __func__, path);
		return (NULL);
	}
	return (texture);
}

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


/**
 * @brief Create a SDLHandle
 * @param width The width of the window
 * @param height The height of the window
 * @param title The title of the window
 * @param board The chess board
 * @return The SDLHandle pointer
*/
SDLHandle *create_sdl_handle(const char* title) {
	SDLHandle *handle = malloc(sizeof(SDLHandle));
	if (!handle) {
		CHESS_LOG(LOG_ERROR, "%s : malloc failed\n", __func__);
		return (NULL);
	}

	handle->window = createWindow(handle, title);
	if (!handle->window) {
		free(handle);
		return (NULL);
	}
	handle->renderer = SDL_GetRenderer(handle->window);
	if (!handle->renderer) {
		SDL_ERR_FUNC();
		SDL_DestroyWindow(handle->window);
		free(handle);
		return (NULL);
	}
	if (!load_piece_texture(handle)) {
		SDL_DestroyRenderer(handle->renderer);
		SDL_DestroyWindow(handle->window);
		free(handle);
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
	SDL_SetRenderDrawColor(renderer, 0, 0, 120, 255);
	SDL_RenderClear(renderer);
}

/**
 * @brief Check if the window is open with SDL2
 * @param window The window pointers
 * @return true if the window is open, false otherwise
 * @note This function is little bit different from the other lib
 * 	- SDL2 doesn't have a function to check if the window is open
 * 	- So we will check if the window ptr is not null
*/
u8 window_is_open(SDL_Window* window) {
	return (window != NULL);
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
		SDL_Quit();
	}
}

/**
 * @brief Draw a color tile with SDL2
 * @param window The window pointers
 * @param tilePos, The position of the tile
 * @param scale The scale of the tile
 * @note If scale.x/y are equal to TILE_SIZE, we use TILE_SPACING to space the tiles
*/
void draw_color_tile(SDLHandle *h, iVec2 tilePos, iVec2 scale, u32 color) {
	SDL_Rect		tileRect = {0,0,0,0};
	s32				pixel_x = 0, pixel_y = 0;
	u8 				r, g, b, a;

	UINT32_TO_RGBA(color, r, g, b, a);

	/* Convert tile coordinates to pixel coordinates */
	// if (scale.x == TILE_SIZE && scale.y == TILE_SIZE) {
	TILE_POSITION_TO_PIXEL(tilePos, pixel_x, pixel_y, scale.x, h->band_size);
	// } else {
	// 	pixel_x = tilePos.x;
	// 	pixel_y = tilePos.y;
	// }

	tileRect.x = pixel_x; tileRect.y = pixel_y;
	tileRect.w = scale.x; tileRect.h = scale.y;

	/* Set the drawing color and draw the tile */
	SDL_SetRenderDrawColor(h->renderer, r, g, b, a);
	SDL_RenderFillRect(h->renderer, &tileRect);
}

/**
 * @brief Draw a texture tile with SDL2
 * @param window The window pointers
 * @param texture The texture pointer
 * @param tilePos, The position of the tile
 * @param scale The scale of the tile
 * @note If the scale is equal to TILE_SIZE, the function will draw the tile at the right position
*/
void draw_texture_tile(SDLHandle *h, SDL_Texture *texture, iVec2 tilePos, iVec2 scale) {
	SDL_Rect 	dstRect;
	s32 		pixel_x, pixel_y;
	
	if (!texture || !h->renderer) {
		return;
	}
	/* Convert tile coordinates to pixel coordinates */
	// if (scale.x == TILE_SIZE && scale.y == TILE_SIZE) {
	TILE_POSITION_TO_PIXEL(tilePos, pixel_x, pixel_y, scale.x, h->band_size);
	// } else {
		// pixel_x = tilePos.x;
		// pixel_y = tilePos.y;
	// }

	dstRect.x = pixel_x;
	dstRect.y = pixel_y;
	dstRect.w = scale.x;
	dstRect.h = scale.y;
	SDL_RenderCopy(h->renderer, texture, NULL, &dstRect);
}

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
SDL_Texture *load_texture(SDL_Renderer *renderer, const char* path) {
	SDL_Texture		*texture = NULL;
	SDL_Surface		*surface = NULL;

	if (!renderer) {
		return (NULL);
	}
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

	/* Close window */
	window_close(handle->window, handle->renderer);

	/* Free ip addr */
	if (handle->player_info.dest_ip) {
		free(handle->player_info.dest_ip);
	}

	/* Free network info and send disconnect to server */
	if (handle->player_info.nt_info) {
		CHESS_LOG(LOG_INFO, ORANGE"Send disconnect to server%s\n", RESET);
		send_disconnect_to_server(handle->player_info.nt_info->sockfd, handle->player_info.nt_info->servaddr);
		close(handle->player_info.nt_info->sockfd);
		free(handle->player_info.nt_info);
	}
}


/**
 * @brief Load a font with SDL2
 * @param path The path of the font
 * @return The font pointer
*/
// void *loadFont(const char *path, s32 fontSize) {
// 	TTF_Font *font = TTF_OpenFont(path, fontSize);
// 	if (!font) {
// 		return (NULL);
// 	}
// 	return (font);
// }

/**
 * @brief Unload a font with SDL2
 * @param font The font pointer
*/
// void unloadFont(TTF_Font *font) {
// 	if (font) {
// 		TTF_CloseFont(font);
// 	}
// }

/**
 * @brief Write a text with SDL2
 * @param window The window pointers
 * @param font The font pointer
 * @param text The text to write
 * @param pos The position of the text
 * @param fontSize The size of the text
 * @param color The color of the text
*/
// void writeText(SDL_Window *window, TTF_Font *font, char *text, iVec2 pos, u32 fontSize, u32 color) {
// 	SDL_Renderer	*renderer = NULL;
// 	SDL_Surface		*textSurface = NULL;
// 	SDL_Texture		*textTexture = NULL;
// 	SDL_Rect		textRect = {0,0,0,0};
// 	u8 				r, g, b, a;

// 	(void)fontSize;

// 	if (!window || !font) {
// 		return;
// 	}

// 	renderer = SDL_GetRenderer(window);
// 	if (!renderer) {
// 		std::cerr << "SDL_GetRenderer Error: " << SDL_GetError() << std::endl;
// 		return;
// 	}

// 	UINT32_TO_RGBA(color, r, g, b, a);

// 	textSurface = TTF_RenderText_Solid(font, text, {r, g, b, a});
// 	if (!textSurface) {
// 		std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
// 		return;
// 	}
// 	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
// 	if (!textTexture) {
// 		std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
// 		SDL_FreeSurface(textSurface);
// 		return;
// 	}

// 	textRect.x = pos.x;
// 	textRect.y = pos.y;
// 	textRect.w = textSurface->w;
// 	textRect.h = textSurface->h;
// 	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
// 	SDL_FreeSurface(textSurface);
// 	SDL_DestroyTexture(textTexture);
// }