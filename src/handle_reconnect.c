#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

#define MOVE_ARRAY_IDX 10

#ifndef CHESS_SERVER
	static void detect_player_turn(SDLHandle *h, ChessPiece last_piece_move, s8 is_player_black) {
		s8 last_move_is_black = last_piece_move >= BLACK_PAWN;
		if (last_piece_move == EMPTY) {
			h->player_info.turn = !(is_player_black);
		} else {
			h->player_info.turn = !(last_move_is_black == is_player_black);
		}
	}

	/* @brief Process the reconnect message
	* @param h The SDLHandle pointer
	* @param msg The message
	*/
	void process_reconnect_message(SDLHandle *h, char *msg) {
		MoveSave	*move_arr = NULL;
		ChessTile	tile_from = INVALID_TILE, tile_to = INVALID_TILE;
		ChessPiece	piece_from = EMPTY, piece_to = EMPTY, last_piece_moved = EMPTY;
		u64			my_remaining_time = 0, enemy_remaining_time = 0;
		u16			list_size = 0, array_byte_size = 0;

		/* Get size and time */
		ft_memcpy(&list_size, &msg[6], sizeof(u16));
		ft_memcpy(&array_byte_size, &msg[8], sizeof(u16));
		ft_memcpy(&my_remaining_time, &msg[MOVE_ARRAY_IDX + array_byte_size], SIZEOF_TIMER);
		ft_memcpy(&enemy_remaining_time, &msg[MOVE_ARRAY_IDX + array_byte_size + TIMER_NB_BYTE], SIZEOF_TIMER);

		/* Set message ID and player color */
		h->msg_id = GET_MESSAGE_ID(msg);
		h->player_info.color = msg[IDX_FROM];

		/* 5 * 0 for white, and 5 * 1 for black */
		h->player_info.piece_start = BLACK_PAWN * h->player_info.color;

		/* 5 * 0 for white, and 5 * 1 for black, + 5 */
		h->player_info.piece_end = BLACK_KING * h->player_info.color + 5;

		/* Set the remaining time */
		h->my_remaining_time = my_remaining_time;
		h->enemy_remaining_time = enemy_remaining_time;

		/* Iter on move array to update board state */
		move_arr = (MoveSave *)&msg[MOVE_ARRAY_IDX];
		for (int i = 0; i < list_size; i++) {
			tile_from = move_arr[i].tile_from;
			tile_to = move_arr[i].tile_to;
			piece_from = move_arr[i].piece_from;
			piece_to = move_arr[i].piece_to;
			CHESS_LOG(LOG_INFO, PINK"Process move Tile: [%s] -> [%s], Piece [%s] -> [%s]\n"RESET,
				ChessTile_to_str(tile_from), ChessTile_to_str(tile_to),
				ChessPiece_to_str(piece_from), ChessPiece_to_str(piece_to));
			/* If the piece is the same */
			if (piece_from == piece_to) {
				// is_legal_move_packet
				move_piece(h, tile_from, tile_to, piece_from);
			} else {
				/* Is promotion move */
				// is_legal_move_packet // send pawn here
				// is_legal_promotion // send new piece here
				CHESS_LOG(LOG_INFO, ORANGE"Promotion move Tile: [%s] -> [%s], Piece [%s] -> [%s]\n"RESET,
					ChessTile_to_str(tile_from), ChessTile_to_str(tile_to),
					ChessPiece_to_str(piece_from), ChessPiece_to_str(piece_to));
				do_promotion_move(h, tile_from, tile_to, piece_to, FALSE);
			}
			last_piece_moved = piece_from;
		}

		/* Set the move list */
		h->board->lst = array_to_list(move_arr, list_size, sizeof(MoveSave));

		// CHESS_LOG(LOG_INFO, PURPLE"Timer Receive %ld %ld\n"RESET, my_remaining_time, enemy_remaining_time);


		/* Detect player turn */
		detect_player_turn(h, last_piece_moved, h->player_info.color);
	}
#endif

/* @brief Build the reconnect message
 * @param h The SDLHandle pointer
 * @param msg_size The size of the message
 * @return The message
 */
char *build_reconnect_message(ChessMoveList *move_lst, u16 *msg_size, u32 my_time, u32 enemy_time, u16 msg_id, s8 color) {
	ChessMoveList	*move_list = move_lst;
	MoveSave		*move_arr = NULL;
	char			*buff = NULL;

	/* Get the size of the move list */
	u16 list_size = (u16)ft_lstsize(move_list);

	/* Get the size of the move list in byte */
	u16 array_byte_size = list_size * sizeof(MoveSave);


	/* Transform the move list in array */
	move_arr = list_to_array(move_list, list_size, sizeof(MoveSave));

	/*
		2 byte for msg_type and color
		2 byte for the msg_id
		4 byte for the size of the message and the size of the list
		2 byte for the size of the list in byte
		size of array
		TIMER_NB_BYTE byte for enemy_remaining time
		TIMER_NB_BYTE byte for my_remaining time
	*/
	*msg_size = 2 + 2 + 4 + 2 + array_byte_size + TIMER_NB_BYTE + TIMER_NB_BYTE;
	buff = ft_calloc(*msg_size, sizeof(char));
	if (!buff) {
		return (NULL);
	}
	buff[IDX_TYPE] = MSG_TYPE_RECONNECT;

	/* Set the message ID */
	ft_memcpy(&buff[IDX_MSG_ID], &msg_id, sizeof(u16));

	CHESS_LOG(LOG_INFO, PURPLE"Build: %s ID: %d\n"RESET, MsgType_to_str(buff[IDX_TYPE]), GET_MESSAGE_ID(buff));


	/* Idx from is for the color on color/reconnect message */
	buff[IDX_FROM] = !color;

	ft_memcpy(&buff[4], msg_size, sizeof(u16));
	ft_memcpy(&buff[6], &list_size, sizeof(u16));
	ft_memcpy(&buff[8], &array_byte_size, sizeof(u16));
	ft_memcpy(&buff[MOVE_ARRAY_IDX], move_arr, array_byte_size);
	ft_memcpy(&buff[MOVE_ARRAY_IDX + array_byte_size], &enemy_time, SIZEOF_TIMER);
	ft_memcpy(&buff[MOVE_ARRAY_IDX + array_byte_size + TIMER_NB_BYTE], &my_time, SIZEOF_TIMER);

	free(move_arr);
	return (buff);
}

