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

/*	@brief	Get possible moves for pawn
	*	@param	pawn			Bitboard of the selected pawn
	*	@param	occupied		Bitboard of the occupied squares
	*	@param	enemy			Bitboard of the enemy pieces
	*	@param	is_black		Flag to check if the pawn is black
	*	@param	only_attacks	Flag to check if only attacks are needed
	*	@return	Bitboard of the possible moves
*/
Bitboard get_pawn_moves(ChessBoard *b, Bitboard pawn, ChessPiece type, s8 is_black, s8 check_legal) {
    Bitboard	one_step = 0, two_steps = 0, attacks_left = 0, attacks_right = 0;
	Bitboard	occupied = b->occupied;
	Bitboard	enemy = is_black ? b->white : b->black;

	/* One step, if pawn is black, it moves up, otherwise it moves down */
    s8 direction = is_black ? 8 : -8;

    one_step = (is_black ? (pawn >> direction) : (pawn << -direction)) & ~occupied;

	/* Compute two steps if pawn is in starting position and first step is ok */
    if (one_step != 0) {
        two_steps = (is_black ? ((pawn & START_BLACK_PAWNS) >> 2*direction) : ((pawn & START_WHITE_PAWNS) << 2*-direction)) & ~occupied;
		if (two_steps != 0 && check_legal && verify_legal_move(b, type, pawn, two_steps, is_black) == FALSE) { two_steps = 0 ; }
	}

	/* @note need the !check_legal to not be infinite recurcise */
	if (one_step != 0 && check_legal && verify_legal_move(b, type, pawn, one_step, is_black) == FALSE) { one_step = 0 ; }

	/* If only_attacks is set, return only the attacks/control tile*/
	if (!check_legal) {
		one_step = 0; two_steps = 0;
		enemy = UINT64_MAX;
	}

	/* Compute attacks left and right, and avoid out of bound */
    attacks_right = (is_black ? (pawn >> (direction - 1)) : (pawn << -(direction - 1))) & ~FILE_A & enemy;
    attacks_left = (is_black ? (pawn >> (direction + 1)) : (pawn << -(direction + 1))) & ~FILE_H & enemy;
    
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
	s8 dir = 0;
	s8 legal_move = TRUE;

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

			/* Check if is a legal move */
			if (check_legal && verify_legal_move(b, type, bishop, move, is_black) == FALSE) {
				legal_move = FALSE;
			}

			/* Check if the move is blocked by an occupied square */
			if (handle_occupied_tile(move, occupied, enemy, &attacks)) { break ; }
			
			if (!legal_move) {continue;}

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
Bitboard get_rook_moves(ChessBoard *b, Bitboard rook, ChessPiece type, s8 is_black, s8 check_legal) {
	static const s8 directions[4] = {8, -8, 1, -1}; /* all directions for rook moves */
	static const Bitboard all_mask[4] = {NOT_RANK_8, NOT_RANK_1, NOT_FILE_H, NOT_FILE_A};
	Bitboard attacks = 0, move = 0, mask = 0;
	Bitboard occupied = b->occupied;
	Bitboard enemy = is_black ? b->white : b->black;
	s8 dir = 0;
	s8 legal_move = TRUE;

	// to implement rook move save for king
	
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

			/* Check if is a legal move */
			if (check_legal && verify_legal_move(b, type, rook, move, is_black) == FALSE) {
				legal_move = FALSE;
			}

			/* Check if the move is blocked by an occupied square */
			if (handle_occupied_tile(move, occupied, enemy, &attacks)) { break ; }
			
			if (!legal_move) {continue;}
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


	/* Check if the king rook has ever moved */
	if (!u8ValueGet(b->info, is_black ? BLACK_KING_ROOK_MOVED : WHITE_KING_ROOK_MOVED)) {
		path = is_black ? BLACK_KING_CASTLE_PATH : WHITE_KING_CASTLE_PATH;
		if (is_empty_path(b->occupied, path) && is_safe_path(enemy_control, path)) {
			move |= (king << 2);
		}
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
		if (check_legal && verify_legal_move(b, type, king, move, is_black) == FALSE) {
				continue ;
		}

		/* Check if the move is blocked by an occupied square */
		if (handle_occupied_tile(move, occupied, enemy, &attacks)) { continue ; }
		
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
		if (check_legal && verify_legal_move(b, type, knight, move, is_black) == FALSE) {
			continue ;
		}
        
		/* Check if the move is blocked by an occupied square */
		if (handle_occupied_tile(move, occupied, enemy, &attacks)) { continue ; }
        
		/* Add the move to the attacks */
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
Bitboard get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type, s8 check_legal) {
 	
	static PieceMove	piece_move_array[PIECE_MOVE_ARRAY_SIZE] = {
		{WHITE_PAWN, BLACK_PAWN, get_pawn_moves},
		{WHITE_KNIGHT, BLACK_KNIGHT, get_knight_moves},
		{WHITE_BISHOP, BLACK_BISHOP, get_bishop_moves},
		{WHITE_ROOK, BLACK_ROOK, get_rook_moves},
		{WHITE_QUEEN, BLACK_QUEEN, get_queen_moves},
		{WHITE_KING, BLACK_KING, get_king_moves},
	}; 
	GetMoveFunc 		get_move_func = NULL;
	s8					is_black = (piece_type >= BLACK_PAWN);

	/* Get the piece move function */
	get_move_func = get_piece_move_func(piece_move_array, piece_type);
	
	/* If not get move function found, empty tile is selected reset the possible move variable */
	if (!get_move_func) {
		return (0);
	}
	return (get_move_func(board, piece, piece_type, is_black, check_legal));
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


/* @brief Update special info byte for the king and rook
 * @param b		ChessBoard struct
 * @param type	ChessPiece enum
 * @param tile_from	ChessTile enum
*/
void board_special_info_handler(ChessBoard *b, ChessPiece type, ChessTile tile_from) {
	
	static const SpecialInfo special_info[SPECIAL_INFO_SIZE] = {
		{WHITE_KING, WHITE_KING_MOVED, WHITE_KING_START_POS},
		{WHITE_ROOK, WHITE_KING_ROOK_MOVED, WHITE_KING_ROOK_START_POS},
		{WHITE_ROOK, WHITE_QUEEN_ROOK_MOVED, WHITE_QUEEN_ROOK_START_POS},
		{BLACK_KING, BLACK_KING_MOVED, BLACK_KING_START_POS},
		{BLACK_ROOK, BLACK_KING_ROOK_MOVED, BLACK_KING_ROOK_START_POS},
		{BLACK_ROOK, BLACK_QUEEN_ROOK_MOVED, BLACK_QUEEN_ROOK_START_POS},
	};

	for (s32 i = 0; i < SPECIAL_INFO_SIZE; i++) {
		s32 idx = special_info[i].info_idx;
		if (special_info[i].type == type && special_info[i].tile_from == tile_from && u8ValueGet(b->info, idx) == FALSE) {
			b->info = u8ValueSet(b->info, idx, TRUE);
			ft_printf_fd(1, PURPLE"Special info set to TRUE for %s on %s\n"RESET, chess_piece_to_string(type), TILE_TO_STRING(tile_from));
		}
	}
}

/* @brief Handle castle move (move rook if needed)
 * @param b		ChessBoard struct
 * @param type	ChessPiece enum
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
*/
void handle_castle_move(ChessBoard *b, ChessPiece type, ChessTile tile_from, ChessTile tile_to) {
	ChessTile rook_from = 0, rook_to = 0;
	ChessPiece rook_type = EMPTY;
	if (type == BLACK_KING || type == WHITE_KING) {
		/* Check if the king is moving 2 tiles (Castle move) */
		if (INT_ABS_DIFF(tile_from, tile_to) == 2) {
			/* Check if the king is moving to the right or left */
			if (tile_to == tile_from + 2) {
				rook_from = tile_from + 3;
				rook_to = tile_from + 1;
			} else if (tile_to == tile_from - 2) {
				rook_from = tile_from - 4;
				rook_to = tile_from - 1;
			}
			rook_type = (type == BLACK_KING) ? BLACK_ROOK : WHITE_ROOK;
			move_piece(b, rook_from, rook_to, rook_type);
		}
	}
}


/* @brief Move a piece from a tile to another and update the board state
 * @param board		ChessBoard struct
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
 * @param type		ChessPiece enum
*/
void move_piece(ChessBoard *board, ChessTile tile_from, ChessTile tile_to, ChessPiece type) {
	Bitboard	mask_from = 1ULL << tile_from;
	Bitboard	mask_to = 1ULL << tile_to;
	ChessPiece	enemy_piece = get_piece_from_mask(board, mask_to);
	
	/* Check if the enemy piece need to be kill/remove */
	if (enemy_piece != EMPTY) {
		ft_printf_fd(1, RED"Kill %s on [%s]\n"RESET, \
			chess_piece_to_string(enemy_piece), TILE_TO_STRING(tile_to));
		board->piece[enemy_piece] &= ~mask_to;
	}

	/* Check if the move is a castle move and move rook if needed */
	handle_castle_move(board, type, tile_from, tile_to);

	/* Remove the piece from the from tile */
	board->piece[type] &= ~mask_from;
	
	/* Add the piece to the to tile */
	board->piece[type] |= mask_to;

	/* Update the piece state */
	update_piece_state(board);

	/* Check if the enemy king is check and mat or PAT */
	verify_check_and_mat(board, !(type >= BLACK_PAWN));

	/* Set special info for the king and rook */
	board_special_info_handler(board, type, tile_from);
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
