#include "../include/chess.h"
#include "../include/chess_log.h"


/* @brief Add a move to the move list
 * @param lst		ChessMoveList head pointer 
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
 * @param piece_from	ChessPiece enum
 * @param piece_to	ChessPiece enum
 * @return TRUE if the move is added, FALSE otherwise
 */
s8 move_save_add(ChessMoveList **lst, ChessTile tile_from, ChessTile tile_to, ChessPiece piece_from, ChessPiece piece_to) {
	ChessMoveList	*node = NULL;
	MoveSave		*m = malloc(sizeof(MoveSave));

	if (!m) {
		return (FALSE);
	}

	m->tile_from = tile_from;
	m->tile_to = tile_to;
	m->piece_from = piece_from;
	m->piece_to = piece_to;

	node = ft_lstnew(m);
	if (!node) {
		free(m);
		return (FALSE);
	}
	ft_lstadd_back(lst, node);
	return (TRUE);
}

int sort_chess_piece(void *a, void *b) {
	ChessPiece	*pa = (ChessPiece *)a;
	ChessPiece	*pb = (ChessPiece *)b;

	return (*pa < *pb);
}

void add_kill_lst(ChessBoard *b, ChessPiece killed_piece) {
	ChessPieceList	*node = NULL;
	ChessPiece		*piece = malloc(sizeof(ChessPiece));
	s8 				is_black = (killed_piece >= BLACK_PAWN);

	if (!piece) {
		CHESS_LOG(LOG_ERROR, "Malloc failed\n");
		return ;
	}
	*piece = killed_piece;
	node = ft_lstnew(piece);
	if (!node) {
		free(piece);
		CHESS_LOG(LOG_ERROR, "Malloc failed\n");
		return ;
	}

	if (is_black) {
		ft_lstadd_back(&b->black_kill_lst, node);
		list_sort(&b->black_kill_lst, sort_chess_piece);
	} else {
		ft_lstadd_back(&b->white_kill_lst, node);
		list_sort(&b->white_kill_lst, sort_chess_piece);
	}
}


/* @brief Display the move list
 * @param lst	ChessMoveList head pointer
 */
void display_move_list(ChessMoveList *lst) {
	ChessMoveList	*tmp = lst;
	MoveSave		*m = NULL;

	while (tmp) {
		m = tmp->content;
		printf("Move from: "CYAN"[%s]"RESET" -> "PURPLE"[%s]"RESET": Piece from: "CYAN"|%s|"RESET" -> to: "PURPLE"|%s|\n"RESET, ChessTile_to_str(m->tile_from), ChessTile_to_str(m->tile_to), ChessPiece_to_str(m->piece_from), ChessPiece_to_str(m->piece_to));
		tmp = tmp->next;
	}
	printf("-----------------------------------------------------------------------\n");
}