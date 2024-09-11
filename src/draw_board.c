#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"
#include "../include/chess_log.h"

/* @brief Is selected possible move
 * @param possible_moves	Bitboard of possible moves
 * @param tile				ChessTile enum
 * @return 1 if selected possible move, 0 otherwise
 */
s8 is_selected_possible_move(Bitboard possible_moves, ChessTile tile) {
    return ((possible_moves & (1ULL << tile)) != 0);
}

/* @brief Is en passant move
 * @param b		ChessBoard structure
 * @param tile	ChessTile enum
 * @return 1 if en passant move, 0 otherwise
 */
s8 is_en_passant_move(ChessBoard *b, ChessTile tile) {
	return ((b->en_passant & (1ULL << tile)) != 0);
}

/**
 * @brief Handle tile incrementation/decrementation
 * @param tile The tile to handle
 * @param player_color The player color
 * @return The new tile value
 */
ChessTile handle_tile(ChessTile tile, s32 player_color) {
	if (player_color == IS_BLACK) {
		return (tile - 1);
	} 
	return (tile + 1);
}

/**
 * @brief Draw piece possible move
 * @param handle The SDLHandle pointer
 * @param tile_pos The tile position
 * @param tile The tile
 */
void draw_possible_move(SDLHandle *handle, iVec2 tile_pos, ChessTile tile) {
	ChessPiece	tile_piece = EMPTY, selected_piece = EMPTY;
	iVec2		center = {0, 0};
	s8			is_pawn = FALSE, is_king = FALSE;
	s32			tile_size = handle->tile_size.x;
	/* Check if tile is current selected possible move */
	if (is_selected_possible_move(handle->board->possible_moves, tile)) {
		tile_piece = get_piece_from_tile(handle->board, tile);
		selected_piece = get_piece_from_tile(handle->board, handle->board->selected_tile);
		is_pawn = (selected_piece == WHITE_PAWN || selected_piece == BLACK_PAWN);
		is_king = (selected_piece == WHITE_KING || selected_piece == BLACK_KING);
		/* Get the center of the tile */
		// TILE_POSITION_TO_PIXEL(tile_pos, center.x, center.y, tile_size, handle->band_size);
		tile_to_pixel_pos(&center, tile_pos, tile_size, handle->band_size);
		center.x += (tile_size >> 1);
		center.y += (tile_size >> 1);

		/*	Check if tile is not empty (kill move) or en passant move (only for pawn) */
		if ((is_pawn && is_en_passant_move(handle->board, tile)) || tile_piece != EMPTY) {
			SDL_SetRenderDrawColor(handle->renderer, 200, 0, 0, 255); // Red color
			draw_circle_outline(handle->renderer, center.x, center.y, OUTLINE_CIRCLE_RADIUS(tile_size));
		} else {
			if (is_king && INT_ABS_DIFF(handle->board->selected_tile, tile) == 2) {
				SDL_SetRenderDrawColor(handle->renderer, 0, 0, 200, 150); // Blue color castle move
				draw_circle_outline(handle->renderer, center.x, center.y, OUTLINE_CIRCLE_RADIUS(tile_size));
			} 
			/* Draw a small black circle in the center of the tile */
			SDL_SetRenderDrawColor(handle->renderer, 0, 0, 0, 150); // Black color
			draw_filled_circle(handle->renderer, center.x, center.y, CIRCLE_RADIUS(tile_size));
		}
	}
}

