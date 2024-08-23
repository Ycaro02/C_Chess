#include "../include/chess.h"
#include "../include/handle_sdl.h"

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
void handle_castle_move(SDLHandle *handle, ChessPiece type, ChessTile tile_from, ChessTile tile_to) {
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
			move_piece(handle, rook_from, rook_to, rook_type);
		}
	}
}

/* @brief Verify if the move is a double step move for a pawn
 * @param type		ChessPiece enum
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
 * @return TRUE if the move is a double step move, FALSE otherwise
*/
static s8 is_pawn_double_step_move(ChessPiece type, ChessTile tile_from, ChessTile tile_to) {
	return ((type == WHITE_PAWN || type == BLACK_PAWN) && INT_ABS_DIFF(tile_from, tile_to) == 16);
}

/* @brief Update 'en passant' Bitboard if needed
 * @param b			ChessBoard struct
 * @param type		ChessPiece enum
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
*/
static void update_en_passant_bitboard(ChessBoard *b, ChessPiece type, ChessTile tile_from, ChessTile tile_to) {
	b->en_passant = 0;
	b->en_passant_tile = INVALID_TILE;
	if (is_pawn_double_step_move(type, tile_from, tile_to)) {
		b->en_passant = (1ULL << (tile_from + tile_to) / 2);
		b->en_passant_tile = tile_to;
	}
}

/* @brief Handle enemy piece kill
 * @param b			ChessBoard struct
 * @param type		ChessPiece enum
 * @param tile_to	ChessTile enum
 * @param mask_to	Bitboard of the destination tile
*/
static void handle_enemy_piece_kill(ChessBoard *b, ChessPiece type, ChessTile tile_to, Bitboard mask_to) {
	ChessPiece	enemy_piece = get_piece_from_mask(b, mask_to);
	
	if (enemy_piece != EMPTY) {
		display_kill_info(enemy_piece, tile_to);
		b->piece[enemy_piece] &= ~(1ULL << tile_to);
	} else if ((type == WHITE_PAWN || type == BLACK_PAWN) && mask_to == b->en_passant) {
		mask_to = (1ULL << b->en_passant_tile);
		enemy_piece = (type == WHITE_PAWN) ? BLACK_PAWN : WHITE_PAWN;
		display_kill_info(enemy_piece, b->en_passant_tile);
		b->piece[enemy_piece] &= ~mask_to;
	}
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



s32 check_pawn_promotion(SDLHandle *handle, ChessPiece type, ChessTile tile_to) {
	// ChessPiece queen = (type == WHITE_PAWN) ? WHITE_QUEEN : BLACK_QUEEN;
	s32 ret = TRUE;
	s8 is_pawn = (type == WHITE_PAWN || type == BLACK_PAWN);
	s8 is_black = (type >= BLACK_PAWN);
	s8 is_white = !is_black;


	/* Check if is the player control pawn or opponent */
	if (handle->player_info.color == IS_WHITE && is_black) {
		return (ret);
	} else if (handle->player_info.color == IS_BLACK && is_white) {
		return (ret);
	}


	/* Check if the pawn need to be promoted */
	if ((is_pawn && is_white && tile_to >= A8 && tile_to <= H8)
		|| (is_pawn && is_black && tile_to >= A1 && tile_to <= H1)) {
		ret = display_promotion_selection(handle, tile_to);
	}
	return (ret);
}

/* @brief Move a piece from a tile to another and update the board state
 * @param board		ChessBoard struct
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
 * @param type		ChessPiece enum
*/
s32 move_piece(SDLHandle *handle, ChessTile tile_from, ChessTile tile_to, ChessPiece type) {
	Bitboard	mask_from = 1ULL << tile_from;
	Bitboard	mask_to = 1ULL << tile_to;

	/* Check if the enemy piece need to be kill, handle 'en passant' kill too */
	handle_enemy_piece_kill(handle->board, type, tile_to, mask_to);

	/* Check if the move is a castle move and move rook if needed */
	handle_castle_move(handle, type, tile_from, tile_to);

	/* Remove the piece from the from tile */
	handle->board->piece[type] &= ~mask_from;
	
	/* Add the piece to the to tile */
	handle->board->piece[type] |= mask_to;

	/* Update the piece state */
	update_piece_state(handle->board);

	/* Check if the pawn need to be promoted */
	if (check_pawn_promotion(handle, type, tile_to) == CHESS_QUIT) {
		return (CHESS_QUIT);
	}

	/* Check if the enemy king is check and mat or PAT */
	verify_check_and_mat(handle->board, !(type >= BLACK_PAWN));

	/* Set special info for the king and rook */
	board_special_info_handler(handle->board, type, tile_from);

	/* Update 'en passant' Bitboard if needed */
	update_en_passant_bitboard(handle->board, type, tile_from, tile_to);

	return (TRUE);
}