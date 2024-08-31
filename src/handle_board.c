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
static ChessTile handle_tile(ChessTile tile, s32 player_color) {
	if (player_color == IS_BLACK) {
		return (tile - 1);
	} 
	return (tile + 1);
}

/* Function to draw a filled circle */
static void draw_filled_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

/* Function to draw a circle outline */
static void draw_circle_outline(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            int distance_squared = dx * dx + dy * dy;
            if (distance_squared <= (radius * radius) && distance_squared >= ((radius - 5) * (radius - 5))) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

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

	draw_timer_rect(handle);

	/* Draw the letter and number for raw/column */
	draw_letter_number(handle, player_color);

	/* Draw the menu */
	// handle->menu.is_open = TRUE;
	if (handle->menu.is_open) {
		draw_menu(handle);
	}
	/* Draw the selected piece over the board */
	if (handle->over_piece_select != EMPTY) {
		draw_piece_over_board(handle, handle->mouse_pos.x - (handle->tile_size.x >> 1), handle->mouse_pos.y - (handle->tile_size.x >> 1));
	}

}

/**
 * @brief Detect click tile on the board
 * @param handle The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
ChessTile detect_tile_click(s32 x, s32 y, s32 tile_size, WinBand wb, s8 player_color) {
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s32 		column = 7;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			if (is_in_x_range(x, raw, tile_size, wb) && is_in_y_range(y, column, tile_size, wb)) {
				CHESS_LOG(LOG_INFO, "Click on "ORANGE"[%s]"RESET" -> "PINK"|%d|\n"RESET, TILE_TO_STRING(tile), tile);
				return (tile);
			}
			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
	}
	return (INVALID_TILE);
}

void reset_selected_tile(SDLHandle *h) {
	h->board->selected_tile = INVALID_TILE;
	h->board->selected_piece = EMPTY;
	h->board->possible_moves = 0;
	h->over_piece_select = EMPTY;
}


void update_mouse_pos(SDLHandle *h, s32 x, s32 y) {
	h->mouse_pos.x = x;
	h->mouse_pos.y = y;
}

void menu_event_handling(SDLHandle *h, SDL_Event event) {
	iVec2 pos = {0, 0};
	BtnType btn_click = BTN_INVALID;

	SDL_GetMouseState(&pos.x, &pos.y);
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		btn_click = detect_button_click(h->menu.btn, h->menu.nb_btn, pos);
		if (btn_click != BTN_INVALID) {
			h->menu.btn[btn_click].state = BTN_PRESSED;
			h->menu.current_btn_clicked = btn_click;
		}
	} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
		btn_click = detect_button_click(h->menu.btn, h->menu.nb_btn, pos);
		h->menu.btn[h->menu.current_btn_clicked].state = BTN_RELEASED;
		if (btn_click == h->menu.current_btn_clicked) {
			// need to call funct here
			if (h->menu.btn[btn_click].func) {
				h->menu.btn[btn_click].func(h);
			}
			h->menu.is_open = FALSE;
		}
	} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
		h->menu.is_open = FALSE;
	}
}

void game_event_handling(SDLHandle *h, SDL_Event event, s8 player_color) {
	ChessPiece piece_select = EMPTY;
	Bitboard aly_pos = h->over_piece_select >= BLACK_PAWN ? h->board->black : h->board->white;;
	s32 x = 0, y = 0;

	
	/* For local mode, aly is all occupied tile */
	if (!has_flag(h->flag, FLAG_NETWORK)) {
		aly_pos = h->board->occupied;
	}

	if (h->player_info.turn == FALSE) { return ; }
	SDL_GetMouseState(&x, &y);
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		h->board->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
		piece_select = get_piece_from_tile(h->board, h->board->last_clicked_tile);
		if (piece_select >= h->player_info.piece_start && piece_select <= h->player_info.piece_end) {
			h->over_piece_select = piece_select;
		}
	} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
		if (h->over_piece_select != EMPTY) {
			h->board->last_clicked_tile = detect_tile_click(x, y, h->tile_size.x, h->band_size, player_color);
			if (h->board->last_clicked_tile == h->board->selected_tile || ((1ULL << h->board->last_clicked_tile) & aly_pos) != 0) {
				h->over_piece_select = EMPTY;
			} 
		}
	} else if (event.type == SDL_MOUSEMOTION) {
		update_mouse_pos(h, x, y);
	} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
		h->menu.is_open = TRUE;
	}
}

