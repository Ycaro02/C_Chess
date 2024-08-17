#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"

#define BLACK_KING_TEXTURE "rsc/texture/piece/black_king.bmp"
#define BLACK_QUEEN_TEXTURE "rsc/texture/piece/black_queen.bmp"
#define BLACK_ROOK_TEXTURE "rsc/texture/piece/black_rook.bmp"
#define BLACK_BISHOP_TEXTURE "rsc/texture/piece/black_bishop.bmp"
#define BLACK_KNIGHT_TEXTURE "rsc/texture/piece/black_knight.bmp"
#define BLACK_PAWN_TEXTURE "rsc/texture/piece/black_pawn.bmp"

#define WHITE_KING_TEXTURE "rsc/texture/piece/white_king.bmp"
#define WHITE_QUEEN_TEXTURE "rsc/texture/piece/white_queen.bmp"
#define WHITE_ROOK_TEXTURE "rsc/texture/piece/white_rook.bmp"
#define WHITE_BISHOP_TEXTURE "rsc/texture/piece/white_bishop.bmp"
#define WHITE_KNIGHT_TEXTURE "rsc/texture/piece/white_knight.bmp"
#define WHITE_PAWN_TEXTURE "rsc/texture/piece/white_pawn.bmp"


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
#define		TILE_SPACING		0

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
	SDL_Texture		**piece_texture;
	ChessBoard		*board;
} SDLHandle;

/* src/sdl_handle */
SDLHandle	*createSDLHandle(u32 width , u32 height, const char* title, ChessBoard *board);
u8 			windowIsOpen(SDL_Window* window);
void		windowClear(SDL_Renderer* renderer);
void		windowClose(SDL_Window* window, SDL_Renderer *renderer);

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char* path);
void unloadTexture(SDL_Texture *texture);
void drawTextureTile(SDL_Renderer *renderer, SDL_Texture *texture, iVec2 tilePos, iVec2 scale);

void colorTile(SDL_Renderer	*renderer , iVec2 tilePos, iVec2 scale, u32 color);
void draw_board(SDLHandle *handle);

#endif /* HANDLE_SDL_H */