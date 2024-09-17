#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

/**
 * @brief Call draw texture with piece at pos and size
*/
static void display_killed_piece(SDLHandle *h, ChessPiece piece, iVec2 pos, iVec2 size) {
	draw_texure(h, h->piece_texture[piece], pos, size);
}

/**
 * @brief Convert piece diff to string
*/
static void piece_diff_to_str(char *str, s8 piece_diff) {
	fast_bzero(str, 8);
	int start = 0;


	if (piece_diff > 0) {
		str[0] = '+';
		start = 1;
	} 
	sprintf(str + start, "%d", piece_diff);
}

/**
 * @brief Display piece diff value
*/
static void display_piece_diff_val(SDLHandle *h, iVec2 start, iVec2 piece_size, s8 is_bot) {
	s8 my_val = h->board->white_piece_val, enemy_val = h->board->black_piece_val;
	char str[8] = {0};

	if (h->player_info.color == IS_BLACK) {
		my_val = h->board->black_piece_val;
		enemy_val = h->board->white_piece_val;
	}

	if (is_bot && my_val != enemy_val) {
		// char *str = ft_itoa(h->board->white_piece_val - h->board->black_piece_val);
		piece_diff_to_str(str, my_val - enemy_val);
		write_text(h, str, h->piece_diff_font, (iVec2){start.x + (piece_size.x >> 2), start.y}, U32_WHITE_COLOR);
	}
}

void pc_draw_piece_kill(SDLHandle *h, s8 is_bot, s8 is_black) {
	iVec2 			start = {0, 0}, piece_size = {h->tile_size.x >> 2, h->tile_size.y >> 2};
	SDL_Rect		name_rect = is_bot ? h->name_rect_bot : h->name_rect_top;
	ChessPieceList	*lst_display = is_black ? h->board->black_kill_lst : h->board->white_kill_lst;

	start.x = name_rect.x;
	if (is_bot) {
		start.y = name_rect.y - (h->tile_size.y >> 1);
	} else {
		start.y = name_rect.y + name_rect.h + (h->tile_size.y >> 1) - (piece_size.y);
	}

	for (ChessPieceList *l = lst_display; l; l = l->next) {
		display_killed_piece(h, *(ChessPiece *)l->content, start, piece_size);
		start.x += piece_size.x;
		/* Check if we need to go to the next line */
		if (start.x + (piece_size.x + (piece_size.x >> 1)) >= h->window_size.x) {
			start.x = name_rect.x;
			if (is_bot) {
				start.y -= piece_size.y + (piece_size.y >> 1);
			} else {
				start.y += piece_size.y + (piece_size.y >> 1);
			}

		}
	}
	display_piece_diff_val(h, start, piece_size, is_bot);
}

void android_draw_piece_kill(SDLHandle *h, s8 is_bot, s8 is_black) {
	iVec2 			start = {0, 0}, piece_size = {h->tile_size.x >> 1, h->tile_size.y >> 1};
	iVec2			curr = {0, 0};
	SDL_Rect		name_rect = is_bot ? h->name_rect_bot : h->name_rect_top;
	ChessPieceList	*lst_display = is_black ? h->board->black_kill_lst : h->board->white_kill_lst;

	start.y = name_rect.y;
	if (is_bot) {
		start.x = name_rect.x - (piece_size.x << 1);
	} else { // is_top 
		start.x = name_rect.x + name_rect.w + (piece_size.x >> 1);
	}

	curr.x = start.x; curr.y = start.y;

	for (ChessPieceList *l = lst_display; l; l = l->next) {
		display_killed_piece(h, *(ChessPiece *)l->content, curr, piece_size);
		if (is_bot) {
			curr.x -= piece_size.x;
		} else {
			curr.x += piece_size.x;
		}

		/* Check if we need to go to the next line */
		if (!is_bot && curr.x + (piece_size.x + (piece_size.x >> 1)) >= h->window_size.x) {
			curr.x = start.x;
			// curr.y -= piece_size.y + (piece_size.y >> 1);
			curr.y += piece_size.y + (piece_size.y >> 1);
		} else if (is_bot && curr.x - (piece_size.x >> 1) <= 0) {
			curr.x = start.x;
			// curr.y += piece_size.y + (piece_size.y >> 1);
			curr.y -= piece_size.y + (piece_size.y >> 1);
		}
	}
	display_piece_diff_val(h, curr, piece_size, is_bot);
}