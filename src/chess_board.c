#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"
#include "../include/network.h"

/* Update control bitboard */
void update_piece_control(ChessBoard *b) {
	b->white_control = get_piece_color_control(b, IS_WHITE);
	b->black_control = get_piece_color_control(b, IS_BLACK);

	/* Check for king in check */
	b->info = u8ValueSet(b->info, WHITE_CHECK, (b->black_control & b->piece[WHITE_KING]) != 0);
	b->info = u8ValueSet(b->info, BLACK_CHECK, (b->white_control & b->piece[BLACK_KING]) != 0);
}

/* Update occupied bitboard */
void update_piece_state(ChessBoard *b) {
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

	/* Update control bitboard */
	update_piece_control(b);
}

void init_board(ChessBoard *b) {

	if (b->lst) {
		CHESS_LOG(LOG_DEBUG, "Free Movelist\n");
		ft_lstclear(&b->lst, free);
	}	

	/* Set all pieces to 0 */
	fast_bzero(b, sizeof(ChessBoard));

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

	b->selected_piece = EMPTY;
	b->selected_tile = INVALID_TILE;
	b->last_tile_from = INVALID_TILE;
	b->last_tile_to = INVALID_TILE;
	b->possible_moves = 0;

	/* Update occupied and control bitboard */
	update_piece_state(b);
}

/* @brief Get piece from tile
 * @param b		ChessBoard struct
 * @param tile	ChessTile enum
 * @return ChessPiece enum
 */
ChessPiece get_piece_from_tile(ChessBoard *b, ChessTile tile) {
	Bitboard mask = 1ULL << tile;
	ChessPiece piece = EMPTY;
	
	if (tile == INVALID_TILE) {
		return (piece);
	}
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

/* @brief Get piece from mask
 * @param b		ChessBoard struct
 * @param mask	Bitboard mask (1ULL << tile), tile is the position of the piece
 * @return ChessPiece enum
 */
ChessPiece get_piece_from_mask(ChessBoard *b, Bitboard mask) {
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


void exit_func(SDLHandle *h) {
	CHESS_LOG(LOG_INFO, "exit_func\n");
	chess_destroy(h);
}

void replay_func(SDLHandle *h) {
	s8 network_flag = FALSE;

	CHESS_LOG(LOG_INFO, "Replay game\n");
	init_board(h->board);

	if (has_flag(h->flag, FLAG_NETWORK)) {
		network_flag = TRUE;
		send_game_end_to_server(h->player_info.nt_info->sockfd, h->player_info.nt_info->servaddr);
		/* Disconect from the server */
		unset_flag(&h->flag, FLAG_NETWORK);
		destroy_network_info(h);
	}
	h->game_start = TRUE;
	center_text_function_set(h, h->center_text, (BtnCenterText){"Cancel", cancel_search_func}, (BtnCenterText){NULL, NULL});
	update_graphic_board(h);
	if (network_flag) {
		search_game(h);
	} else {
		/* Remove center text and his flag */
		center_text_string_set(h, NULL, NULL);
		unset_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
	}
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
				CHESS_LOG(LOG_DEBUG, "Piece %s on [%s] has possible moves\n", ChessPiece_to_str(type), ChessTile_to_str(piece));
				mat = FALSE;
				break ;
			}
		}
	}

	SDLHandle *h = get_SDL_handle();

	if (check && mat) {

		char *checkmate_msg = ft_strjoin(color, " is checkmate");

		set_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
		
		/* Set game_start bool to false */
		h->game_start = FALSE;
		center_text_string_set(h, checkmate_msg, "Do you want to replay ?");
		free(checkmate_msg);
		center_text_function_set(h, h->center_text, (BtnCenterText) {"Replay", replay_func}, (BtnCenterText){"Exit", exit_func});
		return (TRUE);
	} else if (!check && mat) {
		set_flag(&h->flag, FLAG_CENTER_TEXT_INPUT);
		CHESS_LOG(LOG_ERROR, PURPLE"PAT detected Egality for %s\n"RESET, color);

		/* Set game_start bool to false */
		h->game_start = FALSE;
		center_text_string_set(h, "Pat", "Game Over");
		center_text_function_set(h, h->center_text, (BtnCenterText) {"Replay", replay_func}, (BtnCenterText){"Exit", exit_func});
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




/* Display bitboard for debug */
void display_bitboard(Bitboard bitboard, const char *msg) {
	printf("%s", msg);
	for (int i = 0; i < TILE_MAX; i++) {
		if (i % 8 == 0) {
			printf("\n");
		}
		printf("%d", (int)(bitboard >> i) & 1);
	}
	printf("\n");
}