/**
 * @brief Chess event handler
 * @return The tile clicked, or CHESS_QUIT if the user want to quit
 * @note Return the clicled tile or INVALID_TILE if no tile is clicked
*/
s32 event_handler(SDLHandle *h, s8 player_color) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT \
			|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
			return (CHESS_QUIT);
		}
		if (!h->menu.is_open) {
			game_event_handling(h, event, player_color);
		} else {
			menu_event_handling(h, event);
		}
		
	}
	return (TRUE);
}

/* @brief Verify if the king is check and mat or PAT
 * @param b			ChessBoard struct
 * @param is_black	Flag to check if the piece is black
 * @return TRUE if the game is end, FALSE otherwise
*/
s8 verify_check_and_mat(ChessBoard *b, s8 is_black) {

	Bitboard	enemy_pieces, piece, possible_moves;
	ChessPiece	enemy_piece_start = is_black ? BLACK_PAWN : WHITE_PAWN;
    ChessPiece	enemy_piece_end = is_black ? PIECE_MAX : BLACK_PAWN;
	char		*color = is_black ? "Black" : "White";
	s8 			check = FALSE, mat = TRUE;

	/* Check if the king is in check */
	if ((is_black && u8ValueGet(b->info, BLACK_CHECK)) || (!is_black && u8ValueGet(b->info, WHITE_CHECK))) {
		check = TRUE;
	}
	
	for (ChessPiece type = enemy_piece_start; type < enemy_piece_end; type++) {
		enemy_pieces = b->piece[type];
		while (enemy_pieces) {

			/* Get the first bit set */
			piece = enemy_pieces & -enemy_pieces;

			/* Clear the first bit set */
			enemy_pieces &= enemy_pieces - 1;

			/* Get the possible moves */
			possible_moves = get_piece_move(b, piece, type, TRUE);
			if (possible_moves != 0) {
				CHESS_LOG(LOG_DEBUG, "Piece %s on [%s] has possible moves\n", chess_piece_to_string(type), TILE_TO_STRING(piece));
				mat = FALSE;
				break ;
			}
		}
	}

	if (check && mat) {
		CHESS_LOG(LOG_ERROR, YELLOW"Checkmate detected for %s\n"RESET, color);
		return (TRUE);
	} else if (!check && mat) {
		CHESS_LOG(LOG_ERROR, PURPLE"PAT detected Egality for %s\n"RESET, color);
		return (TRUE);	
	}
	return (FALSE);
}


/* @brief Get the piece color control
 * @param b			ChessBoard struct
 * @param is_black	Flag to check if the piece is black
 * @return Bitboard of the controled tiles
 */
Bitboard get_piece_color_control(ChessBoard *b, s8 is_black) {
	Bitboard control = 0, possible_moves = 0, enemy_pieces = 0, piece = 0;
    ChessPiece enemy_piece_start = is_black ? BLACK_PAWN : WHITE_PAWN;
    ChessPiece enemy_piece_end = is_black ? PIECE_MAX : BLACK_PAWN;

    for (ChessPiece type = enemy_piece_start; type < enemy_piece_end; type++) {
        enemy_pieces = b->piece[type];

		/* For each enemy piece */
        while (enemy_pieces) {
			/* Get the first bit set */
            piece = enemy_pieces & -enemy_pieces;
            
			/* Clear the first bit set */
			enemy_pieces &= enemy_pieces - 1;

			/* Get the possible moves */
			possible_moves = get_piece_move(b, piece, type, FALSE);
			
			/* Add the possible moves to the control bitboard */
			control |= possible_moves;
        }
    }
	return (control);
}

/* @brief update_graphic_board, just call window_clear, draw_board and SDL_RenderPresent
 * @param h The SDLHandle pointer
*/
void update_graphic_board(SDLHandle *h) {
	window_clear(h->renderer);
	draw_board(h, h->player_info.color);
	SDL_RenderPresent(h->renderer);
}
