#include "../include/chess.h"

#include "../include/handle_sdl.h"

/**
 * @brief Create a window with SDL2
 * @param width The width of the window
 * @param height The height of the window
 * @param title The title of the window
 * @return The window pointer
*/
SDL_Window* createWindow(u32 width ,u32 height, const char* title) {
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

static SDL_Texture *safeLoadTexture(SDL_Renderer *renderer, const char *path) {
	SDL_Texture *texture = loadTexture(renderer, path);
	if (!texture) {
		ft_printf_fd(2, "Error %s: loadTexture %s failed\n", __func__, path);
		return (NULL);
	}
	return (texture);
}

static void loadPieceTexture(SDLHandle *handle) {
	handle->piece_texture = malloc(sizeof(SDL_Texture*) * PIECE_MAX);
	if (!handle->piece_texture) {
		ft_printf_fd(2, "Error %s: malloc failed\n", __func__);
		return ;
	}
	
	/* Load black pieces */
	handle->piece_texture[BLACK_KING] = safeLoadTexture(handle->renderer, BLACK_KING_TEXTURE);
	handle->piece_texture[BLACK_QUEEN] = safeLoadTexture(handle->renderer, BLACK_QUEEN_TEXTURE);
	handle->piece_texture[BLACK_ROOK] = safeLoadTexture(handle->renderer, BLACK_ROOK_TEXTURE);
	handle->piece_texture[BLACK_BISHOP] = safeLoadTexture(handle->renderer, BLACK_BISHOP_TEXTURE);
	handle->piece_texture[BLACK_KNIGHT] = safeLoadTexture(handle->renderer, BLACK_KNIGHT_TEXTURE);
	handle->piece_texture[BLACK_PAWN] = safeLoadTexture(handle->renderer, BLACK_PAWN_TEXTURE);

	/* Load white pieces */
	handle->piece_texture[WHITE_KING] = safeLoadTexture(handle->renderer, WHITE_KING_TEXTURE);
	handle->piece_texture[WHITE_QUEEN] = safeLoadTexture(handle->renderer, WHITE_QUEEN_TEXTURE);
	handle->piece_texture[WHITE_ROOK] = safeLoadTexture(handle->renderer, WHITE_ROOK_TEXTURE);
	handle->piece_texture[WHITE_BISHOP] = safeLoadTexture(handle->renderer, WHITE_BISHOP_TEXTURE);
	handle->piece_texture[WHITE_KNIGHT] = safeLoadTexture(handle->renderer, WHITE_KNIGHT_TEXTURE);
	handle->piece_texture[WHITE_PAWN] = safeLoadTexture(handle->renderer, WHITE_PAWN_TEXTURE);
}

SDLHandle *createSDLHandle(u32 width , u32 height, const char* title, ChessBoard *board) {
	SDLHandle *handle = malloc(sizeof(SDLHandle));
	if (!handle) {
		ft_printf_fd(2, "Error: malloc failed\n");
		return (NULL);
	}
	handle->window = createWindow(width, height, title);
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
	loadPieceTexture(handle);
	handle->board = board;
	windowClear(handle->renderer);
	return (handle);
}

/**
 * @brief Clear the window with SDL2
 * @param window The window pointers
*/
void windowClear(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
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
u8 windowIsOpen(SDL_Window* window) {
	return (window != NULL);
}

/**
 * @brief Close the window with SDL2
 * @param window The window pointers
 * @note We need to destroy renderer before destroying the window
 */
void windowClose(SDL_Window* window, SDL_Renderer *renderer) {
	if (!window || !renderer) {
		return ;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();
}

/**
 * @brief Draw a color tile with SDL2
 * @param window The window pointers
 * @param tilePos, The position of the tile
 * @param scale The scale of the tile
 * @note If scale.x/y are equal to TILE_SIZE, we use TILE_SPACING to space the tiles
*/
void colorTile(SDL_Renderer	*renderer , iVec2 tilePos, iVec2 scale, u32 color) {
	SDL_Rect		tileRect = {0,0,0,0};
	s32				pixel_x = 0, pixel_y = 0;
	u8 				r, g, b, a;

	UINT32_TO_RGBA(color, r, g, b, a);

	/* Convert tile coordinates to pixel coordinates */
	if (scale.x == TILE_SIZE && scale.y == TILE_SIZE) {
		TILE_POSITION_TO_PIXEL(tilePos, pixel_x, pixel_y);
	} else {
		pixel_x = tilePos.x;
		pixel_y = tilePos.y;
	}

	tileRect.x = pixel_x; tileRect.y = pixel_y;
	tileRect.w = scale.x; tileRect.h = scale.y;

	/* Set the drawing color and draw the tile */
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &tileRect);
}

/**
 * @brief Draw a texture tile with SDL2
 * @param window The window pointers
 * @param texture The texture pointer
 * @param tilePos, The position of the tile
 * @param scale The scale of the tile
 * @note If the scale is equal to TILE_SIZE, the function will draw the tile at the right position
*/
void drawTextureTile(SDL_Renderer *renderer, SDL_Texture *texture, iVec2 tilePos, iVec2 scale) {
	SDL_Rect 	dstRect;
	s32 		pixel_x, pixel_y;
	
	if (!texture || !renderer) {
		return;
	}
	/* Convert tile coordinates to pixel coordinates */
	if (scale.x == TILE_SIZE && scale.y == TILE_SIZE) {
		TILE_POSITION_TO_PIXEL(tilePos, pixel_x, pixel_y);
	} else {
		pixel_x = tilePos.x;
		pixel_y = tilePos.y;
	}

	dstRect.x = pixel_x;
	dstRect.y = pixel_y;
	dstRect.w = scale.x;
	dstRect.h = scale.y;
	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

/**
 * @brief Load a texture from a path with SDL2
 * @param window The window pointers
 * @param path The path of the texture
 * @return The texture pointer
*/
SDL_Texture *loadTexture(SDL_Renderer *renderer, const char* path) {
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
void unloadTexture(SDL_Texture *texture) {
	if (!texture) {
		return ;
	}
	SDL_DestroyTexture(texture);
}


static s8 isXRange(s32 x, s32 raw) {
	return (x >= raw * TILE_SIZE + (raw + 1) * TILE_SPACING
			&& x <= (raw + 1) * TILE_SIZE + (raw + 1) * TILE_SPACING);
}

static s8 isYRange(s32 y, s32 column) {
	return (y >= column * TILE_SIZE + (column + 1) * TILE_SPACING + TOP_BAND_HEIGHT
			&& y <= (column + 1) * TILE_SIZE + (column + 1) * TILE_SPACING + TOP_BAND_HEIGHT);
}

/**
 * @brief Detect click tile on the board
 * @param handle The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
ChessTile detectClickTile(s32 x, s32 y) {
	ChessTile tile = A1;
	for (s32 column = 0; column < 8; column++) {
		for (s32 raw = 0; raw < 8; raw++) {
			if (isXRange(x, raw) && isYRange(y, column)) {
				ft_printf_fd(1, "Click on tile %d\n", tile);
				return (tile);
			}
			tile++;
		}
	}
	return (tile);
}


/**
 * @brief Mouse event handler
*/
s32 eventHandler(SDLHandle *handle) {
	SDL_Event event;
	ChessTile tile = INVALID_TILE;
	s32 x = 0, y = 0;
	(void)handle;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT \
			|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
			return (CHESS_QUIT);
		}
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			SDL_GetMouseState(&x, &y);
			tile = detectClickTile(x, y);
		}
	}
	return (tile);
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