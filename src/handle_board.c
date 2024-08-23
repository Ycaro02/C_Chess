#include "../include/chess.h"
#include "../include/handle_sdl.h"

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
	s8			is_pawn = FALSE;
	/* Check if tile is current selected possible move */
	if (is_selected_possible_move(handle->board->possible_moves, tile)) {
		tile_piece = get_piece_from_tile(handle->board, tile);
		selected_piece = get_piece_from_tile(handle->board, handle->board->selected_tile);
		is_pawn = (selected_piece == WHITE_PAWN || selected_piece == BLACK_PAWN);
		/* Get the center of the tile */
		center.x = tile_pos.x * TILE_SIZE + (TILE_SIZE >> 1);
		center.y = tile_pos.y * TILE_SIZE + (TILE_SIZE >> 1);
		/*	Check if tile is not empty (kill move) or en passant move (only for pawn) */
		if ((is_pawn && is_en_passant_move(handle->board, tile)) || tile_piece != EMPTY) {
			SDL_SetRenderDrawColor(handle->renderer, 200, 0, 0, 255); // Red color
			draw_circle_outline(handle->renderer, center.x, center.y, OUTLINE_CIRCLE_RADIUS);
		} else {
			/* Draw a small black circle in the center of the tile */
			SDL_SetRenderDrawColor(handle->renderer, 0, 0, 0, 150); // Black color
			draw_filled_circle(handle->renderer, center.x, center.y, CIRCLE_RADIUS);
		}
	}
}

/* Draw chess board */
void draw_board(SDLHandle *handle, s8 player_color) {
	iVec2		tile_pos = {0, 0};
	u32			color = 0;
	s32 		column = 7;
	ChessPiece	pieceIdx = EMPTY;
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;

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
			if (pieceIdx != EMPTY) {
				draw_texture_tile(handle->renderer, handle->piece_texture[pieceIdx], tile_pos, (iVec2){TILE_SIZE, TILE_SIZE});
			}

			/* Increment or decrement tile */
			tile = handle_tile(tile, player_color);
		}
		column--;
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

/**
 * @brief Chess event handler
 * @return The tile clicked, or CHESS_QUIT if the user want to quit
 * @note Return INVALID_TILE if no tile is clicked
*/
s32 event_handler(s8 player_color) {
	SDL_Event event;
	ChessTile tile = INVALID_TILE;
	s32 x = 0, y = 0;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT \
			|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
			return (CHESS_QUIT);
		}
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			SDL_GetMouseState(&x, &y);
			tile = detect_tile_click(x, y, player_color);
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
