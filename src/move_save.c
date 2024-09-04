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

/* @brief Display the move list
 * @param lst	ChessMoveList head pointer
 */
void display_move_list(ChessMoveList *lst) {
	ChessMoveList	*tmp = lst;
	MoveSave		*m = NULL;

	while (tmp) {
		m = tmp->content;
		printf("Move from: %s, to: %s, piece from: %s, piece to: %s\n", ChessTile_to_str(m->tile_from), ChessTile_to_str(m->tile_to), ChessPiece_to_str(m->piece_from), ChessPiece_to_str(m->piece_to));
		tmp = tmp->next;
	}
	printf("-----------------------------------------------------------------------\n");
}