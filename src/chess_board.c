#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"
#include "../include/network.h"
#include "../include/FEN_notation.h"

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

void init_board(ChessBoard *b, u32 *app_flag) {

	if (b->lst) {
		CHESS_LOG(LOG_DEBUG, "Free Movelist\n");
		ft_lstclear(&b->lst, free);
		if (b->white_kill_lst) {
			CHESS_LOG(LOG_DEBUG, "Free white kill list\n");
			ft_lstclear(&b->white_kill_lst, free);
		}
		if (b->black_kill_lst) {
			CHESS_LOG(LOG_DEBUG, "Free black kill list\n");
			ft_lstclear(&b->black_kill_lst, free);
		}
	}	

	if (has_flag(*app_flag, FLAG_FIRST_MOVE_PLAYED)) {
		CHESS_LOG(LOG_DEBUG, "First move played unset\n");
		unset_flag(app_flag, FLAG_FIRST_MOVE_PLAYED);
	}

	/* Set all pieces to 0 */
	fast_bzero(b, sizeof(ChessBoard));
	// CHESS_LOG(LOG_INFO, ORANGE"sizeof(ChessBoard) = %lu\n"RESET, sizeof(ChessBoard));

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
	// b->fen = NULL;
	// b->possible_moves = 0;
	// b->half_turn_count = 0;
	b->fullmove_count = 1;

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
	// init_board(h->board, &h->flag);
	reset_board(h);


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


ChessPiece fen_to_chess_piece(char fen_char) {
    switch (fen_char) {
        case 'P': return (WHITE_PAWN);
        case 'N': return (WHITE_KNIGHT);
        case 'B': return (WHITE_BISHOP);
        case 'R': return (WHITE_ROOK);
        case 'Q': return (WHITE_QUEEN);
        case 'K': return (WHITE_KING);
        case 'p': return (BLACK_PAWN);
        case 'n': return (BLACK_KNIGHT);
        case 'b': return (BLACK_BISHOP);
        case 'r': return (BLACK_ROOK);
        case 'q': return (BLACK_QUEEN);
        case 'k': return (BLACK_KING);
        default:  return (EMPTY);
    }
}

void init_board_from_FEN_data(SDLHandle *h, FenFormat *fen) {
    init_board(h->board, &h->flag);

    if (has_flag(h->flag, FLAG_STOCKFISH_BOT)) {
        unset_flag(&h->flag, FLAG_STOCKFISH_BOT);
        h->board->is_bot_playing = FALSE;
    }

    for (s32 i = 0; i < PIECE_MAX; i++) {
        h->board->piece[i] = 0;
    }

    ChessTile tile = 0;
    for (s32 row = 7; row >= 0; row--) {
        for (size_t col = 0; col < ft_strlen(fen->board[row]); col++) {
            char fen_char = fen->board[row][col];
            if (fen_char >= '1' && fen_char <= '8') {
                s32 empty_count = fen_char - '0';
                tile += empty_count;
            } else {
                ChessPiece piece = fen_to_chess_piece(fen_char);
                if (piece != EMPTY) {
                    h->board->piece[piece] |= (1ULL << tile);
                    tile++;
                } else {
                    CHESS_LOG(LOG_ERROR, "Invalid FEN character: %c\n", fen_char);
                    tile++;
                }
            }
        }
    }

    s8 color_turn = (fen->color_turn[0] == 'b') ? IS_BLACK  : IS_WHITE;

    if (has_flag(h->flag, FLAG_NETWORK)) {
        h->player_info.turn = (color_turn == h->player_info.color);
    } else {
        h->player_info.color = color_turn;
        h->player_info.piece_start = (color_turn == IS_WHITE) ? WHITE_PAWN : BLACK_PAWN;
        h->player_info.piece_end = (color_turn == IS_WHITE) ? WHITE_KING : BLACK_KING;
        h->player_info.turn = TRUE;
    }

    // s8 player_is_black = h->player_info.color == IS_BLACK;
    // if (is_black_turn == player_is_black) {
    //     h->player_info.turn = TRUE;
    // } else {
    //     h->player_info.turn = FALSE;
    // }

    // handle castling
    h->board->info = u8ValueSet(h->board->info, WHITE_KING_ROOK_MOVED, TRUE);
    h->board->info = u8ValueSet(h->board->info, WHITE_QUEEN_ROOK_MOVED, TRUE);
    h->board->info = u8ValueSet(h->board->info, BLACK_KING_ROOK_MOVED, TRUE);
    h->board->info = u8ValueSet(h->board->info, BLACK_QUEEN_ROOK_MOVED, TRUE);
    for (size_t i = 0; i < ft_strlen(fen->castling); i++) {
        char c = fen->castling[i];
        switch (c) {
            case 'K':
                h->board->info = u8ValueSet(h->board->info, WHITE_KING_ROOK_MOVED, FALSE);
                break;
            case 'Q':
                h->board->info = u8ValueSet(h->board->info, WHITE_QUEEN_ROOK_MOVED, FALSE);
                break;
            case 'k':
                h->board->info = u8ValueSet(h->board->info, BLACK_KING_ROOK_MOVED, FALSE);
                break;
            case 'q':
                h->board->info = u8ValueSet(h->board->info, BLACK_QUEEN_ROOK_MOVED, FALSE);
                break;
            default:
                break;
        }
    }


    h->board->halfmove_count = (u8)ft_atoi(fen->halfmove);
    h->board->fullmove_count = (u16)ft_atoi(fen->fullmove);

    // handle en passant
    ChessTile en_passant_tile = INVALID_TILE;
    if (fen->en_passant[0] != '-') {
        char letter = fen->en_passant[0];
        char number = fen->en_passant[1];
        s32 letter_idx = letter - 'a';
        s32 number_idx = number - '1';
        en_passant_tile = (ChessTile)(number_idx * 8 + letter_idx);
        h->board->en_passant = (1ULL << en_passant_tile);
        h->board->en_passant_tile = en_passant_tile;
        CHESS_LOG(LOG_INFO, "En passant tile: %s\n", ChessTile_to_str(en_passant_tile));
    } else {
        h->board->en_passant = 0;
        h->board->en_passant_tile = INVALID_TILE;
    }

    /* Update occupied and control bitboard */
    update_piece_state(h->board);
}

void init_board_from_FEN(SDLHandle *h, const char *fen_str) {
    init_board(h->board, &h->flag);

    FenFormat *fen = FEN_str_to_FEN_struct(h, fen_str);
    if (!fen) {
        CHESS_LOG(LOG_ERROR, "FEN_str_to_FEN_struct failed\n");
        return;
    }

    // Initialize the board with the FEN data
    init_board_from_FEN_data(h, fen);
    free(fen);
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