#include "../include/chess.h"
#include "../include/chess_log.h"
#include "../include/FEN_notation.h"
#include "../include/handle_sdl.h"

/**
 * @brief Check if the rook is alive on the given tile
 * @param b The ChessBoard pointer
 * @param wanted_piece The wanted piece
 * @param tile The tile to check
 * @return TRUE if the rook is alive, FALSE otherwise
 */
FT_INLINE s8 is_rook_alive(ChessBoard *b, ChessPiece wanted_piece, ChessTile tile) {
	return (get_piece_from_tile(b, tile) == wanted_piece);
}

/**
 * @brief Check if the black king has moved
 * @param info The info to check
 * @return TRUE if the black king has moved, FALSE otherwise
 */
FT_INLINE s8 black_king_move(u8 info) {
	return (u8ValueGet(info, BLACK_KING_MOVED));
}

/**
 * @brief Check if the white king has moved
 * @param info The info to check
 * @return TRUE if the white king has moved, FALSE otherwise
 */
FT_INLINE s8 white_king_move(u8 info) {
	return (u8ValueGet(info, WHITE_KING_MOVED));
}

/**
 * @brief Check if the rook has moved
 * @param b The ChessBoard pointer
 * @param rook_piece The rook piece
 * @param start_tile The start tile
 * @param idx_info The index info
 * @return TRUE if the rook has moved, FALSE otherwise
 */
FT_INLINE s8 check_rook_move(ChessBoard *b, ChessPiece rook_piece, ChessTile start_tile, u8 idx_info) {
	if (!is_rook_alive(b, rook_piece, start_tile)) {
		return (TRUE);
	}
	return (u8ValueGet(b->info, idx_info));
}

/**
 * @brief Fill the castling permission
 * @param fen The FenFormat pointer
 * @param idx The index of the castling permission
 * @param perm The permission to check
 */
static void fill_castling_perm(FenFormat *fen, int *idx, char perm) {
	if (perm != ' ') {
		fen->castling[*idx] = perm;
		*idx += 1;
	}
}

/**
 * @brief Compute the castling permission
 * @param b The ChessBoard pointer
 * @param fen The FenFormat pointer
 * @param info The info to check
 */
static void compute_castling_perm(ChessBoard *b, FenFormat *fen, u8 info) {
	int		idx = 0;
	char	white_king_castle = ' ', white_queen_castle = ' ';
	char	black_king_castle = ' ', black_queen_castle = ' ';

    if (!white_king_move(info)) {
        white_king_castle = check_rook_move(b, WHITE_ROOK, WHITE_KING_ROOK_START_POS, WHITE_KING_ROOK_MOVED) ? ' ' : 'K';
        white_queen_castle = check_rook_move(b, WHITE_ROOK, WHITE_QUEEN_ROOK_START_POS, WHITE_QUEEN_ROOK_MOVED) ? ' ' : 'Q';
    }

    if (!black_king_move(info)) {
        black_king_castle = check_rook_move(b, BLACK_ROOK, BLACK_KING_ROOK_START_POS, BLACK_KING_ROOK_MOVED) ? ' ' : 'k';
        black_queen_castle= check_rook_move(b, BLACK_ROOK, BLACK_QUEEN_ROOK_START_POS, BLACK_QUEEN_ROOK_MOVED) ? ' ' : 'q';
    }

	fill_castling_perm(fen, &idx, white_king_castle);
	fill_castling_perm(fen, &idx, white_queen_castle);
	fill_castling_perm(fen, &idx, black_king_castle);
	fill_castling_perm(fen, &idx, black_queen_castle);

	if (idx == 0) {
		fen->castling[0] = '-';
	}

}

/**
 * @brief Convert the chess piece to FEN notation
 * @param piece The piece to convert
 * @param empty_count The empty count
 * @return The char FEN notation
 */
static char chess_piece_to_fen(ChessPiece piece, int *empty_count) {
	static const PieceToFen array[PIECE_MAX] = { \
		{WHITE_PAWN, 'P'},
		{WHITE_KNIGHT, 'N'},
		{WHITE_BISHOP, 'B'},
		{WHITE_ROOK, 'R'},
		{WHITE_QUEEN, 'Q'},
		{WHITE_KING, 'K'},
		{BLACK_PAWN, 'p'},
		{BLACK_KNIGHT, 'n'},
		{BLACK_BISHOP, 'b'},
		{BLACK_ROOK, 'r'},
		{BLACK_QUEEN, 'q'},
		{BLACK_KING, 'k'}
	};

	if (piece == EMPTY) {
		*empty_count += 1;
		return (EMPTY_PIECE);
	} else if (piece < 0 || piece >= PIECE_MAX) {
		return (UNKOWN_PIECE);
	} 
	return (array[piece].fen);
}

