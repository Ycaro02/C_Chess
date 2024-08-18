#include "../include/chess.h"

/*	@brief	Get possible moves for pawn
	*	@param	pawn			Bitboard of the selected pawn
	*	@param	occupied		Bitboard of the occupied squares
	*	@param	enemy			Bitboard of the enemy pieces
	*	@param	is_black		Flag to check if the pawn is black
	*	@param	only_attacks	Flag to check if only attacks are needed
	*	@return	Bitboard of the possible moves
*/
Bitboard get_pawn_moves(Bitboard pawn, Bitboard occupied, Bitboard enemy, s8 is_black, s8 only_attacks) {
    Bitboard one_step = 0, two_steps = 0, attacks_left = 0, attacks_right = 0;
	/* One step, if pawn is black, it moves up, otherwise it moves down */
    s8 direction = is_black ? 8 : -8;

    one_step = (is_black ? (pawn >> direction) : (pawn << -direction)) & ~occupied;
    
	/* Compute two steps if pawn is in starting position and first step is ok */
    if (one_step != 0) {
        two_steps = (is_black ? ((pawn & START_BLACK_PAWNS) >> 2*direction) : ((pawn & START_WHITE_PAWNS) << 2*-direction)) & ~occupied;
    }

	/* If only_attacks is set, return only the attacks/control tile*/
	if (only_attacks) {
		one_step = 0;
		two_steps = 0;
		enemy = UINT64_MAX;
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
static inline s8 handle_occupied_tile(Bitboard move, Bitboard occupied, Bitboard enemy, Bitboard *attacks) {
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

/* @brief	Get piece move function
 * @param	piece_move	Array of PieceMove struct
 * @param	piece_type	ChessPiece enum
 * @return	GetMoveFunc function pointer
*/
static GetMoveFunc get_piece_move_func(PieceMove *piece_move, ChessPiece piece_type) {
	for (int i = 0; i < PIECE_MOVE_ARRAY_SIZE; i++) {
		if (piece_move[i].white_piece_type == piece_type || piece_move[i].black_piece_type == piece_type) {
			return (piece_move[i].get_move_func);
		}
	}
	return (NULL);
}

/* @brief	Get piece move generic function
 * @param	board		ChessBoard struct
 * @param	piece		Bitboard of the selected piece
 * @param	piece_type	ChessPiece enum
 * @return	Bitboard of the possible moves
 */
Bitboard get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type) {
 	
	static PieceMove	piece_move[PIECE_MOVE_ARRAY_SIZE] = PIECE_MOVE_ARRAY;
	GetMoveFunc 		get_move = NULL;
	s8					is_black = (piece_type >= BLACK_PAWN);
	Bitboard			enemy = is_black ? board->white : board->black;
	
	/* If the piece is a pawn, get only the pawn moves */
	if (piece_type == WHITE_PAWN || piece_type == BLACK_PAWN) {
		return (get_pawn_moves(piece, board->occupied, enemy, is_black, FALSE));
	}
	
	/* Get the piece move function */
	get_move = get_piece_move_func(piece_move, piece_type);
	
	/* If not get move function found, empty tile is selected reset the possible move variable */
	if (!get_move) {
		return (0);
	}
	return (get_move(piece, board->occupied, enemy));
}

void move_piece(ChessBoard *board, ChessTile tile_from, ChessTile tile_to, ChessPiece type) {

	Bitboard mask_from = 1ULL << tile_from;
	Bitboard mask_to = 1ULL << tile_to;

	/* Remove the piece from the from tile */
	board->piece[type] &= ~mask_from;
	
	/* Add the piece to the to tile */
	board->piece[type] |= mask_to;
	
	/* Update the piece state */
	update_piece_state(board);
}