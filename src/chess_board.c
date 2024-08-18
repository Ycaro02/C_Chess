#include "../include/chess.h"
#include "../include/handle_sdl.h"

/* Update occupied bitboard */
void update_occupied(ChessBoard *b) {
	b->occupied = 0;
	b->white = 0;
	b->black = 0;
	for (s32 i = 0; i < PIECE_MAX; i++) {
		b->occupied |= b->piece[i];
		if (i < BLACK_PAWN) {
			b->white |= b->piece[i];
		} else {
			b->black |= b->piece[i];
		}
	}
}

void init_board(ChessBoard *b) {
	/* Set start for white and black piece */
	b->piece[WHITE_PAWN] = START_WHITE_PAWNS;
	b->piece[WHITE_KNIGHT] = START_WHITE_KNIGHTS;
	b->piece[WHITE_BISHOP] = START_WHITE_BISHOPS;
	b->piece[WHITE_ROOK] = START_WHITE_ROOKS;
	b->piece[WHITE_QUEEN] = START_WHITE_QUEENS;
	b->piece[WHITE_KING] = START_WHITE_KING;

	b->piece[BLACK_PAWN] = START_BLACK_PAWNS;
	b->piece[BLACK_KNIGHT] = START_BLACK_KNIGHTS;
	b->piece[BLACK_BISHOP] = START_BLACK_BISHOPS;
	b->piece[BLACK_ROOK] = START_BLACK_ROOKS;
	b->piece[BLACK_QUEEN] = START_BLACK_QUEENS;
	b->piece[BLACK_KING] = START_BLACK_KING;

	// b->piece[WHITE_BISHOP] |= (1ULL << E4);
	// b->piece[WHITE_QUEEN] |= (1ULL << E5);
	// b->piece[WHITE_KING] |= (1ULL << E6);
	// b->piece[WHITE_KNIGHT] = START_WHITE_KNIGHTS | START_WHITE_PAWNS;
	// b->piece[BLACK_KNIGHT] = START_BLACK_KNIGHTS | START_BLACK_PAWNS;
	// b->piece[WHITE_KNIGHT] |= (1ULL << E5);

	update_occupied(b);
}

/* Get piece from tile */
ChessPiece get_piece(ChessBoard *b, ChessTile tile) {
	Bitboard mask = 1ULL << tile;
	ChessPiece piece = EMPTY;
	if (b->occupied & mask) {
		for (s32 i = 0; i < PIECE_MAX; i++) {
			if (b->piece[i] & mask) {
				piece = i;
				break;
			}
		}
	}
	return (piece);
}

s8 isPossibleMove(Bitboard possible_moves, ChessTile tile) {
    return ((possible_moves & (1ULL << tile)) != 0);
}


/* Draw chess board */
void draw_board(SDLHandle *handle) {
	iVec2 tilePos;
	u32 color;
	ChessTile tile = A1; // start from A1 for white player
	ChessPiece pieceIdx = EMPTY;
	for (s32 column = 0; column < 8; column++) {
		for (s32 raw = 0; raw < 8; raw++) {
			tilePos = (iVec2){raw, column};
			if (((column + raw) & 1)) {
				color = BLACK_TILE;
			} else {
				color = WHITE_TILE;
			}
			colorTile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			
			/* Check if tile is possible move */
			if (isPossibleMove(handle->board->possible_moves, tile)) {
				color = RGBA_TO_UINT32(0, 0, 200, 100);
				colorTile(handle->renderer, tilePos, (iVec2){TILE_SIZE, TILE_SIZE}, color);
			}

			pieceIdx = get_piece(handle->board, tile);
			if (pieceIdx != EMPTY) {
				drawTextureTile(handle->renderer, handle->piece_texture[pieceIdx], tilePos, (iVec2){TILE_SIZE, TILE_SIZE});
			}
			tile++;
		}
	}
}

/* Get possible moves for pawn */

