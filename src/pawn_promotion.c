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
	if (is_black) {
		return (BLACK_PAWN + idx);
	}
	return (WHITE_KNIGHT + idx);
}

/* @brief Display the promotion selection
 * @param h The SDLHandle pointer
 * @param tile_from The tile from
 * @param tile_to The tile to
 * @return TRUE if the promotion is done, CHESS_QUIT if the player quit
 */
static s32 display_promotion_selection(SDLHandle *h, ChessTile tile_from, ChessTile tile_to) {
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

	/* Clear the window */
	h->board->possible_moves = 0;
	window_clear(h->renderer);
	draw_board(h, h->player_info.color);


	/* Draw a black rectangle */
	for (s32 i = 0; i < 4; i++) {
		draw_color_tile(h, start_pos, h->tile_size, RGBA_TO_UINT32(0, 100, 100, 255));
		start_pos.x++;
	}

	start_pos.x = 2;

	/* Draw the promotion pieces */
	s32 idx_texture_start = is_black ? BLACK_KNIGHT : WHITE_KNIGHT;
	for (s32 i = 0; i < 4; i++) {
		draw_texture_tile(h, h->piece_texture[idx_texture_start], start_pos, h->tile_size);
		idx_texture_start++;
		start_pos.x++;
	}
	SDL_RenderPresent(h->renderer);

	/* Wait for the player to select a piece */
	while (1) {
		s32 event = event_handler(h, h->player_info.color);
		ChessTile last_click = h->board->last_clicked_tile;
		if (last_click >= tile_start && last_click <= tile_end) {
			piece_idx = !is_black ? last_click - tile_start : tile_end - last_click;
			piece_selected = get_promot_selected_piece(piece_idx, is_black);
			promote_pawn(h->board, tile_to, piece_selected, is_black ? BLACK_PAWN : WHITE_PAWN);
			/* We can build the message here and return a special value to avoir double message create/sending */
			build_message(h->player_info.msg_tosend, MSG_TYPE_PROMOTION, tile_from, tile_to, piece_selected, h->board->turn);
			break ;
		} else if (event == CHESS_QUIT) {
			return (CHESS_QUIT);
		}
	}
	return (TRUE);
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
		ret = display_promotion_selection(handle, tile_from, tile_to);
	}
	return (ret);
}
