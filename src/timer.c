#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

void fill_timer_str(SDLHandle *h, struct timeval *time) {
	s32 min = time->tv_sec / 60;
	s32 sec = time->tv_sec % 60;

	snprintf(h->timer_str, TIME_STR_SIZE, "%02d:%02d", min, sec);
}

void wite_timer_in_rect(SDLHandle *h, SDL_Rect rect, struct timeval *time) {
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

void decrement_timer(struct timeval *time) {
    if (time->tv_sec > 0) {
		time->tv_sec--;
		time->tv_usec = 0;
    }
}

void draw_timer_rect(SDLHandle *h) {
	static struct timeval	prev_time = {0, 0};
	s8 						is_black = h->player_info.color == IS_BLACK;
    struct timeval			now = {0, 0};
	struct timeval			*timer_to_update = NULL;
	struct timeval			*bot_timer = is_black ? &h->black_time : &h->white_time;
	struct timeval			*top_timer = is_black ? &h->white_time : &h->black_time;

	/* Update player time */
	if (h->player_info.turn == TRUE) {
		timer_to_update = is_black ? &h->black_time : &h->white_time;
	} else {
		timer_to_update = is_black ? &h->white_time : &h->black_time;
	} 


    gettimeofday(&now, NULL);

	/* Elapsed time in microsecond */
    u64 elapsed_time = (now.tv_sec - prev_time.tv_sec) * 1000000L + (now.tv_usec - prev_time.tv_usec);

	/* Update timer every second */
    if (elapsed_time >= 1000000L) {
        decrement_timer(timer_to_update);
        prev_time = now;
    }

	/* Draw timer rect */
	SDL_SetRenderDrawColor(h->renderer, 180, 180, 180, 255);
	SDL_RenderFillRect(h->renderer, &h->timer_rect_bot);
	SDL_RenderFillRect(h->renderer, &h->timer_rect_top);
	wite_timer_in_rect(h, h->timer_rect_bot, bot_timer);
	wite_timer_in_rect(h, h->timer_rect_top, top_timer);
}

#ifdef CHESS_WINDOWS_VERSION
	int gettimeofday(struct timeval* tp, struct timezone* tzp) {
		/* Retrieve the current system time as a FILETIME structure */
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		(void)tzp;
		/* Convert FILETIME to a 64-bit integer */
		uint64_t time = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

		/* FILETIME is in 100-nanosecond units since January 1, 1601 */
		/* Convert to microseconds and adjust to the Unix epoch (January 1, 1970) */
		time -= 116444736000000000ULL; /* Convert to the number of 100ns intervals since 1970 */
		time /= 10; /* Convert to microseconds */

		/* Fill the timeval structure */
		tp->tv_sec = (long)(time / 1000000); /* Seconds part */
		tp->tv_usec = (long)(time % 1000000); /* Microseconds part */

		return 0;
	}
#endif