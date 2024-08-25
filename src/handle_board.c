#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/network.h"

/* @brief Is selected possible move
 * @param possible_moves	Bitboard of possible moves
 * @param tile				ChessTile enum
 * @return 1 if selected possible move, 0 otherwise
 */
s8 is_selected_possible_move(Bitboard possible_moves, ChessTile tile) {
    return ((possible_moves & (1ULL << tile)) != 0);
}

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
	/* Check if tile is current selected possible move */
	if (is_selected_possible_move(handle->board->possible_moves, tile)) {
		tile_piece = get_piece_from_tile(handle->board, tile);
		selected_piece = get_piece_from_tile(handle->board, handle->board->selected_tile);
		is_pawn = (selected_piece == WHITE_PAWN || selected_piece == BLACK_PAWN);
		is_king = (selected_piece == WHITE_KING || selected_piece == BLACK_KING);
		/* Get the center of the tile */
		center.x = tile_pos.x * TILE_SIZE + (TILE_SIZE >> 1);
		center.y = tile_pos.y * TILE_SIZE + (TILE_SIZE >> 1);
		/*	Check if tile is not empty (kill move) or en passant move (only for pawn) */
		if ((is_pawn && is_en_passant_move(handle->board, tile)) || tile_piece != EMPTY) {
			SDL_SetRenderDrawColor(handle->renderer, 200, 0, 0, 255); // Red color
			draw_circle_outline(handle->renderer, center.x, center.y, OUTLINE_CIRCLE_RADIUS);
		} else {
			/* Draw a small black circle in the center of the tile */
			if (is_king && INT_ABS_DIFF(handle->board->selected_tile, tile) == 2) {
				SDL_SetRenderDrawColor(handle->renderer, 0, 0, 200, 150); // Blue color castle move
				draw_circle_outline(handle->renderer, center.x, center.y, OUTLINE_CIRCLE_RADIUS);
			} 
			SDL_SetRenderDrawColor(handle->renderer, 0, 0, 0, 150); // Black color
			draw_filled_circle(handle->renderer, center.x, center.y, CIRCLE_RADIUS);
		}
	}
}

void promote_pawn(ChessBoard *board, ChessTile tile, ChessPiece new_piece, ChessPiece pawn_type) {
	Bitboard mask = 1ULL << tile;
	/* Remove the pawn */
	board->piece[pawn_type] &= ~mask;
	/* Add the new piece */
	board->piece[new_piece] |= mask;
	/* Update the piece state */
	update_piece_state(board);
}

ChessPiece get_selected_piece(s32 idx, s8 is_black) {
	// ft_printf_fd(1, "Selected piece idx: %d\n", idx);
	if (is_black) {
		return (BLACK_PAWN + idx);
	}
	return (WHITE_KNIGHT + idx);
}

s32 display_promotion_selection(SDLHandle *h, ChessTile tile_to) {
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
		draw_color_tile(h->renderer, start_pos, (iVec2){TILE_SIZE, TILE_SIZE}, RGBA_TO_UINT32(0, 100, 100, 255));
		start_pos.x++;
	}

	start_pos.x = 2;

	/* Draw the promotion pieces */
	s32 idx_texture_start = is_black ? BLACK_KNIGHT : WHITE_KNIGHT;
	for (s32 i = 0; i < 4; i++) {
		draw_texture_tile(h->renderer, h->piece_texture[idx_texture_start], start_pos, (iVec2){TILE_SIZE, TILE_SIZE});
		idx_texture_start++;
		start_pos.x++;
	}
	SDL_RenderPresent(h->renderer);

	/* Wait for the player to select a piece */
	while (1) {
		ChessTile tile_selected = event_handler(h, h->player_info.color);
		if (tile_selected >= tile_start && tile_selected <= tile_end) {
			piece_idx = !is_black ? tile_selected - tile_start : tile_end - tile_selected;
			piece_selected = get_selected_piece(piece_idx, is_black);
			// ft_printf_fd(1, "Tile selected: %d\n", tile_selected);
			promote_pawn(h->board, tile_to, piece_selected, is_black ? BLACK_PAWN : WHITE_PAWN);
			/* We can build the message here and return a special value to avoir double message create/sending */
			build_message(h->player_info.msg_tosend, MSG_TYPE_PROMOTION, h->player_info.color, tile_to, piece_selected);
			break ;
		} else if (tile_selected == CHESS_QUIT) {
			return (CHESS_QUIT);
		}
	}
	return (TRUE);
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

	draw_texure(h, texture, (iVec2){x, y}, (iVec2){TILE_SIZE, TILE_SIZE});
}


