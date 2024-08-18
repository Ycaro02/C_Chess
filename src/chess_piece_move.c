#include "../include/chess.h"

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
	static const s8 direction[4] = {7, 9, -7, -9};
	/* Mask for out of bound */
    static const Bitboard oob_mask[4] = {
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
	static const s8 directions[4] = {8, -8, 1, -1}; /* all directions for rook moves */
	static const Bitboard all_mask[4] = {NOT_RANK_8, NOT_RANK_1, NOT_FILE_H, NOT_FILE_A};
	Bitboard attacks = 0, move = 0, mask = 0;
	s8 dir = 0;
	
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
	static const s8 directions[8] = {8, -8, 1, -1, 7, 9, -7, -9};
	static const Bitboard all_mask[8] = {
		NOT_RANK_8, NOT_RANK_1,  // 8 , -8
		NOT_FILE_H, NOT_FILE_A,  // 1, -1
		NOT_FILE_A & NOT_RANK_8, // 7 
		NOT_FILE_H & NOT_RANK_8, // 9
		NOT_FILE_A & NOT_RANK_1, // -7
		NOT_FILE_H & NOT_RANK_1, // -9
	};
	Bitboard attacks = 0, mask = 0, move = 0;
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
    static const s8 directions[8] = {6, 10, 15, 17, -6, -10, -15, -17};
    static const Bitboard all_mask[8] = {
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