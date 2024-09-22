#include "../include/chess.h"

/*	@brief	Verify if the move is legal
	*	@param	b			ChessBoard struct
	*	@param	type		ChessPiece enum
	*	@param	from		Bitboard of the piece position
	*	@param	to			Bitboard of the destination position
	*	@param	is_black	Flag to check if the piece is black
	*	@return	TRUE if the move is legal, FALSE otherwise
*/
s8 verify_legal_move(ChessBoard *b, ChessPiece type, Bitboard from, Bitboard to, s8 is_black) {
	ChessPiece	enemy_piece = get_piece_from_mask(b, to);
	s8			legal = TRUE;
	
	/* Remove the enemy piece from the from tile is needed */
	if (enemy_piece != EMPTY) {
		b->piece[enemy_piece] &= ~to;
	}

	/* Move the piece */
	b->piece[type] &= ~from;
	b->piece[type] |= to;
	update_piece_state(b);

	/* Check if the king is in check */
	if (is_black && u8ValueGet(b->info, BLACK_CHECK)) {
		legal = FALSE;
	} else if (!is_black && u8ValueGet(b->info, WHITE_CHECK)) {
		legal = FALSE;
	}

	/* Reset the piece position */
	b->piece[type] &= ~to;
	b->piece[type] |= from;
	
	/* Restore enemy piece if needed */
	if (enemy_piece != EMPTY) {
		b->piece[enemy_piece] |= to;
	}
	update_piece_state(b);
	return (legal);
}

/*	@brief	Get the en passant attack
	*	@param	b			ChessBoard struct
	*	@param	current_type	ChessPiece enum
	*	@param	mask		Bitboard mask
	*	@return	Bitboard of the en passant attack
*/
Bitboard get_en_passant_atk(ChessBoard *b, ChessPiece current_type, Bitboard mask) {
	ChessPiece wanted_type = current_type == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
	ChessPiece enemy = EMPTY;

	/* Check if the en passant piece is opponent pawn */
	enemy = get_piece_from_tile(b, b->en_passant_tile);
	if (enemy != wanted_type) {
		return (0);
	}
	/* Check if the en passant attack is possible */
	return (b->en_passant & mask);
}

