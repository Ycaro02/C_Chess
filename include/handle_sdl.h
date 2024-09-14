#ifndef HANDLE_SDL_H
#define HANDLE_SDL_H

#include "chess.h"

#ifdef __ANDROID__
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_video.h>
	#include <SDL2/SDL_render.h>
	#include <SDL2/SDL_ttf.h>
	#include <jni.h>
	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>
#else 
	/* Basic SDL include */
	#include "../rsc/lib/install/include/SDL2/SDL.h"
	#include "../rsc/lib/install/include/SDL2/SDL_video.h"
	#include "../rsc/lib/install/include/SDL2/SDL_render.h"
	/* TTF include */
	#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"
#endif

#include "chess_log.h"

// #define ESCAPE_KEY(_sym_) ((_sym_) == SDLK_ESCAPE || (_sym_) == SDLK_AC_BACK || (_sym_) == SDLK_AC_HOME)

FT_INLINE s8 ESCAPE_KEY(SDL_Keycode sym) {
	if (sym == SDLK_AC_BACK) {
		CHESS_LOG(LOG_INFO, "AC BACK KEY\n");
		return (TRUE);
	} else if (sym == SDLK_ESCAPE) {
		CHESS_LOG(LOG_INFO, "ESCAPE KEY\n");
		return (TRUE);
	}
	return (FALSE);
}

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
#define	FONT_PATH "rsc/font/arial.ttf"


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

/* Button function pointer */
typedef void (*ButtonFunc)(SDLHandle*);

typedef struct s_text_field TextField;

/* TextField function pointer */
typedef s8		(*AcceptedCharFunc)(SDL_Keycode key);
typedef void 	(*UpdateFunc)(SDLHandle*, TextField*);

#define IP_INPUT_SIZE 16

#define SERVER_INFO_STR "Server IP: "

struct s_text_field {
    SDL_Rect	rect;				/* The rect of the text input */
    char		*text;				/* The text buffer */
    TTF_Font	*font;				/* The font */
	AcceptedCharFunc	is_accepted_char;	/* The accepted char function ptr */
	UpdateFunc			update_data;		/* The update data function ptr */
	u64			last_cursor_time;	/* The last cursor time */
	int			buffer_size;		/* The buffer size */
    int			cursor;				/* The cursor position */
	s8			is_active;			/* The text input is active */
	s8			cursor_visible;		/* The cursor is visible */
};

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

typedef enum s_rect_text_pos {
	TOP_CENTER,
	BOT_CENTER,
	CENTER,
	TOP,
} RectTextPos;

typedef enum e_profile_field_type {
	PFT_INVALID=-1,
	PFT_NAME,
	PFT_TIMER,
} ProfileFieldType;

typedef struct s_profile {
	TextField			**tf;					/* The text field array pointer */
	char 				**describe_field;		/* The string array */
	Button				*btn;					/* The button array */
	TTF_Font			*font;					/* The font for field text display */
	TTF_Font			*describe_font;			/* The font for field description */
	SDL_Rect			rect;					/* The rect of the profile */
	ProfileFieldType	btn_hover;				/* The idx of button hover */
	ProfileFieldType	current_btn_clicked;	/* The idx of current button clicked */
	ProfileFieldType	field_active;			/* The idx of field selected (active) */
	s32					nb_field;				/* The number of field */
} Profile;

typedef struct s_chess_menu {
	/* Server info rect */
	SDL_Rect	server_info;			/* The server info rect */
	iVec2		server_info_str_pos;	/* The server info string position */
	TextField	*ip_field;				/* The server ip field */

	/* Profile */
	Profile			*profile;			/* The profile page */

	/* Menu rect data  */
	iVec2		start;				/* The start position */
	iVec2		end;				/* The end position */
	s32			width;				/* The width */
	s32			height;				/* The height */

	/* Button data */
	s32			total_btn;			/* The total number of button */
	s32			menu_btn;			/* The number of button in the menu */
	Button		*btn;				/* The button array */
	TTF_Font 	*btn_text_font;		/* The button text font */
	BtnType		current_btn_clicked;	/* The current button clicked */
	BtnType		btn_hover;				/* The button hover */

	/* Menu is open */
	s8			is_open;			/* The menu is open */
} ChessMenu;

typedef struct s_btn_center_tex {
	char		*str;	/* The button text */
	ButtonFunc	func;	/* The button function */
} BtnCenterText;

