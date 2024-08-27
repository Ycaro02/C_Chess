#include "../include/chess_log.h"

u8 *get_chess_log_level() {
	static u8 level = 0;
	return (&level);
}

void set_log_level(u8 level) {
	*get_chess_log_level() = level;
}