/*	@brief	Get possible moves for pawn
	*	@param	pawn			Bitboard of the selected pawn
	*	@param	occupied		Bitboard of the occupied squares
	*	@param	enemy			Bitboard of the enemy pieces
	*	@param	is_black		Flag to check if the pawn is black
	*	@param	only_attacks	Flag to check if only attacks are needed
	*	@return	Bitboard of the possible moves
*/
Bitboard get_pawn_moves(ChessBoard *b, Bitboard pawn, ChessPiece type, s8 is_black, s8 check_legal) {
    Bitboard	one_step = 0, two_steps = 0, attacks_left = 0, attacks_right = 0, atk_right_mask = 0, atk_left_mask = 0;
	Bitboard	occupied = b->occupied, enemy = is_black ? b->white : b->black;
	/* If pawn is black, it moves up, otherwise it moves down */
    s8 direction = is_black ? 8 : -8;

	/* Compute one step */
    one_step = (is_black ? (pawn >> direction) : (pawn << -direction)) & ~occupied;

	/* Compute two steps if pawn is in starting position and first step is ok */
    if (one_step != 0) {
        two_steps = (is_black ? ((pawn & START_BLACK_PAWNS) >> 2*direction) : ((pawn & START_WHITE_PAWNS) << 2*-direction)) & ~occupied;
		if (two_steps != 0 && check_legal && verify_legal_move(b, type, pawn, two_steps, is_black) == FALSE) { two_steps = 0 ; }
	}

	/* @note need the check_legal to not be infinite recurcise */
	if (one_step != 0 && check_legal && verify_legal_move(b, type, pawn, one_step, is_black) == FALSE) { one_step = 0 ; }

	/* If only_attacks is set, return only the attacks/control tile*/
	if (!check_legal) {
		one_step = 0; two_steps = 0;
		enemy = UINT64_MAX;
	}

	atk_right_mask = is_black ? (pawn >> (direction - 1)) : (pawn << -(direction - 1));
	atk_left_mask = is_black ? (pawn >> (direction + 1)) : (pawn << -(direction + 1));

	/* Compute attacks left and right, and avoid out of bound */
    attacks_right = atk_right_mask & ~FILE_A & enemy;
    attacks_left = atk_left_mask & ~FILE_H & enemy;
    
	/* Check if the attack en passant is possible */
	if (attacks_right == 0) { attacks_right = get_en_passant_atk(b, type, atk_right_mask) ; }
	if (attacks_left == 0) { attacks_left = get_en_passant_atk(b, type, atk_left_mask) ; }

	/* Check if the attacks are legal */
	if (check_legal) {
		if (verify_legal_move(b, type, pawn, attacks_right, is_black) == FALSE) { attacks_right = 0 ; }
		if (verify_legal_move(b, type, pawn, attacks_left, is_black) == FALSE) { attacks_left = 0 ; }
	}

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
static inline s8 handle_occupied_tile(Bitboard move, Bitboard occupied, Bitboard enemy) {
	if (move & occupied) {
		if (move & enemy) {
			return (ENEMY_TILE);
		}
		return (ALLY_TILE);
	}
	return (EMPTY_TILE);
}


/* @brief	Get possible moves for bishop
	*	@param	bishop		Bitboard of the selected bishop
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_bishop_moves(ChessBoard *b, Bitboard bishop, ChessPiece type, s8 is_black, s8 check_legal) {
    /* All directions for bishop moves */
	static const s8 direction[4] = {7, 9, -7, -9};
	/* Mask for out of bound */
    static const Bitboard oob_mask[4] = {
		NOT_FILE_A & NOT_RANK_8, // 7
		NOT_FILE_H & NOT_RANK_8, // 9
		NOT_FILE_H & NOT_RANK_1, // -7
		NOT_FILE_A & NOT_RANK_1, // -9
	};
    Bitboard attacks = 0, mask = 0, move = 0;
	Bitboard occupied = b->occupied;
	Bitboard enemy = is_black ? b->white : b->black;
	s8 dir = 0, occupied_tile = EMPTY_TILE;

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

			/* Check if the move is blocked by an occupied square, if is ally break now */
			occupied_tile = handle_occupied_tile(move, occupied, enemy);
			if (occupied_tile == ALLY_TILE) { break ; }

			/* Check if is a legal move */
			// if (check_legal && verify_legal_move(b, type, bishop, move, is_black) == FALSE) { continue ; }
			if (check_legal && verify_legal_move(b, type, bishop, move, is_black) == FALSE) { break ; }

            /* Add the move to the attacks */
            attacks |= move;

			/* If the move is blocked by an enemy piece, stop the travel in this direction */
			if (occupied_tile == ENEMY_TILE) { break ; }
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
Bitboard get_rook_moves(ChessBoard *b, Bitboard rook, ChessPiece type, s8 is_black, s8 check_legal) {
	static const s8 directions[4] = {8, -8, 1, -1}; /* all directions for rook moves */
	static const Bitboard all_mask[4] = {NOT_RANK_8, NOT_RANK_1, NOT_FILE_H, NOT_FILE_A};
	Bitboard attacks = 0, move = 0, mask = 0;
	Bitboard occupied = b->occupied;
	Bitboard enemy = is_black ? b->white : b->black;
	s8 dir = 0, occupied_tile = EMPTY_TILE;

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

			/* Check if the move is blocked by an occupied square, if is ally break now */
			occupied_tile = handle_occupied_tile(move, occupied, enemy);
			if (occupied_tile == ALLY_TILE) { break ; }

			/* Check if is a legal move */
			// if (check_legal && verify_legal_move(b, type, rook, move, is_black) == FALSE) { continue ; }
			if (check_legal && verify_legal_move(b, type, rook, move, is_black) == FALSE) { break ; }

			/* Add the move to the attacks */
			attacks |= move;

			/* If the move is blocked by an enemy piece, stop the travel in this direction */
			if (occupied_tile == ENEMY_TILE) { break ; }

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
Bitboard get_queen_moves(ChessBoard *b, Bitboard queen, ChessPiece type, s8 is_black, s8 check_legal) {
	Bitboard attacks = 0;

	attacks |= get_bishop_moves(b, queen, type, is_black, check_legal);
	attacks |= get_rook_moves(b, queen, type, is_black, check_legal);
	return (attacks);
}


static inline s8 is_safe_path(Bitboard enemy_control, Bitboard path) {
	return ((enemy_control & path) == 0);
}

static inline s8 is_empty_path(Bitboard occupied, Bitboard path) {
	return ((occupied & path) == 0);
}

static Bitboard verify_castle_move(ChessBoard *b, Bitboard king, s8 is_black){
	Bitboard path = 0, move = 0;
	Bitboard enemy_control = is_black ? b->white_control : b->black_control;

	/* Check if the king has ever moved */
	if (u8ValueGet(b->info, is_black ? BLACK_KING_MOVED : WHITE_KING_MOVED)) {
		return (0);
	}

	/* Check if the king is in check */
	if (is_black && u8ValueGet(b->info, BLACK_CHECK)) {
		return (0);
	} else if (!is_black && u8ValueGet(b->info, WHITE_CHECK)) {
		return (0);
	}


	/* Check if the king rook is on the board */
	ChessPiece	wanted_rook = is_black ? BLACK_ROOK : WHITE_ROOK;
	ChessTile	rook_tile = is_black ? BLACK_KING_ROOK_START_POS : WHITE_KING_ROOK_START_POS;
	ChessPiece	rook = get_piece_from_tile(b, rook_tile);
	if (rook == wanted_rook) {
		/* Check if the king rook has ever moved */
		if (!u8ValueGet(b->info, is_black ? BLACK_KING_ROOK_MOVED : WHITE_KING_ROOK_MOVED)) {
			path = is_black ? BLACK_KING_CASTLE_PATH : WHITE_KING_CASTLE_PATH;
			if (is_empty_path(b->occupied, path) && is_safe_path(enemy_control, path)) {
				move |= (king << 2);
			}
		}
	}

	/* Check if the queen rook is on the board */
	wanted_rook = is_black ? BLACK_ROOK : WHITE_ROOK;
	rook_tile = is_black ? BLACK_QUEEN_ROOK_START_POS : WHITE_QUEEN_ROOK_START_POS;
	rook = get_piece_from_tile(b, rook_tile);
	if (rook != wanted_rook) {
		return (move);
	}

	/* Check if the queen rook has ever moved */
	if (!u8ValueGet(b->info, is_black ? BLACK_QUEEN_ROOK_MOVED : WHITE_QUEEN_ROOK_MOVED)) {
		path = is_black ? BLACK_QUEEN_CASTLE_PATH : WHITE_QUEEN_CASTLE_PATH;
		if (is_empty_path(b->occupied, path) && is_safe_path(enemy_control, path)) {
			move |= (king >> 2);
		}
	}
	return (move);
}

/*	@brief	Get possible moves for king
	*	@param	king		Bitboard of the selected king
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_king_moves(ChessBoard *b, Bitboard king, ChessPiece type, s8 is_black, s8 check_legal) {
	static const s8 directions[8] = {8, -8, 1, -1, 7, 9, -7, -9};
	static const Bitboard all_mask[8] = {
		NOT_RANK_8, NOT_RANK_1,  // 8 , -8
		NOT_FILE_H, NOT_FILE_A,  // 1, -1
		NOT_FILE_A & NOT_RANK_8, // 7 
		NOT_FILE_H & NOT_RANK_8, // 9
		NOT_FILE_H & NOT_RANK_1, // -7
		NOT_FILE_A & NOT_RANK_1, // -9
	};
	Bitboard attacks = 0, mask = 0, move = 0;
	Bitboard occupied = b->occupied;
	Bitboard enemy = is_black ? b->white : b->black;
	s8 dir = 0;

	for (s8 i = 0; i < 8; i++) {
		dir = directions[i];
		mask = all_mask[i];
		move = king;

		/* Apply the mask to check for out of bounds before moving */
		if ((move & mask) == 0) { continue ; }

		/* Shift the king in the current direction */
		move = (dir > 0) ? (move << dir) : (move >> -dir);

		/* If the move is zero, continue to the next direction */
		if (move == 0) { continue ; }

		/* Check if is a legal move */
		if (check_legal && verify_legal_move(b, type, king, move, is_black) == FALSE) { continue ; }

		/* Check if the move is blocked by an occupied square */
		if (handle_occupied_tile(move, occupied, enemy) == ALLY_TILE) { continue ; }

		/* Add the move to the attacks */
		attacks |= move;
	}

	/* Get castle move */
	if (check_legal) {
		attacks |= verify_castle_move(b, king, is_black);
	}

	return (attacks);
}

