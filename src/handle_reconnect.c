#include "../include/network.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

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

	printf(RED"Process reconnect message %s\n"RESET, message_type_to_str(msg[IDX_TYPE]));
	printf(RED"Process reconnect message + 16 %s\n"RESET, message_type_to_str(msg[IDX_TYPE + 16]));

	/* Get size and time */
	ft_memcpy(&list_size, &msg[5], sizeof(u16));
	ft_memcpy(&array_byte_size, &msg[7], sizeof(u16));
	ft_memcpy(&my_remaining_time, &msg[9 + array_byte_size], sizeof(u64));
	ft_memcpy(&enemy_remaining_time, &msg[9 + array_byte_size + 8], sizeof(u64));

	/* Set the turn and player color */
	h->board->turn = msg[IDX_TURN];
	h->player_info.color = msg[IDX_FROM];
	/* 5 * 0 for white, and 5 * 1 for black */
	h->player_info.piece_start = BLACK_PAWN * h->player_info.color;
	/* 5 * 0 for white, and 5 * 1 for black, + 5 */
	h->player_info.piece_end = BLACK_KING * h->player_info.color + 5;

	/* Set the remaining time */
	h->my_remaining_time = my_remaining_time;
	h->enemy_remaining_time = enemy_remaining_time;

	/* Transform the array in list */
	move_arr = (MoveSave *)&msg[9];
	for (int i = 0; i < list_size; i++) {
		tile_from = move_arr[i].tile_from;
		tile_to = move_arr[i].tile_to;
		piece_from = move_arr[i].piece_from;
		piece_to = move_arr[i].piece_to;
		// printf(ORANGE"tile_from: %d, tile_to: %d, piece_from: %d, piece_to: %d\n", tile_from, tile_to, piece_from, piece_to);
		// printf(GREEN"Move from %s to %s\n", TILE_TO_STRING(tile_from), TILE_TO_STRING(tile_to));
		// printf(PURPLE"Piece from %s to %s\n"RESET, chess_piece_to_string(piece_from), chess_piece_to_string(piece_to));
		/* If the piece is the same */
		if (piece_from == piece_to) {
			// is_legal_move_packet
			move_piece(h, tile_from, tile_to, piece_from);
		} else {
			/* Is promotion move */
			// is_legal_move_packet // send pawn here
			// is_legal_promotion // send new piece here
			do_promotion_move(h, tile_from, tile_to, piece_to, FALSE);
		}
		last_piece_moved = piece_from;
	}

	/* Set the move list */
	h->board->lst = array_to_list(move_arr, list_size, sizeof(MoveSave));

	t_list *tmp = h->board->lst;
	while (tmp) {
		MoveSave *move = tmp->content;
		printf(ORANGE"After list cpy: tile_from: %d, tile_to: %d, piece_from: %d, piece_to: %d\n", move->tile_from, move->tile_to, move->piece_from, move->piece_to);
		tmp = tmp->next;
	}

	/* Detect player turn */
	detect_player_turn(h, last_piece_moved, h->player_info.color);
}

/* @brief Build the reconnect message
 * @param h The SDLHandle pointer
 * @param msg_size The size of the message
 * @return The message
 */
char *build_reconnect_message(SDLHandle *h, u16 *msg_size) {
	ChessMoveList	*move_list = h->board->lst;
	MoveSave		*move_arr = NULL;
	char			*buff = NULL;

	/* Get the size of the move list */
	u16 list_size = (u16)ft_lstsize(move_list);

	/* Get the size of the move list in byte */
	u16 array_byte_size = list_size * sizeof(MoveSave);


	/* Transform the move list in array */
	move_arr = list_to_array(move_list, list_size, sizeof(MoveSave));

	/*
		3 byte for msg_type, turn and color
		4 byte for the size of the message and the size of the list
		2 byte for the size of the list in byte
		size of array
		8 byte for enemy_remaining time
		8 byte for my_remaining time
	*/
	*msg_size = 3 + 4 + 2 + array_byte_size + 8 + 8;
	buff = ft_calloc(*msg_size, sizeof(char));
	if (!buff) {
		return (NULL);
	}
	buff[IDX_TYPE] = MSG_TYPE_RECONNECT;
	buff[IDX_TURN] = h->board->turn;
	buff[IDX_FROM] = !h->player_info.color;
	ft_memcpy(&buff[3], msg_size, sizeof(u16));
	ft_memcpy(&buff[5], &list_size, sizeof(u16));
	ft_memcpy(&buff[7], &array_byte_size, sizeof(u16));
	ft_memcpy(&buff[9], move_arr, array_byte_size);
	ft_memcpy(&buff[9 + array_byte_size], &h->enemy_remaining_time, sizeof(u64));
	ft_memcpy(&buff[9 + array_byte_size + 8], &h->my_remaining_time, sizeof(u64));
	free(move_arr);
	return (buff);
}