/* Draw chess board */
void draw_board(SDLHandle *handle, s8 player_color) {
	iVec2		tile_pos = {0, 0};
	u32			color = 0;
	s32 		column = 7;
	ChessPiece	pieceIdx = EMPTY;
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s8			piece_hover = FALSE;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			tile_pos = (iVec2){raw, column};
			
			/* Set color of tile */
			color = (column + raw) & 1 ? BLACK_TILE : WHITE_TILE;
			
			draw_color_tile(handle->renderer, tile_pos, (iVec2){TILE_SIZE, TILE_SIZE}, color);

			/* Draw possible move */
			draw_possible_move(handle, tile_pos, tile);

			/* Draw piece */
			pieceIdx = get_piece_from_tile(handle->board, tile);
			// piece_hover = (handle->over_piece_select == pieceIdx && (((1Ull << tile) & handle->board->piece[pieceIdx]) != 0)); ;
			piece_hover = (handle->over_piece_select == pieceIdx && tile == handle->board->selected_tile); ;
			if (pieceIdx != EMPTY && !piece_hover) {
				draw_texture_tile(handle->renderer, handle->piece_texture[pieceIdx], tile_pos, (iVec2){TILE_SIZE, TILE_SIZE});
			}

			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
	}

	if (handle->over_piece_select != EMPTY) {
		draw_piece_over_board(handle, handle->mouse_pos.x - (TILE_SIZE >> 1), handle->mouse_pos.y - (TILE_SIZE >> 1));
	}

}

static s8 is_in_x_range(s32 x, s32 raw) {
	return (x >= (raw * TILE_SIZE) + ((raw + 1) * TILE_SPACING)
			&& x <= ((raw + 1) * TILE_SIZE )+ ((raw + 1) * TILE_SPACING));
}

static s8 is_in_y_range(s32 y, s32 column) {
	return (y >= (column * TILE_SIZE) + ((column + 1) * TILE_SPACING) + TOP_BAND_HEIGHT
			&& y <= ((column + 1) * TILE_SIZE )+ ((column + 1) * TILE_SPACING) + TOP_BAND_HEIGHT);
}

/**
 * @brief Detect click tile on the board
 * @param handle The SDLHandle pointer
 * @param x The x position of the mouse
 * @param y The y position of the mouse
*/
ChessTile detect_tile_click(s32 x, s32 y, s8 player_color) {
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;
	s32 		column = 7;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			if (is_in_x_range(x, raw) && is_in_y_range(y, column)) {
				// ft_printf_fd(1, "Click on "ORANGE"[%s]"RESET" -> "PINK"|%d|\n"RESET, TILE_TO_STRING(tile), tile);
				return (tile);
			}
			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
	}
	return (tile);
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

/**
 * @brief Chess event handler
 * @return The tile clicked, or CHESS_QUIT if the user want to quit
 * @note Return the clicled tile or INVALID_TILE if no tile is clicked
*/
s32 event_handler(SDLHandle *h, s8 player_color) {
	SDL_Event event;
	// Bitboard aly_pos = player_color == IS_BLACK ? h->board->black : h->board->white;
	Bitboard aly_pos = 0;
	ChessTile tile = INVALID_TILE;
	ChessPiece piece_select = EMPTY;
	s32 x = 0, y = 0;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT \
			|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
			return (CHESS_QUIT);
		}
		if (h->player_info.turn == FALSE) {
			continue ;
		}
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			SDL_GetMouseState(&x, &y);
			tile = detect_tile_click(x, y, player_color);
			piece_select = get_piece_from_tile(h->board, tile);
			if (piece_select >= h->player_info.piece_start && piece_select <= h->player_info.piece_end) {
				h->over_piece_select = piece_select;
			}
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			SDL_GetMouseState(&x, &y);
			if (h->over_piece_select != EMPTY) {
				/* Here we need to fix for no network mode, aly is all occupied tile */
				aly_pos = h->over_piece_select >= BLACK_PAWN ? h->board->black : h->board->white;
				tile = detect_tile_click(x, y, player_color);
				if (tile == h->board->selected_tile || ((1ULL << tile) & aly_pos) != 0) {
					h->over_piece_select = EMPTY;
				} 
			}
		} else if (event.type == SDL_MOUSEMOTION) {
			SDL_GetMouseState(&x, &y);
			update_mouse_pos(h, x, y);
		}
	}
	return (tile);
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
				// ft_printf_fd(1, "Piece %s on [%s] has possible moves\n", chess_piece_to_string(type), TILE_TO_STRING(piece));
				mat = FALSE;
				break ;
			}
		}
	}

	if (check && mat) {
		ft_printf_fd(1, YELLOW"Checkmate detected for %s\n"RESET, color);
		return (TRUE);
	} else if (!check && mat) {
		ft_printf_fd(1, PURPLE"PAT detected Egality for %s\n"RESET, color);
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