/*	@brief	Get possible moves for knight
	*	@param	knight		Bitboard of the selected knight
	*	@param	occupied	Bitboard of the occupied squares
	*	@param	enemy		Bitboard of the enemy pieces
	*	@return	Bitboard of the possible moves
*/
Bitboard get_knight_moves(ChessBoard *b, Bitboard knight, ChessPiece type, s8 is_black, s8 check_legal) {
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
	Bitboard occupied = b->occupied;
	Bitboard enemy = is_black ? b->white : b->black;
    s8 dir = 0;

    for (s8 i = 0; i < 8; i++) {
        dir = directions[i];
        mask = all_mask[i];
        move = knight;

		/* Apply the mask to check for out of bounds before moving */
        if ((move & mask) == 0) { continue ; }

		/* Shift the knight in the current direction */
        move = (dir > 0) ? (move << dir) : (move >> -dir);

		/* If the move is zero, continue to the next direction */
        if (move == 0) { continue ; }

		/* Check if is a legal move */
		if (check_legal && verify_legal_move(b, type, knight, move, is_black) == FALSE) { continue ; }
        
		/* Check if the move is blocked by an occupied square */
		if (handle_occupied_tile(move, occupied, enemy) == ALLY_TILE) { continue ; }

		/* Add the move to the attacks */
		attacks |= move;
    }
    return (attacks);
}