static char *concat_FEN(FenFormat *fen) {
	char *str_fen = NULL;

	/* Concat the board */
	for (int i = 7; i >= 0; i--) {
		str_fen = ft_strjoin_free(str_fen, fen->board[i], 'f');
		if (i > 0) {
			str_fen = ft_strjoin_free(str_fen, "/", 'f');
		}
	}

	/* Concat the color turn */
	str_fen = ft_strjoin_free(str_fen, " ", 'f');
	str_fen = ft_strjoin_free(str_fen, fen->color_turn, 'f');

	/* Concat the castling */
	str_fen = ft_strjoin_free(str_fen, " ", 'f');
	str_fen = ft_strjoin_free(str_fen, fen->castling, 'f');

	/* Concat the en passant */
	str_fen = ft_strjoin_free(str_fen, " ", 'f');
	str_fen = ft_strjoin_free(str_fen, fen->en_passant, 'f');

	/* Concat the halfmove */
	str_fen = ft_strjoin_free(str_fen, " ", 'f');
	str_fen = ft_strjoin_free(str_fen, (char[2]){fen->halfmove, '\0'}, 'f');
	str_fen = ft_strjoin_free(str_fen, " ", 'f');

	/* Concat the fullmove */
	str_fen = ft_strjoin_free(str_fen, fen->fullmove, 'f');
	return (str_fen);
}

/**
 * @brief Display the FEN notation
 * @param fen The FenFormat pointer
 */
static void display_FEN_notation(FenFormat *fen) {
	for (int i = 7; i >= 0; i--) {
		printf(ORANGE"%s", fen->board[i]);
		if (i > 0) {
			printf(PURPLE"/");
		}
	}
	printf (" %s", fen->color_turn);
	printf (" %s", fen->castling);
	printf (" %s", fen->en_passant);
	printf (" %c %s", fen->halfmove, fen->fullmove);
	printf("\n"RESET);

}

/**
 * @brief call the __builtin_ctzll function is a built-in function provided by GCC (GNU Compiler Collection) and Clang compilers. 
 * It stands for "Count Trailing Zeros Long Long" and is used to count
 * the number of trailing zero bits in a long long integer (which is typically 64 bits on most platforms).
 * @example For the binary number 0000 0000 0000 1000 (which is 8 in decimal), __builtin_ctzll would return 3 
 * because there are three trailing zeros before the first set bit.
 * @note this function is UB if value is 0
 * @param value The value to check
 * @return The tile enable or INVALID_TILE if not found
 */
ChessTile find_enable_tile(u64 value) {
    // for (int i = 0; i < 64; i++) {
    //     if (value & (1ULL << i)) {
    //         return (i);
    //     }
    // }
    // return (INVALID_TILE);
	if (value == 0) {
		return (INVALID_TILE);
	}
 	return ((ChessTile)__builtin_ctzll(value));
}

/**
 * @brief Build the FEN notation
 * @param h The SDLHandle pointer
 */
char *build_FEN_notation(SDLHandle *h) {
	ChessPiece	piece = EMPTY;
	FenFormat	*fen = ft_calloc(1, sizeof(FenFormat));
	char		fen_char = EMPTY_PIECE;
	int			empty_count = 0;
	int 		line_idx = 0;

	// CHESS_LOG(LOG_INFO, "Size FenFormat %lu\n", sizeof(FenFormat));

	for (int raw = 0; raw < 8; raw++) {
		line_idx = 0;
		empty_count = 0;
		for (int i = 0; i < 8; i++) {
			piece = get_piece_from_tile(h->board, (raw * 8) + i);
			fen_char = chess_piece_to_fen(piece, &empty_count);
			/* If not empty piece */
			if (fen_char != EMPTY_PIECE) {
				/* Fill the empty counter char before */
				if (empty_count > 0) {
					fen->board[raw][line_idx] = (empty_count + '0');
					line_idx++;
					empty_count = 0;
				}
				/* Fill the piece */
				fen->board[raw][line_idx] = fen_char;
				line_idx++;
			} else if (i == 7) { /* Last tile of the raw */
				fen->board[raw][line_idx] = (empty_count) + '0';
			}
			// CHESS_LOG(LOG_INFO, "Piece at %s: %s\n", ChessTile_to_str(raw * 8 + i), ChessPiece_to_str(piece));
			if (i == 7) {
				CHESS_LOG(LOG_INFO, "Row %d: |%s|\n", raw + 1, fen->board[raw]);
			}
		}
	}

	/* Set the color turn */
	fen->color_turn[0] = h->player_info.piece_start == WHITE_PAWN ? 'w' : 'b';

	/* Set the castling permission */
	compute_castling_perm(h->board, fen, h->board->info);

	/* Set the en passant value */
	fen->en_passant[0] = '-';
	fen->en_passant[1] = '\0';
	ChessTile tile = find_enable_tile(h->board->en_passant);
	if (tile != INVALID_TILE) {
		fen->en_passant[0] = ChessTile_to_str(tile)[0] + 32; // Lowercase
		fen->en_passant[1] = ChessTile_to_str(tile)[1];
	}

	/* Set the halfmove */
	fen->halfmove = h->board->halfmove_count + '0';

	/* Set the fullmove */
	fen->fullmove = ft_itoa(h->board->fullmove_count);

	/* Print the FEN notation */
	display_FEN_notation(fen);

	char *str_fen = concat_FEN(fen);
	CHESS_LOG(LOG_INFO, "FEN Notation: "ORANGE"%s\n"RESET, str_fen);
	// free(str_fen);

	/* Free the memory */
	free(fen->fullmove);
	free(fen);

	return (str_fen);

} 