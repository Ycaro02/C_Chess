#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

s8 is_legal_promotion_packet(SDLHandle *handle, ChessPiece new_piece, ChessTile tile_to) {
	s8 enemy_color = handle->player_info.color == IS_WHITE ? IS_BLACK : IS_WHITE;
	ChessPiece start_piece = enemy_color == IS_WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
	ChessPiece end_piece = enemy_color == IS_WHITE ? WHITE_QUEEN : BLACK_QUEEN;

	if (new_piece < start_piece || new_piece > end_piece) {
		CHESS_LOG(LOG_INFO, "Piece type promotion is out of range %s\n", chess_piece_to_string(new_piece));
		return (FALSE);
	}

	/* Is if a promotion we need to check te pawn reach the raw of the promotion */
	if (enemy_color == IS_WHITE && tile_to < A8 && tile_to > H8) {
		CHESS_LOG(LOG_INFO, "Tile to is not in the promotion raw %s\n", TILE_TO_STRING(tile_to));
		return (FALSE);
	} else if (enemy_color == IS_BLACK && tile_to < A1 && tile_to > H1) {
		CHESS_LOG(LOG_INFO, "Tile to is not in the promotion raw %s\n", TILE_TO_STRING(tile_to));
		return (FALSE);
	}

	return (TRUE);
}

s8 is_legal_move_packet(SDLHandle *handle, ChessTile tile_from, ChessTile tile_to, ChessPiece piece_type) {
	Bitboard	possible_moves = 0;
	s8			enemy_color = handle->player_info.color == IS_WHITE ? IS_BLACK : IS_WHITE;
	ChessPiece	enemy_piece_start = enemy_color == IS_WHITE ? WHITE_PAWN : BLACK_PAWN;
	ChessPiece	enemy_piece_end = enemy_color == IS_WHITE ? WHITE_KING : BLACK_KING;

	/* Check if the tile is out of bound */
	if (tile_from < A1 || tile_from > H8 || tile_to < A1 || tile_to > H8) {
		CHESS_LOG(LOG_INFO, "Tile from or to is out of bound %s %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to));
		return (FALSE);
	}

	/* Check if the piece is out of range */
	if (piece_type < enemy_piece_start || piece_type > enemy_piece_end) {
		CHESS_LOG(LOG_INFO, "Piece type is out of range %s\n", chess_piece_to_string(piece_type));
		return (FALSE);
	}

	/* Check if the piece is on the tile */
	if ((handle->board->piece[piece_type] & (1ULL << tile_from)) == 0) {
		CHESS_LOG(LOG_INFO, "Piece is %s not on the tile %s\n", chess_piece_to_string(piece_type), TILE_TO_STRING(tile_from));
		return (FALSE);
	}

	/* Check if the move is possible */
	possible_moves = get_piece_move(handle->board, 1ULL << tile_from, piece_type, TRUE);
	if ((possible_moves & (1ULL << tile_to)) == 0) {
		CHESS_LOG(LOG_INFO, "Move is not possible from %s to %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to));
		return (FALSE);
	}
	return (TRUE);
}

/**
 * @brief Check if the packet is illegal
 * @param h The SDLHandle pointer
 * @param buffer The buffer
 * @param len The length of the buffer
 * @return s8 TRUE if the packet is illegal, FALSE otherwise
*/
s8 is_illegal_packet(SDLHandle *h, char *buffer, int len) {
	ChessTile	tile_from = INVALID_TILE, tile_to = INVALID_TILE;
	ChessPiece	piece_type = EMPTY, piece_check_legal = EMPTY;
	s8			msg_type = buffer[IDX_TYPE];
	s8			color = -1;
	
	(void)len;

	/* If the message is not a valid message type return here */
	if (msg_type < MSG_TYPE_COLOR || msg_type > MSG_TYPE_QUIT) {
		CHESS_LOG(LOG_INFO, RED"Buffer message type is not valid %s\n", RESET);
		return (TRUE);
	}

	if (msg_type != MSG_TYPE_QUIT && ft_memcmp(buffer, h->player_info.last_msg, MSG_SIZE) == 0) {
		return (TRUE);
	}

	/* If the message is COLOR and not the right size return here */
	if (msg_type == MSG_TYPE_COLOR) {
		color = buffer[IDX_FROM] - 1;
		if (color != IS_WHITE && color != IS_BLACK) {
			CHESS_LOG(LOG_INFO, RED"Buffer color is not WHITE or BLACK%s\n", RESET);
			return (TRUE);
		}
	}

	if (msg_type == MSG_TYPE_MOVE || msg_type == MSG_TYPE_PROMOTION) {
		
		tile_from = buffer[IDX_FROM] - 1;
		tile_to = buffer[IDX_TO] - 1;
		piece_type = buffer[IDX_PIECE] - 1;

		piece_check_legal = piece_type;

		/* If the message is PROMOTION, we need to check for pawn move instead of piece receive */
		if (msg_type == MSG_TYPE_PROMOTION) {
			piece_check_legal = h->player_info.color == IS_WHITE ? BLACK_PAWN : WHITE_PAWN;
		}

		/* If the message is MOVE or PROMOTION and the data for move are not valid return here */
		if (is_legal_move_packet(h, tile_from, tile_to, piece_check_legal) == FALSE) {
			return (TRUE);
		}

		/* If the message is PROMOTION and the data are not valid return here */
		if (msg_type == MSG_TYPE_PROMOTION && is_legal_promotion_packet(h, piece_type, tile_to) == FALSE) {
			return (TRUE);
		}
	}
	return (FALSE);
}


/**
 * @brief Ignore the message
 * @param h The SDLHandle pointer
 * @param buffer The buffer
 * @param last_msg_processed The last message processed
 * @return s8 TRUE if the message is ignored, FALSE otherwise
*/
s8 ignore_msg(SDLHandle *h, char *buffer) {
	int			len = fast_strlen(buffer);

	/* If the message is a hello,ack message or same than last msg process return here */
	if (fast_strcmp(buffer, HELLO_STR) == 0 || fast_strcmp(buffer, ACK_STR) == 0) {
		return (TRUE);
	}

	if (is_illegal_packet(h, buffer, len)) {
		return (TRUE);
	}
	return (FALSE);
}
