#ifndef CHESS_LOG_H
#define CHESS_LOG_H

#include "../libft/libft.h"

#include "stdio.h"

/* Chess log level */
typedef enum e_log_level {
	LOG_NONE = 0U,
	LOG_ERROR = 1U,
	LOG_INFO = 2U,
	LOG_DEBUG = 3U,
} LogLevel;

/* Get the chess log level */
u8 *get_chess_log_level();

/* Set the chess log level */
void set_log_level(u8 level);

/* Log message */
#define LOG_MESSAGE(_color_, _level_, _func_, _line_, _format_str_, ...) do { \
	printf("%s%s: %s:%d: "RESET, _color_, _level_, _func_, _line_); \
	printf(_format_str_, ##__VA_ARGS__); \
} while (0) \

/* Chess Log macro */
#define CHESS_LOG(_msg_level_, _format_str_, ...) do { \
    u8 _curr_level_ = *get_chess_log_level(); \
    switch (_msg_level_) { \
        case LOG_ERROR: \
            if (_curr_level_ >= LOG_ERROR) \
                LOG_MESSAGE(RED, "Error", __func__, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        case LOG_INFO: \
            if (_curr_level_ >= LOG_INFO) \
                LOG_MESSAGE(GREEN, "Info", __func__, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        case LOG_DEBUG: \
            if (_curr_level_ >= LOG_DEBUG) \
                LOG_MESSAGE(YELLOW, "Debug", __func__, __LINE__, _format_str_, ##__VA_ARGS__); \
            break; \
        default: \
            break; \
    } \
} while (0) \

#endif