/**
 * @brief Smooth piece move, just move piece on the mouve position over the board
 * @param h The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
void draw_piece_over_board(SDLHandle *h, s32 x, s32 y) {
	ChessPiece	piece = h->over_piece_select;
	SDL_Texture	*texture = h->piece_texture[piece];

	draw_texure(h, texture, (iVec2){x, y}, h->tile_size);
}

void display_killed_piece(SDLHandle *h, ChessPiece piece, iVec2 pos, iVec2 size) {
	SDL_Texture	*texture = h->piece_texture[piece];
	draw_texure(h, texture, pos, size);
}

void piece_diff_to_str(char *str, s8 piece_diff) {
	fast_bzero(str, 8);
	int start = 0;


	if (piece_diff > 0) {
		str[0] = '+';
		start = 1;
	} 
	sprintf(str + start, "%d", piece_diff);
}

void draw_piece_kill(SDLHandle *h, s8 is_bot, s8 is_black) {
	iVec2 			start = {0, 0}, piece_size = {h->tile_size.x >> 2, h->tile_size.y >> 2};
	SDL_Rect		name_rect = is_bot ? h->name_rect_bot : h->name_rect_top;
	ChessPieceList	*lst_display = is_black ? h->board->black_kill_lst : h->board->white_kill_lst;

	start.x = name_rect.x;
	if (is_bot) {
		start.y = name_rect.y - (h->tile_size.y >> 1);
	} else {
		start.y = name_rect.y + name_rect.h + (h->tile_size.y >> 1) - (piece_size.y);
	}



	for (ChessPieceList *l = lst_display; l; l = l->next) {
		display_killed_piece(h, *(ChessPiece *)l->content, start, piece_size);
		start.x += piece_size.x;
		/* Check if we need to go to the next line */
		if (start.x + (piece_size.x + (piece_size.x >> 1)) >= h->window_size.x) {
			start.x = name_rect.x;
			if (is_bot) {
				start.y -= piece_size.y + (piece_size.y >> 1);
			} else {
				start.y += piece_size.y + (piece_size.y >> 1);
			}

		}
	}

	s8 my_val = h->board->white_piece_val, enemy_val = h->board->black_piece_val;

	if (h->player_info.color == IS_BLACK) {
		my_val = h->board->black_piece_val;
		enemy_val = h->board->white_piece_val;
	}

	if (is_bot && my_val != enemy_val) {
		// char *str = ft_itoa(h->board->white_piece_val - h->board->black_piece_val);
		char str[8] = {0};
		piece_diff_to_str(str, my_val - enemy_val);
		write_text(h, str, h->piece_diff_font, (iVec2){start.x + (piece_size.x >> 2), start.y}, U32_WHITE_COLOR);
	}

}


/* Draw chess board */
void draw_board(SDLHandle *handle, s8 player_color) {
	iVec2		tile_pos = {0, 0};
	u32			color = 0;
	s32 		column = 7;
	ChessPiece	pieceIdx = EMPTY;
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s8			piece_hiden = FALSE;
	s8 			black_check = u8ValueGet(handle->board->info, BLACK_CHECK);
	s8 			white_check = u8ValueGet(handle->board->info, WHITE_CHECK);
	s8			is_black = 0, is_king = 0;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			tile_pos = (iVec2){raw, column};
			
			/* Set color of tile */
			color = (column + raw) & 1 ? BLACK_TILE : WHITE_TILE;
			
			/* Draw tile */
			draw_color_tile(handle, tile_pos, handle->tile_size, color);

			if (tile == handle->board->last_tile_from || tile == handle->board->last_tile_to) {
				draw_color_tile(handle, tile_pos, handle->tile_size, RGBA_TO_UINT32(0, 120, 0, 150));
			}

			/* Draw possible move */
			draw_possible_move(handle, tile_pos, tile);

			/* Draw piece */
			pieceIdx = get_piece_from_tile(handle->board, tile);

			/* Check if the piece is the current selected piece */
			piece_hiden = (handle->over_piece_select == pieceIdx && tile == handle->board->selected_tile); ;
			
			if (pieceIdx != EMPTY && !piece_hiden) {
				is_black = (pieceIdx >= BLACK_PAWN);
				is_king = (pieceIdx == WHITE_KING || pieceIdx == BLACK_KING);
				if (is_king && ((is_black && black_check) || (!is_black && white_check))) {
					draw_color_tile(handle, tile_pos, handle->tile_size, RGBA_TO_UINT32(120, 0, 0, 255));
				}
				draw_texture_tile(handle, handle->piece_texture[pieceIdx], tile_pos, handle->tile_size);
			}

			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
	}

	if (has_flag(handle->flag, FLAG_PROMOTION_SELECTION)) {
		display_promotion_selection(handle);
	}

	draw_timer_rect(handle);

	/* Draw the letter and number for raw/column */
	draw_letter_number(handle, player_color);

	/* Draw the menu */
	if (handle->menu.is_open) {
		draw_menu(handle);
	}
	/* Draw the selected piece over the board */
	if (handle->over_piece_select != EMPTY) {
		draw_piece_over_board(handle, handle->mouse_pos.x - (handle->tile_size.x >> 1), handle->mouse_pos.y - (handle->tile_size.x >> 1));
	}

	if (has_flag(handle->flag, FLAG_EDIT_PROFILE)) {
		draw_profile_page(handle, handle->menu.profile);	
	}

	if (handle->center_text->str) {
		center_text_draw(handle, handle->center_text);
	}

	compute_piece_value(handle->board);

	draw_piece_kill(handle, TRUE, !handle->player_info.color);
	draw_piece_kill(handle, FALSE, handle->player_info.color);
}

/* @brief update_graphic_board, just call window_clear, draw_board and SDL_RenderPresent
 * @param h The SDLHandle pointer
*/
void update_graphic_board(SDLHandle *h) {
	window_clear(h->renderer);
	draw_board(h, h->player_info.color);
	SDL_RenderPresent(h->renderer);
}
