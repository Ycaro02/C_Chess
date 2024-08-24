#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
// #include "../rsc/lib/install/include/SDL2/SDL_ttf.h"

/* Texture path */
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


/* Element size in pixel */
#define		TOP_BAND_HEIGHT		0
#define		TILE_SIZE			60
#define		TILE_SPACING		0

/* Circle size */
#define		CIRCLE_RADIUS ((TILE_SIZE >> 3) + (TILE_SIZE >> 5))
#define		OUTLINE_CIRCLE_RADIUS (TILE_SIZE >> 1)

/* Window size */
#define WINDOW_WIDTH (8 * TILE_SIZE + 9 * TILE_SPACING)
#define WINDOW_HEIGHT (8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT)

/* Macro to convert tile position to pixel position */
#define TILE_POSITION_TO_PIXEL(p, px, py) \
    do { \
        (px) = (p).x * TILE_SIZE + ((p).x + 1) * TILE_SPACING; \
        (py) = ((p).y * TILE_SIZE + ((p).y + 1) * TILE_SPACING) + TOP_BAND_HEIGHT; \
    } while (0)



#define SDL_ERR_FUNC() ft_printf_fd(2, "SDL Error %s: %s\n", __func__, SDL_GetError())
// #define TTF_ERR_FUNC() ft_printf_fd(2, "TTF Error %s: %s\n", __func__, TTF_GetError())

typedef struct s_iVec2 {
	s32 x;
	s32 y;
} iVec2;


typedef struct s_sdl_handle {
	SDL_Window		*window;
	SDL_Renderer	*renderer;
	SDL_Texture		**piece_texture;
	ChessBoard		*board;
	PlayerInfo		player_info;
} SDLHandle;

/* src/sdl_handle */
SDLHandle	*create_sdl_handle(u32 width , u32 height, const char* title);
u8 			window_is_open(SDL_Window* window);
void		window_clear(SDL_Renderer* renderer);
void		window_close(SDL_Window* window, SDL_Renderer *renderer);
SDL_Texture	*load_texture(SDL_Renderer *renderer, const char* path);
void		unload_texture(SDL_Texture *texture);
void		draw_texture_tile(SDL_Renderer *renderer, SDL_Texture *texture, iVec2 tilePos, iVec2 scale);
void		draw_color_tile(SDL_Renderer	*renderer , iVec2 tilePos, iVec2 scale, u32 color);


#endif /* HANDLE_SDL_H */