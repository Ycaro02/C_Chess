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

SDLHandle *createSDLHandle(u32 width , u32 height, const char* title) {
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
	windowClear(handle->renderer);
	return (handle);
}

/**
 * @brief Clear the window with SDL2
 * @param window The window pointers
*/
void windowClear(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
void windowClose(SDL_Window* window) {
	SDL_Renderer *renderer = NULL;
	if (window) {
		renderer = SDL_GetRenderer(window);
		if (!renderer) {
			SDL_ERR_FUNC();
			return;
		}
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
	}
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
	pixel_x = tilePos.x; pixel_y = tilePos.y;

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
// void drawTextureTile(SDL_Window *window, SDL_Texture *texture, iVec2 tilePos, iVec2 scale) {
// 	SDL_Renderer *renderer = SDL_GetRenderer(window);
// 	SDL_Rect 	dstRect;
// 	s32 		pixel_x, pixel_y;
	
// 	if (!texture || !renderer) {
// 		return;
// 	}

// 	/* Convert tile coordinates to pixel coordinates */
// 	if (scale.x == TILE_SIZE && scale.y == TILE_SIZE) {
// 		TILE_POSITION_TO_PIXEL(tilePos, pixel_x, pixel_y);
// 	} else {
// 		pixel_x = tilePos.x;
// 		pixel_y = tilePos.y;
// 	}

// 	dstRect.x = pixel_x;
// 	dstRect.y = pixel_y;
// 	dstRect.w = scale.x;
// 	dstRect.h = scale.y;
// 	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
// }

/**
 * @brief Load a texture from a path with SDL2
 * @param window The window pointers
 * @param path The path of the texture
 * @return The texture pointer
*/
// void *loadTexture(SDL_Window* window, const char* path) {
// 	SDL_Renderer	*renderer = NULL;
// 	SDL_Texture		*texture = NULL;
// 	SDL_Surface		*surface = NULL;

// 	renderer = SDL_GetRenderer(window);
// 	if (!renderer) {
// 		return (NULL);
// 	}
// 	surface = SDL_LoadBMP(path);
// 	if (!surface) {
// 		std::cerr << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
// 		return (NULL);
// 	}
// 	texture = SDL_CreateTextureFromSurface(renderer, surface);
// 	if (!texture) {
// 		std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
// 		return (NULL);
// 	}
// 	SDL_FreeSurface(surface);
// 	return (texture);
// }

/**
 * @brief Unload a texture with SDL2
 * @param texture The texture pointer
*/
// void unloadTexture(SDL_Texture *texture) {
// 	if (!texture) {
// 		return ;
// 	}
// 	SDL_DestroyTexture(texture);
// }

/**
 * @brief Destructor for the SDL2 library
 * @note This function will quit the SDL2 subsystem
*/
// void libDestructor() {
// 	if (SDL_WasInit(SDL_INIT_VIDEO) != 0) {
// 		TTF_Quit();
// 		SDL_Quit();
// 	}
// }


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