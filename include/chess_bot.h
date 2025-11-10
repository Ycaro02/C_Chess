#ifndef CHESS_BOT_H
#define CHESS_BOT_H

// #include oppenssl for https send module
#include <openssl/ssl.h>
#include <openssl/err.h>

int send_http_request(char *url);

void send_stockfish_fen(char *fen_str);

#endif