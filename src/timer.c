#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

void fill_timer_str(SDLHandle *h, u32 time) {
	s32 min = time / 60;
	s32 sec = time % 60;

	snprintf(h->timer_str, TIME_STR_SIZE, "%02d:%02d", min, sec);
}

void write_timer_in_rect(SDLHandle *h, SDL_Rect rect, u32 time) {
	u32 color = RGBA_TO_UINT32(0, 0, 0, 255);
	iVec2 pos = {0, 0};
	iVec2 text_size = {0, 0};

	fill_timer_str(h, time);

	TTF_SizeText(h->timer_font, h->timer_str, &text_size.x, &text_size.y);
	pos.x = rect.x + (rect.w >> 1);
	pos.x -= (text_size.x >> 1);

	pos.y = rect.y + (rect.h >> 1);
	pos.y -= (text_size.y >> 1);

	write_text(h, h->timer_str, h->timer_font, pos, color);

}

u64 get_time_sec() {
	return (SDL_GetTicks64() / 1000ULL);
}

void draw_timer_rect(SDLHandle *h) {
	static u64	prev_tick = 0;
	u64 		now = 0, elapsed_time = 0;
	u32 		*timer_to_update = NULL;
	s8 			decrement_time = has_flag(h->flag, FLAG_NETWORK) && h->player_info.nt_info && h->player_info.nt_info->peer_conected && has_flag(h->flag, FLAG_FIRST_MOVE_PLAYED);

	/* Draw timer rect */
	SDL_SetRenderDrawColor(h->renderer, 180, 180, 180, 255);
	SDL_RenderFillRect(h->renderer, &h->timer_rect_bot);
	SDL_RenderFillRect(h->renderer, &h->timer_rect_top);

	/* Draw name rect */
	SDL_RenderFillRect(h->renderer, &h->name_rect_bot);
	SDL_RenderFillRect(h->renderer, &h->name_rect_top);

	if (h->game_start) {
		now = get_time_sec();
		elapsed_time = now - prev_tick;
		SDL_SetRenderDrawColor(h->renderer, 0, 0, 150, 150);
		/* Get the timer to update */
		if (h->player_info.turn == TRUE) {
			SDL_RenderFillRect(h->renderer, &h->timer_rect_bot);
			SDL_RenderFillRect(h->renderer, &h->name_rect_bot);
			timer_to_update = &h->my_remaining_time;
		} else {
			SDL_RenderFillRect(h->renderer, &h->timer_rect_top);
			SDL_RenderFillRect(h->renderer, &h->name_rect_top);
			timer_to_update = &h->enemy_remaining_time;
		} 

		/* Update timer every second */
		if (decrement_time && elapsed_time >= 1) {
			if (*timer_to_update > 0) { (*timer_to_update)-- ; }
			prev_tick = now;
		}
	}
	/* Draw timer text */
	write_timer_in_rect(h, h->timer_rect_bot, h->my_remaining_time);
	write_timer_in_rect(h, h->timer_rect_top, h->enemy_remaining_time);

	/* Draw name text */
	write_text_in_rect(h, h->name_font, h->name_rect_bot, h->player_info.name, CENTER, U32_BLACK_COLOR);
	char *enemy_name = "Bot";

	if (has_flag(h->flag, FLAG_NETWORK) && h->player_info.nt_info && h->player_info.nt_info->peer_nickname[0] != '\0') {
		enemy_name = h->player_info.nt_info->peer_nickname;
	}
	write_text_in_rect(h, h->name_font, h->name_rect_top, enemy_name, CENTER, U32_BLACK_COLOR);
}