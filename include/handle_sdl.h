#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"

#define UINT32_TO_RGBA(color, r, g, b, a) {\
	r = (color & 0xFF000000) >> 24;\
	g = (color & 0x00FF0000) >> 16;\
	b = (color & 0x0000FF00) >> 8;\
	a = (color & 0x000000FF);\
}\


#define SDL_ERR_FUNC() ft_printf_fd(2, "SDL Error %s: %s\n", __func__, SDL_GetError())
#define TTF_ERR_FUNC() ft_printf_fd(2, "TTF Error %s: %s\n", __func__, TTF_GetError())

typedef struct iVec2 {
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

#endif /* HANDLE_SDL_H */