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

/* Struct for piece move */
struct s_piece_move {
	ChessPiece	white_piece_type;
	ChessPiece	black_piece_type;
	GetMoveFunc get_move_func;
};

/* Typedef for piece move */
typedef struct s_piece_move PieceMove;

/* Array of piece move struct */
#define PIECE_MOVE_ARRAY { \
	{WHITE_KNIGHT, BLACK_KNIGHT, get_knight_moves}, \
	{WHITE_BISHOP, BLACK_BISHOP, get_bishop_moves}, \
	{WHITE_ROOK, BLACK_ROOK, get_rook_moves}, \
	{WHITE_QUEEN, BLACK_QUEEN, get_queen_moves}, \
	{WHITE_KING, BLACK_KING, get_king_moves}, \
} 

#define PIECE_MOVE_ARRAY_SIZE 5


GetMoveFunc get_piece_move_func(PieceMove *piece_move, ChessPiece piece_type) {
	for (int i = 0; i < PIECE_MOVE_ARRAY_SIZE; i++) {
		if (piece_move[i].white_piece_type == piece_type || piece_move[i].black_piece_type == piece_type) {
			return (piece_move[i].get_move_func);
		}
	}
	return (NULL);
}

Bitboard get_piece_move(ChessBoard *board, Bitboard piece, ChessPiece piece_type) {
 	
	PieceMove	piece_move[PIECE_MOVE_ARRAY_SIZE] = PIECE_MOVE_ARRAY;
	GetMoveFunc get_move = NULL;
	Bitboard	enemy = (piece_type >= BLACK_PAWN) ? board->white : board->black;
	
	if (piece_type == WHITE_PAWN || piece_type == BLACK_PAWN) {
		return (get_pawn_moves(piece, board->occupied, enemy, piece_type == BLACK_PAWN));
	}
	
	get_move = get_piece_move_func(piece_move, piece_type);
	if (!get_move) {
		ft_printf_fd(2, "Error: get_piece_move_func failed\n");
		return (0);
	}
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

	// Bitboard enemy = 0, piece = 0;
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
			// piece = 1ULL << tile_selected;
			board->possible_moves = get_piece_move(board, (1ULL << tile_selected), piece_type);
			// if (piece_type == WHITE_PAWN || piece_type == BLACK_PAWN) {
			// 	enemy = (piece_type == BLACK_PAWN) ? board->white : board->black;
			// 	board->possible_moves = get_pawn_moves(piece, board->occupied, enemy, piece_type == BLACK_PAWN);
			// } 
			// else if (piece_type == BLACK_BISHOP || piece_type == WHITE_BISHOP) {
			// 	board->possible_moves = get_possible_move(board, piece, piece_type, get_bishop_moves);
			// } 
			// else if (piece_type == BLACK_ROOK || piece_type == WHITE_ROOK) {
			// 	board->possible_moves = get_possible_move(board, piece, piece_type, get_rook_moves);
			// }
			// else if (piece_type == BLACK_QUEEN || piece_type == WHITE_QUEEN) {
			// 	board->possible_moves = get_possible_move(board, piece, piece_type, get_queen_moves);
			// }
			// else if (piece_type == BLACK_KING || piece_type == WHITE_KING) {
			// 	board->possible_moves = get_possible_move(board, piece, piece_type, get_king_moves);
			// }
			// else if (piece_type == BLACK_KNIGHT || piece_type == WHITE_KNIGHT) {
			// 	board->possible_moves = get_possible_move(board, piece, piece_type, get_knight_moves);
			// }
			// else {
			// 	board->possible_moves = 0;
			// }
		}

		windowClear(handle->renderer);
		draw_board(handle);
		SDL_RenderPresent(handle->renderer);
	}
	return (0);
}
