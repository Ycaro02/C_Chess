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

/* Draw chess board */
void draw_board(SDLHandle *handle, s8 player_color) {
	iVec2		tilePos = {0, 0};
	u32			color = 0;
	s32 		column = 7;
	ChessPiece	pieceIdx = EMPTY;
	ChessTile	tile = player_color == IS_BLACK ? H8 : A1;

	while (column >= 0) {
		for (s32 raw = 0; raw < 8; raw++) {
			tilePos = (iVec2){raw, column};
			
			/* Set color of tile */
			color = (column + raw) & 1 ? BLACK_TILE : WHITE_TILE;
			
			draw_color_tile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			
			/* Check if tile is current selected possible move */
			if (is_selected_possible_move(handle->board->possible_moves, tile)) {
				color = RGBA_TO_UINT32(0, 0, 200, 100);
				draw_color_tile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			}

			pieceIdx = get_piece_from_tile(handle->board, tile);
			if (pieceIdx != EMPTY) {
				draw_texture_tile(handle->renderer, handle->piece_texture[pieceIdx], tilePos, (iVec2){TILE_SIZE, TILE_SIZE});
			}
			tile = handle_tile(tile, player_color);
		}
		column--;
	}
}

static s8 is_in_x_range(s32 x, s32 raw) {
	return (x >= raw * TILE_SIZE + (raw + 1) * TILE_SPACING
			&& x <= (raw + 1) * TILE_SIZE + (raw + 1) * TILE_SPACING);
}

static s8 is_in_y_range(s32 y, s32 column) {
	return (y >= column * TILE_SIZE + (column + 1) * TILE_SPACING + TOP_BAND_HEIGHT
			&& y <= (column + 1) * TILE_SIZE + (column + 1) * TILE_SPACING + TOP_BAND_HEIGHT);
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
