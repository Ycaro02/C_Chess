#include "../include/chess.h"
#include "../include/network.h"
#include "../include/handle_sdl.h"

/* @brief Promot the pawn
 * @param board The ChessBoard structure
 * @param tile The tile to promote
 * @param new_piece The new piece to promote
 * @param pawn_type The pawn type to remove
*/
static void promote_pawn(ChessBoard *board, ChessTile tile, ChessPiece new_piece, ChessPiece pawn_type) {
	Bitboard mask = 1ULL << tile;
	/* Remove the pawn */
	board->piece[pawn_type] &= ~mask;
	/* Add the new piece */
	board->piece[new_piece] |= mask;
	/* Update the piece state */
	update_piece_state(board);
}

/* @brief Get the selected piece
 * @param idx The index of the piece
 * @param is_black The player color
 * @return The selected piece
 */
static ChessPiece get_promot_selected_piece(s32 idx, s8 is_black) {
	ChessPiece selected = EMPTY;
	
	if (is_black) {
		selected = BLACK_PAWN + idx;
		if (selected > BLACK_QUEEN || selected < BLACK_KNIGHT) {
			selected = EMPTY;
		}
		return (selected);
	}
	selected = WHITE_KNIGHT + idx;
	if (selected > WHITE_QUEEN || selected < WHITE_KNIGHT) {
		selected = EMPTY;
	}
	return (selected);
}

void pawn_selection_event(SDLHandle *h) {
	// s32 event = event_handler(h, h->player_info.color);
	ChessTile tile_start = C7;
	ChessTile tile_end = G7;
	ChessPiece piece_selected = EMPTY;
	s32 piece_idx = 0;
	s8 is_black = (h->player_info.color == IS_BLACK);
	ChessTile last_click = h->board->last_clicked_tile;
	
	if (is_black) {
		tile_start = C2;
		tile_end = G2;
	}

	ChessTile tile_to = h->board->last_tile_to;

	if (last_click >= tile_start && last_click <= tile_end) {
		piece_idx = !is_black ? last_click - tile_start : tile_end - last_click;
		piece_selected = get_promot_selected_piece(piece_idx, is_black);
		if (piece_selected == EMPTY) { return ; }  
		promote_pawn(h->board, tile_to, piece_selected, is_black ? BLACK_PAWN : WHITE_PAWN);
		/* We can build the message here and return a special value to avoir double message create/sending */
		if (has_flag(h->flag, FLAG_NETWORK)) {
			build_message(h, h->player_info.msg_tosend, MSG_TYPE_PROMOTION, h->board->last_tile_from, tile_to, piece_selected);
			chess_msg_send(h->player_info.nt_info, h->player_info.msg_tosend, MSG_SIZE);
			h->player_info.turn = FALSE;
		}
		unset_flag(&h->flag, FLAG_PROMOTION_SELECTION);
	}
}

/* @brief Display the promotion selection
 * @param h The SDLHandle pointer
 * @param tile_from The tile from
 * @param tile_to The tile to
 * @return TRUE if the promotion is done, CHESS_QUIT if the player quit
 */
void display_promotion_selection(SDLHandle *h, ChessTile tile_from, ChessTile tile_to) {
	iVec2 start_pos = {2, 1}; // x, y
	ChessTile tile_start = C7;
	ChessTile tile_end = G7;
	ChessPiece piece_selected = EMPTY;
	s32 piece_idx = 0;
	s8 is_black = (h->player_info.color == IS_BLACK);

	if (is_black) {
		tile_start = C2;
		tile_end = G2;
	}

	/* Update board move (mandatory ? ) */
	h->board->possible_moves = 0;

	/* Draw a black rectangle */
	for (s32 i = 0; i < 4; i++) {
		draw_color_tile(h, start_pos, h->tile_size, RGBA_TO_UINT32(0, 100, 100, 255));
		start_pos.x++;
	}

	start_pos.x = 2;

	/* Draw the different promotion pieces */
	s32 idx_texture_start = is_black ? BLACK_KNIGHT : WHITE_KNIGHT;
	for (s32 i = 0; i < 4; i++) {
		draw_texture_tile(h, h->piece_texture[idx_texture_start], start_pos, h->tile_size);
		idx_texture_start++;
		start_pos.x++;
	}
}


/* @brief Do the promotion move
 * @param h The SDLHandle pointer
 * @param tile_from The tile from
 * @param tile_to The tile to
 * @param new_piece_type The new piece type
 * @param add_list Add the move to the list
 */
void do_promotion_move(SDLHandle *h, ChessTile tile_from, ChessTile tile_to, ChessPiece new_piece_type, s8 add_list) {
	/* If the message is a promotion message, promote the pawn */
	ChessPiece	opponent_pawn = new_piece_type >= BLACK_KNIGHT ? BLACK_PAWN : WHITE_PAWN;
	ChessPiece	piece_to_remove = get_piece_from_tile(h->board, tile_to);
	
	/* Remove the opponent pawn */
	h->board->piece[opponent_pawn] &= ~(1ULL << tile_from);

	/* Remove the piece if there is one on the tile */
	if (piece_to_remove != EMPTY) {
		h->board->piece[piece_to_remove] &= ~(1ULL << tile_to);
	}

	/* Add the new piece */
	h->board->piece[new_piece_type] |= (1ULL << tile_to);
	
	/* Update the last move */
	h->board->last_tile_from = tile_from;
	h->board->last_tile_to = tile_to;

	if (add_list) {
		move_save_add(&h->board->lst, tile_from, tile_to, opponent_pawn, new_piece_type);
	}

	update_piece_state(h->board);
}



/* @brief Check for pawn promotion
 * @param handle The SDLHandle pointer
 * @param type The piece type
 * @param tile_from The tile from
 * @param tile_to The tile to
 * @return TRUE if the pawn need to be promoted, FALSE otherwise, CHESS_QUIT if the player quit
*/
s32 check_pawn_promotion(SDLHandle *handle, ChessPiece type, ChessTile tile_from, ChessTile tile_to) {
	s32 ret = FALSE;
	s8 is_pawn = (type == WHITE_PAWN || type == BLACK_PAWN);
	s8 is_black = (type >= BLACK_PAWN);
	s8 is_white = !is_black;


	/* Check if is the player control pawn or opponent (no mandatory in network version) */
	if (handle->player_info.color == IS_WHITE && is_black) {
		return (ret);
	} else if (handle->player_info.color == IS_BLACK && is_white) {
		return (ret);
	}


	/* Check if the pawn need to be promoted */
	if ((is_pawn && is_white && tile_to >= A8 && tile_to <= H8)
		|| (is_pawn && is_black && tile_to >= A1 && tile_to <= H1)) {
		// ret = display_promotion_selection(handle, tile_from, tile_to);
		set_flag(&handle->flag, FLAG_PROMOTION_SELECTION);
		ret = TRUE;
	}
	return (ret);
}
