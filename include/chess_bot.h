#ifndef CHESS_BOT_H
#define CHESS_BOT_H

// #include oppenssl for https send module
// #ifdef __ANDROID__
//     #include "/openssl-lib/include/openssl/ssl.h"
//     #include "/openssl-lib/include/openssl/err.h"
// #else
#include <openssl/ssl.h>
#include <openssl/err.h>
// #endif


#define STOCKFISH_URL "https://stockfish.online/api/s/v2.php?fen="

typedef enum {
	LEVEL_EASY,
	LEVEL_MEDIUM,
	LEVEL_HARD,
	LEVEL_EXPERT,
} BotLevel;

typedef struct s_bot_skill_level {
	BotLevel	level;
	char		*name;
	u8			depth_min;
	u8			depth_max;
} BotSkillLevel;

#define SKILL_LEVEL_ARRAY_SIZE 4

#define SKILL_LVL_ARRAY { \
	{LEVEL_EASY, "Easy", 1, 2}, \
	{LEVEL_MEDIUM, "Medium", 3, 4}, \
	{LEVEL_HARD, "Hard", 5, 7}, \
	{LEVEL_EXPERT, "Expert", 8, 12} \
}

typedef struct s_move_struct {
	ChessTile	                from;
	ChessTile	                to;
	ChessGenericPieceLetter		promotion_piece_letter;
} MoveStruct;


char        *send_http_request(char *url);
MoveStruct  send_stockfish_fen(char *fen_str);
s8          is_valid_move_struct(MoveStruct move);


#endif