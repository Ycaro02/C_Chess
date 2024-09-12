#ifndef CHESS_LOG_H
#define CHESS_LOG_H

#include "../libft/libft.h"

typedef enum {
	LOG_NONE,
	LOG_ERROR,
	LOG_INFO,
	LOG_DEBUG
} LogLevel;

/* Get the chess log level */
u8 *get_chess_log_level();

/* Set the chess log level */
void set_log_level(u8 level);

/* Log message */
#define LOG_MESSAGE(_color_, _level_, _func_, _line_, _format_str_, ...) do { \
	if (_func_ == NULL) {\
		printf("[%s%s]"RESET": ", _color_, _level_); \
	} else { \
		printf("[%s%s]"RESET": %s:%d: ", _color_, _level_, _func_, _line_); \
	} \
	printf(_format_str_, ##__VA_ARGS__); \
} while (0) \

/* Chess Log macro */
#define CHESS_LOG(_msg_level_, _format_str_, ...) do { \
    u8 _curr_level_ = *get_chess_log_level(); \
    switch (_msg_level_) { \
        case LOG_ERROR: \
            if (_curr_level_ >= LOG_ERROR) \
                LOG_MESSAGE(RED, "ERR", __func__, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        case LOG_INFO: \
            if (_curr_level_ >= LOG_INFO) \
                LOG_MESSAGE(GREEN, "INFO", (char *)NULL, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        case LOG_DEBUG: \
            if (_curr_level_ >= LOG_DEBUG) \
                LOG_MESSAGE(ORANGE, "DBG", __func__, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        default: \
            break; \
    } \
} while (0) \

#endif