/*	@brief	Get possible moves for pawn
	*	@param	pawn		Bitboard of the selected pawn
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@param	is_black	Flag to check if the pawn is black
	*	@return	Bitboard of the possible moves
*/
Bitboard get_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_black) {
    Bitboard one_step, two_steps, attacks_left, attacks_right;
	/* One step, if pawn is white, it moves up, if black, it moves down */
    s8 direction = is_black ? 8 : -8;

    one_step = (is_black ? (pawn >> direction) : (pawn << -direction)) & ~occupied;
    
    s8 one_step_free = one_step != 0;

	/* Compute two steps if pawn is in starting position */
    if (one_step_free) {
        two_steps = (is_black ? ((pawn & START_BLACK_PAWNS) >> 2*direction) : ((pawn & START_WHITE_PAWNS) << 2*-direction)) & ~occupied;
    } else {
        two_steps = 0;
    }

	/* Compute attacks left and right, and avoid out of bound */
    attacks_right = (is_black ? (pawn >> (direction - 1)) : (pawn << -(direction - 1))) & ~FILE_A & enemy;
    attacks_left = (is_black ? (pawn >> (direction + 1)) : (pawn << -(direction + 1))) & ~FILE_H & enemy;
    return (one_step | two_steps | attacks_left | attacks_right);
}


/*	@brief	Check if the move is blocked by an occupied square
	*		If the move captures an enemy piece, include it in the attacks
	*		Stop travel in this direction 
	*	@param	move		Bitboard of the move (piece position to check)
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@param	attacks		Pointer to the attacks bitboard
*/
static inline s8 handle_occupied_tile(Bitboard move, Bitboard occupied ,Bitboard enemy, Bitboard *attacks) {
	if (move & occupied) {
		if (move & enemy) {
			*attacks |= move;
		}
		return (TRUE);
	}
	return (FALSE);
}


/* @brief	Get possible moves for bishop
	*	@param	bishop		Bitboard of the selected bishop
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_bishop_moves(Bitboard bishop, Bitboard occupied, Bitboard enemy) {
    /* All directions for bishop moves */
	s8 direction[4] = {7, 9, -7, -9};
	/* Mask for out of bound */
    Bitboard oob_mask[4] = {
		NOT_FILE_A & NOT_RANK_8, // 7
		NOT_FILE_H & NOT_RANK_8, // 9
		NOT_FILE_A & NOT_RANK_1, // -7
		NOT_FILE_H & NOT_RANK_1, // -9
	};
    Bitboard attacks = 0, mask = 0, move = 0;
	s8 dir = 0;

    for (s8 i = 0; i < 4; i++) {
        dir = direction[i];
        mask = oob_mask[i];
        move = bishop;
        while (1) {
            /* Apply the mask to check for out of bounds before moving */
			if ((move & mask) == 0) { break ; }
            
			/* Shift the bishop in the current dir */
            move = (dir > 0) ? (move << dir) : (move >> -dir);

            /* If the move is zero, break the loop to avoid infinite loop */
            if (move == 0) { break ; }

			/* Check if the move is blocked by an occupied square */
			if (handle_occupied_tile(move, occupied, enemy, &attacks)) { break ; }
			
            /* Add the move to the attacks */
            attacks |= move;
        }
    }
    return attacks;
}

/*	@brief	Get possible moves for rook
	*	@param	rook		Bitboard of the selected rook
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_rook_moves(Bitboard rook, Bitboard occupied, Bitboard enemy) {
	Bitboard attacks = 0, move = 0, mask = 0;
	s8 directions[4] = {8, -8, 1, -1}; /* all directions for rook moves */
	Bitboard all_mask[4] = {NOT_RANK_8, NOT_RANK_1, NOT_FILE_H, NOT_FILE_A};
	s8 dir;
	
	for (s8 i = 0; i < 4; i++) {
		dir = directions[i];
		mask = all_mask[i];
		move = rook;
		while (1) {
			/* Apply the mask to check for out of bounds before moving */
			if ((move & mask) == 0) { break ; }

			/* Shift the rook in the current direction */
			move = (dir > 0) ? (move << dir) : (move >> -dir);

			/* If the move is zero, break the loop to avoid infinite loop */
			if (move == 0) { break ; }

			/* Check if the move is blocked by an occupied square */

			if (handle_occupied_tile(move, occupied, enemy, &attacks)) { break ; }
			
			/* Add the move to the attacks */
			attacks |= move;
		}
	}
	return (attacks);
}