typedef struct s_center_text {
	SDL_Rect	rect;		/* The rect */
	TTF_Font	*font;		/* The font */
	char		*str;		/* The string */
	char		*str2;		/* The second string */
	s32			curent_btn_enable;	/* The current button enable 1 or 2 (btn are stored in menu struct)*/
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
	TTF_Font		*name_font;					/* The font */
	TTF_Font		*piece_diff_font;			/* The font */
	char			timer_str[TIME_STR_SIZE];	/* Timer string */
	iVec2			window_size;				/* The size of the window */
	iVec2			tile_size;					/* The size of the tile */
	iVec2			mouse_pos;					/* Mouse position */
	WinBand			band_size;					/* The band size */
	SDL_Rect		timer_rect_bot;				/* Timer rect */
	SDL_Rect		timer_rect_top;				/* Timer rect */
	SDL_Rect		name_rect_bot;				/* Name rect bot */
	SDL_Rect		name_rect_top;				/* Name rect top */
	ChessPiece		over_piece_select;			/* The piece selected (over display) */

	/* Player info */
	u64				my_remaining_time;		/* Current player ramaining time */
	u64				enemy_remaining_time;	/* Enemy player ramaining time */
	PlayerInfo		player_info;			/* Player info */
	u32				flag;					/* App Flag */
	u16				msg_id;					/* Over flag */
	s8				game_start;				/* Game start flag */
}	SDLHandle ;


/* Accepted char return if key pressed is uppercase */
#define UPPERCASE_CHAR 2

#define NICKNAME_MAX_LEN 8


/* @brief Inline func to convert tile position to pixel position
 * @param pos The pixel position to fill
 * @param p The tile position
 * @param tile_size The size of the tile
 * @param wb The window band struct
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

typedef enum e_btn_state {
	IS_SHIFT,
	IS_CAPS,
	IS_CTRL,
} KeyMod;

FT_INLINE void get_keyboard_mod(u8 *container) {
	SDL_Keymod mod = SDL_GetModState();
    
	*container = u8ValueSet(*container, IS_SHIFT, (mod & KMOD_SHIFT) != 0);
	*container = u8ValueSet(*container, IS_CAPS, (mod & KMOD_CAPS) != 0);
	*container = u8ValueSet(*container, IS_CTRL, (mod & KMOD_CTRL) != 0);
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
void		draw_letter_number(SDLHandle *handle, s8 player_color);
void		center_text_function_set(SDLHandle *h, CenterText *ct, BtnCenterText btn1, BtnCenterText btn2);
void		cancel_search_func(SDLHandle *h);
void		write_text_in_rect(SDLHandle *h, TTF_Font *font, SDL_Rect rect, char *str, RectTextPos align, u32 color);
// center text
CenterText *center_text_init(SDLHandle *h);
void		center_text_string_set(SDLHandle *h, char *str, char *str2);
void		center_text_draw(SDLHandle *h, CenterText *ct);
void		center_text_destroy(CenterText *ct);

/* src/text_input.c */
TextField	*init_text_field(SDL_Rect rect, TTF_Font *font, char *initial_text, s32 buff_size_max, AcceptedCharFunc accept_char_func, UpdateFunc update_func);
void		handle_text_input(SDLHandle *h, SDL_Event *event, TextField *tf);
void		render_text_field(SDL_Renderer *renderer, TextField *text_field, SDL_Color text_color, SDL_Color bg_color);
void		ip_server_update_data(SDLHandle *h, TextField *text_field);
s8			ip_server_accepted_char(SDL_Keycode type);
void		destroy_text_field(TextField *text_field);

/* src/chess_menu.c */
void		menu_close(ChessMenu *menu);
s8			init_menu(SDLHandle *h, s32 nb_btn);
void 		draw_menu(SDLHandle *h);
void		destroy_menu(SDLHandle *h);

/* src/button.c */
void		search_game(SDLHandle *h);
void		init_button(SDLHandle *h, ChessMenu *menu, s32 nb_btn);
void		center_btn_text(Button *btn, TTF_Font *font);
void		draw_button(SDLHandle *h,TTF_Font *font, Button btn, SDL_Color c);
void		update_btn_disabled(SDLHandle *h, Button *btn);
s32			detect_button_click(Button *btn, s32 btn_start, s32 nb_btn, iVec2 mouse_pos);
void 		set_btn_info(SDLHandle *h, s32 btn_idx, iVec2 start, iVec2 size, char *text, ButtonFunc func);
void		draw_multiple_button(SDLHandle *h, TTF_Font *font, Button *btn, s32 btn_hover, s32 btn_start, s32 nb_btn);

// btn func
void	edit_ip_click(SDLHandle *h);

/* src/profile.c */
Profile	*init_profile_page(SDLHandle *h, s32 nb_field);
void	destroy_profile_page(Profile *profile);
void	draw_profile_page(SDLHandle *h, Profile *profile);

/* sdl_handle_file.c */

SDL_RWops	*sdl_open(const char* file, const char* mode);
size_t		sdl_read(SDL_RWops* rw, void* buffer, size_t size, size_t maxnum);
size_t		sdl_write(SDL_RWops* rw, const void* buffer, size_t size, size_t num);
int			sdl_close(SDL_RWops* rw);
size_t		sdl_read_complete_file(SDL_RWops *rw, void *buffer, size_t total_size);
s64			get_file_size_sdl(const char *filename);
void		sdl_erase_file_data(const char* filename);

#endif /* HANDLE_SDL_H */
