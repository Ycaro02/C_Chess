#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"

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
// #define		TILE_SIZE			80
#define		TILE_SPACING		0

/* Circle size */
#define		CIRCLE_RADIUS(_ts_) ((_ts_ >> 3) + (_ts_ >> 5))
#define		OUTLINE_CIRCLE_RADIUS(_ts_) (_ts_ >> 1)


#define SDL_ERR_FUNC() printf("SDL Error %s: %s\n", __func__, SDL_GetError())
#define TTF_ERR_FUNC() printf("TTF Error %s: %s\n", __func__, TTF_GetError())

typedef struct s_iVec2 {
	s32 x;
	s32 y;
} iVec2;

typedef struct s_window_band {
	s32 top;
	s32 left;
	s32 right;
	s32 bot;
} WinBand;

typedef struct s_sdl_handle {
	SDL_Window		*window;			/* The window ptr */
	SDL_Renderer	*renderer;			/* The renderer ptr */
	SDL_Texture		**piece_texture;	/* Array of texture for each piece */
	ChessBoard		*board;				/* The chess board */
	TTF_Font		*font;				/* The font */
	iVec2			window_size;		/* The size of the window */
	iVec2			tile_size;			/* The size of the tile */
	WinBand			band_size;			/* The band size */
	PlayerInfo		player_info;		/* Player info */
	iVec2			mouse_pos;			/* Mouse position */
	ChessPiece		over_piece_select;	/* The piece selected (over display) */
	u32				flag;				/* App Flag */
} SDLHandle;



#define FONT_SIZE 24
#define FONT_SHIFT (FONT_SIZE >> 1)
#define FONT_PATH "rsc/font/arial.ttf"

/* @brief Macro to convert tile position to pixel position
 * @param p The tile position
 * @param px The pixel x position
 * @param py The pixel y position
 * @param _ts_ The tile size
 * @param _wb_ The window band size
*/ 
#define TILE_POSITION_TO_PIXEL(p, px, py, _ts_, _wb_) \
    do { \
        (px) = ((p).x * _ts_) + _wb_.left ; \
        (py) = ((p).y * _ts_) + _wb_.top; \
    } while (0)

FT_INLINE void tile_to_pixel_pos(iVec2 p, iVec2 *pos, s32 tile_size, WinBand wb) {
	pos->x = (p.x * tile_size) + wb.left;
	pos->y = (p.y * tile_size) + wb.top;
}

FT_INLINE s8 is_in_x_range(s32 x, s32 raw, s32 tile_size, WinBand wb) {
	return (x >= (raw * tile_size) + wb.left && x <= ((raw + 1) * tile_size) + wb.left);
}

FT_INLINE s8 is_in_y_range(s32 y, s32 column, s32 tile_size, WinBand wb) {
	return (y >= (column * tile_size) + wb.top && y <= ((column + 1) * tile_size) + wb.top);
}

/* src/sdl_handle */
SDLHandle	*create_sdl_handle(const char* title);
u8 			window_is_open(SDL_Window* window);
void		window_clear(SDL_Renderer* renderer);
void		window_close(SDL_Window* window, SDL_Renderer *renderer);
SDL_Texture	*load_texture(SDL_Renderer *renderer, const char* path);
void		unload_texture(SDL_Texture *texture);
void		draw_texture_tile(SDLHandle *h, SDL_Texture *texture, iVec2 tilePos, iVec2 scale);
void		draw_texure(SDLHandle *handle, SDL_Texture *texture, iVec2 pos, iVec2 scale);
void		draw_color_tile(SDLHandle *h, iVec2 tilePos, iVec2 scale, u32 color);
void		destroy_sdl_handle(SDLHandle *handle);
TTF_Font	*load_font(const char *path, s32 size);
void		unload_font(TTF_Font *font);
void 		write_text(SDLHandle *h, char *text, iVec2 pos, u32 color);
#endif /* HANDLE_SDL_H */