/*	@brief	Get possible moves for queen
	*	@param	queen		Bitboard of the selected queen
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_queen_moves(Bitboard queen, Bitboard occupied, Bitboard enemy) {
	Bitboard attacks = 0;
	attacks |= get_bishop_moves(queen, occupied, enemy);
	attacks |= get_rook_moves(queen, occupied, enemy);
	return (attacks);
}


/*	@brief	Get possible moves for king
	*	@param	king		Bitboard of the selected king
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_king_moves(Bitboard king, Bitboard occupied, Bitboard enemy) {
	Bitboard attacks = 0;
	Bitboard mask = 0;
	s8 directions[8] = {8, -8, 1, -1, 7, 9, -7, -9};
	Bitboard all_mask[8] = {
		NOT_RANK_8, NOT_RANK_1,  // 8 , -8
		NOT_FILE_H, NOT_FILE_A,  // 1, -1
		NOT_FILE_A & NOT_RANK_8, // 7 
		NOT_FILE_H & NOT_RANK_8, // 9
		NOT_FILE_A & NOT_RANK_1, // -7
		NOT_FILE_H & NOT_RANK_1, // -9
	};
	Bitboard move = 0;
	s8 dir = 0;

	for (s8 i = 0; i < 8; i++) {
		dir = directions[i];
		mask = all_mask[i];
		move = king;
		if ((move & mask) == 0) { continue ; }
		move = (dir > 0) ? (move << dir) : (move >> -dir);
		if (move == 0) { continue ; }
		if (handle_occupied_tile(move, occupied, enemy, &attacks)) { continue ; }
		attacks |= move;
	}
	return (attacks);
}

/*	@brief	Get possible moves for knight
	*	@param	knight		Bitboard of the selected knight
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_knight_moves(Bitboard knight, Bitboard occupied, Bitboard enemy) {
    s8 directions[8] = {6, 10, 15, 17, -6, -10, -15, -17};
    Bitboard all_mask[8] = {
        NOT_FILE_A & NOT_FILE_B & NOT_RANK_8, // 6
        NOT_FILE_G & NOT_FILE_H & NOT_RANK_8, // 10
        NOT_FILE_A & NOT_RANK_7 & NOT_RANK_8, // 15
        NOT_FILE_H & NOT_RANK_7 & NOT_RANK_8, // 17
        NOT_FILE_G & NOT_FILE_H & NOT_RANK_1, // -6
        NOT_FILE_A & NOT_FILE_B & NOT_RANK_1, // -10
        NOT_FILE_H & NOT_RANK_1 & NOT_RANK_2, // -15
        NOT_FILE_A & NOT_RANK_1 & NOT_RANK_2  // -17
    };
    Bitboard attacks = 0, mask = 0, move = 0;
    s8 dir = 0;

    for (s8 i = 0; i < 8; i++) {
        dir = directions[i];
        mask = all_mask[i];
        move = knight;
        if ((move & mask) == 0) { continue ; }
        move = (dir > 0) ? (move << dir) : (move >> -dir);
        if (move == 0) { continue ; }
        if (handle_occupied_tile(move, occupied, enemy, &attacks)) { continue ; }
        attacks |= move;
    }
    return (attacks);
}

/* Display bitboard for debug */
void display_bitboard(Bitboard bitboard, const char *msg) {
	ft_printf_fd(1, "%s\n", msg);
	for (int i = 0; i < TILE_MAX; i++) {
		if (i % 8 == 0) {
			ft_printf_fd(1, "\n");
		}
		ft_printf_fd(1, "%d", (bitboard >> i) & 1);
	}
	ft_printf_fd(1, "\n");
}