#include "../include/chess.h"
#include "../include/handle_sdl.h"

void destroy_sdl_handle(SDLHandle *handle) {
	free(handle->board);
	for (int i = 0; i < PIECE_MAX; i++) {
		unloadTexture(handle->piece_texture[i]);
	}
	free(handle->piece_texture);
}

/* Window size */
#define WINDOW_WIDTH (8 * TILE_SIZE + 9 * TILE_SPACING)
#define WINDOW_HEIGHT (8 * TILE_SIZE + 9 * TILE_SPACING + TOP_BAND_HEIGHT)

/* Function pointer for get move */
typedef Bitboard (*GetMoveFunc)(Bitboard, Bitboard, Bitboard);

Bitboard get_possible_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type, GetMoveFunc get_move) {
	Bitboard enemy = (piece_type >= BLACK_PAWN) ? board->white : board->black;
	return (get_move(piece, board->occupied, enemy));
}

int main(void) {
	ChessBoard *board = ft_calloc(1, sizeof(ChessBoard));
	SDLHandle	*handle = NULL;
	if (!board) {
		return (1);
	}

	init_board(board);
	handle = createSDLHandle(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess", board);
	if (!handle) {
		return (1);
	}

	Bitboard enemy = 0, piece = 0;
	ChessTile tile_selected = INVALID_TILE;
	ChessPiece piece_type = EMPTY; 

	while (windowIsOpen(handle->window)) {
		tile_selected = eventHandler(handle);
		if (tile_selected == CHESS_QUIT) {
			destroy_sdl_handle(handle);
			windowClose(handle->window, handle->renderer);
			free(handle);
			break ;
		}
		if (tile_selected != INVALID_TILE) {
			piece_type = get_piece(board, tile_selected);
			piece = 1ULL << tile_selected;
			if (piece_type == WHITE_PAWN || piece_type == BLACK_PAWN) {
				enemy = (piece_type == BLACK_PAWN) ? board->white : board->black;
				board->possible_moves = get_pawn_moves(piece, board->occupied, enemy, piece_type == BLACK_PAWN);
			} 
			else if (piece_type == BLACK_BISHOP || piece_type == WHITE_BISHOP) {
				board->possible_moves = get_possible_move(board, piece, piece_type, get_bishop_moves);
			} 
			else if (piece_type == BLACK_ROOK || piece_type == WHITE_ROOK) {
				board->possible_moves = get_possible_move(board, piece, piece_type, get_rook_moves);
			}
			else if (piece_type == BLACK_QUEEN || piece_type == WHITE_QUEEN) {
				board->possible_moves = get_possible_move(board, piece, piece_type, get_queen_moves);
			}
			else if (piece_type == BLACK_KING || piece_type == WHITE_KING) {
				board->possible_moves = get_possible_move(board, piece, piece_type, get_king_moves);
			}
			else if (piece_type == BLACK_KNIGHT || piece_type == WHITE_KNIGHT) {
				board->possible_moves = get_possible_move(board, piece, piece_type, get_knight_moves);
			}
			else {
				board->possible_moves = 0;
			}
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
