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
	iVec2 center = {0, 0};
	/* Check if tile is current selected possible move */
	if (is_selected_possible_move(handle->board->possible_moves, tile)) {
		center.x = tile_pos.x * TILE_SIZE + (TILE_SIZE >> 1);
		center.y = tile_pos.y * TILE_SIZE + (TILE_SIZE >> 1);
		/* Check if tile is not empty (kill move), if so, draw a red circle arround the piece */
		if (is_en_passant_move(handle->board, tile) || get_piece_from_tile(handle->board, tile) != EMPTY) {
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
