#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"


/* Macro to convert RGBA to UINT32 and UINT32 to RGBA */
#define		RGBA_TO_UINT32(r, g, b, a)		((u32)((a << 24) | (r << 16) | (g << 8) | b))

#define		UINT32_TO_RGBA(color, r, g, b, a)	\
	do { \
		r = (color >> 16) & 0xFF;				\
		g = (color >> 8) & 0xFF;				\
		b = color & 0xFF;						\
		a = (color >> 24) & 0xFF;				\
	} while (0)


#define		TOP_BAND_HEIGHT		0
#define		TILE_SIZE			80
#define		TILE_SPACING		1

/* Macro to convert tile position to pixel */
#define TILE_POSITION_TO_PIXEL(p, px, py) \
    do { \
        (px) = (p).x * TILE_SIZE + ((p).x + 1) * TILE_SPACING; \
        (py) = ((p).y * TILE_SIZE + ((p).y + 1) * TILE_SPACING) + TOP_BAND_HEIGHT; \
    } while (0)



#define SDL_ERR_FUNC() ft_printf_fd(2, "SDL Error %s: %s\n", __func__, SDL_GetError())
#define TTF_ERR_FUNC() ft_printf_fd(2, "TTF Error %s: %s\n", __func__, TTF_GetError())

typedef struct s_iVec2 {
	s32 x;
	s32 y;
} iVec2;


typedef struct s_sdl_handle {
	SDL_Window		*window;
	SDL_Renderer	*renderer;
} SDLHandle;

/* src/sdl_handle */
SDLHandle	*createSDLHandle(u32 width ,u32 height, const char* title);
u8 			windowIsOpen(SDL_Window* window);
void		windowClear(SDL_Renderer* renderer);

/* To test */
void colorTile(SDL_Renderer	*renderer , iVec2 tilePos, iVec2 scale, u32 color);

void draw_empty_chess_board(SDLHandle *handle);

#endif /* HANDLE_SDL_H */