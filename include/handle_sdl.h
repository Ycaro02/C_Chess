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


#define BTN_HOVER_COLOR ((SDL_Color){100, 10, 10, 255})
#define BTN_BASIC_COLOR ((SDL_Color){10, 10, 10, 255})
#define BLACK_COLOR 	0, 0, 0, 255
#define WHITE_COLOR 	255, 255, 255, 255
#define U32_WHITE_COLOR RGBA_TO_UINT32(255, 255, 255, 255)
#define U32_BLACK_COLOR RGBA_TO_UINT32(0, 0, 0, 255)


/* Clear color */
#define CLEAR_COLOR 70, 70, 70, 255

/* Menu background color */
#define MENU_BG_COLOR 	70, 70, 70, 220

typedef void (*ButtonFunc)(SDLHandle*);

#define TEXT_INPUT_SIZE 16

#define SERVER_INFO_STR "Server IP: "

typedef struct {
    SDL_Rect	rect;					/* The rect of the text input */
    char		text[TEXT_INPUT_SIZE];	/* The text buffer */
    TTF_Font	*font;					/* The font */
	u64			last_cursor_time;		/* The last cursor time */
	int			buffer_size;			/* The buffer size */
    int			cursor;					/* The cursor position */
	s8			is_active;				/* The text input is active */
	s8			cursor_visible;			/* The cursor is visible */
} TextField;

typedef struct s_button {
	ButtonFunc	func;		/* The button function */
	iVec2		start;		/* The start position */
	iVec2		end;		/* The end position */
	s32			width;		/* The width */
	s32			height;		/* The height */
	char		*text;		/* The button text */
	iVec2		text_pos;	/* The text position */
	s8			state;		/* The button state */
} Button;

typedef struct s_chess_menu {
	/* Server info rect */
	SDL_Rect	server_info;
	iVec2		server_info_str_pos;
	TextField	ip_field;

	/* Menu rect data  */
	iVec2		start;
	iVec2		end;
	s32			width;
	s32			height;

	/* Button data */
	s32			total_btn;
	s32			menu_btn;
	Button		*btn;
	TTF_Font 	*btn_text_font;
	BtnType		current_btn_clicked;
	BtnType		btn_hover;

	/* Menu is open */
	s8			is_open;
} ChessMenu;

typedef struct s_btn_center_tex {
	char		*str;
	ButtonFunc	func;
} BtnCenterText;

typedef struct s_center_text {
	SDL_Rect	rect;
	TTF_Font	*font;
	char		*str;
	char		*str2;
	s32			curent_btn_enable;
} CenterText;

#define TIME_STR_SIZE 16

typedef struct s_sdl_handle {
	SDL_Window		*window;			/* The window ptr */
	SDL_Renderer	*renderer;			/* The renderer ptr */
	SDL_Texture		**piece_texture;	/* Array of texture for each piece */
	ChessBoard		*board;				/* The chess board */

	/* GUI */
	ChessMenu		menu;						/* The menu */
	CenterText		*center_text;				/* The center text */
	TTF_Font		*tile_font;					/* The font for tile number/letters */
	TTF_Font		*timer_font;				/* The font */
	char			timer_str[TIME_STR_SIZE];	/* Timer string */
	iVec2			window_size;				/* The size of the window */
	iVec2			tile_size;					/* The size of the tile */
	iVec2			mouse_pos;					/* Mouse position */
	WinBand			band_size;					/* The band size */
	SDL_Rect		timer_rect_bot;				/* Timer rect */
	SDL_Rect		timer_rect_top;				/* Timer rect */
	ChessPiece		over_piece_select;			/* The piece selected (over display) */

	/* Player info */
	u64				my_remaining_time;		/* Current player ramaining time */
	u64				enemy_remaining_time;	/* Enemy player ramaining time */
	PlayerInfo		player_info;			/* Player info */
	u32				flag;					/* App Flag */
	u16				msg_id;				/* Over flag */
	s8				game_start;				/* Game start flag */
}	SDLHandle ;

#define	FONT_PATH "rsc/font/arial.ttf"

/* @brief Inline func to convert tile position to pixel position
 * @param p The tile position
 * @param px The pixel x position
 * @param py The pixel y position
 * @param _ts_ The tile size
 * @param _wb_ The window band size
*/
FT_INLINE void tile_to_pixel_pos(iVec2 *pos, iVec2 p, s32 tile_size, WinBand wb) {
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
void 		write_text(SDLHandle *h, char *text, TTF_Font *font, iVec2 pos, u32 color);
void		draw_filled_circle(SDL_Renderer *renderer, int x, int y, int radius);
void		draw_circle_outline(SDL_Renderer *renderer, int x, int y, int radius);


/* src/text_display.c */
// void		left_band_center_text(iVec2 *char_pos, char *text, TTF_Font *font, s32 tile_size, s32 band_left_size);
// void		bot_band_center_text(iVec2 *char_pos, char *text, TTF_Font *font, s32 tile_size, s32 band_bot_size);
void		draw_letter_number(SDLHandle *handle, s8 player_color);
void		center_text_function_set(SDLHandle *h, CenterText *ct, BtnCenterText btn1, BtnCenterText btn2);
void		cancel_search_func(SDLHandle *h);
// center text
CenterText *center_text_init(SDLHandle *h);
void		center_text_string_set(SDLHandle *h, char *str, char *str2);
void		center_text_draw(SDLHandle *h, CenterText *ct);
void		center_text_destroy(CenterText *ct);

/* src/text_input.c */
TextField	init_text_field(SDL_Rect rect, int buff_size_max, TTF_Font *font, char *initial_text);
void		handle_text_input(SDLHandle *h, SDL_Event *event);
void		render_text_field(SDL_Renderer *renderer, TextField *text_field, SDL_Color text_color, SDL_Color bg_color);

/* src/chess_menu.c */
void	menu_close(ChessMenu *menu);
s8		init_menu(SDLHandle *h, s32 nb_btn);
void 	draw_menu(SDLHandle *h);
void	destroy_menu(SDLHandle *h);

/* src/button.c */
void	search_game(SDLHandle *h);
void	init_button(SDLHandle *h, ChessMenu *menu, s32 nb_btn);
void	center_btn_text(Button *btn, TTF_Font *font);
void	draw_button(SDLHandle *h, Button btn, SDL_Color c);
void	draw_multiple_button(SDLHandle *h, s32 btn_start, s32 nb_btn);
void	update_btn_disabled(SDLHandle *h, Button *btn);
s32		detect_button_click(Button *btn, s32 btn_start, s32 nb_btn, iVec2 mouse_pos);
void 	set_btn_info(SDLHandle *h, s32 btn_idx, iVec2 start, iVec2 size, char *text, ButtonFunc func);
// btn func
void	change_ip_click(SDLHandle *h);
#endif /* HANDLE_SDL_H */