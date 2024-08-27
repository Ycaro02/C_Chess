#include "../include/chess.h"
#include "../include/chess_log.h"


/* @brief Add a move to the move list
 * @param lst		ChessMoveList head pointer 
 * @param id		u32 id
 * @param tile_from	ChessTile enum
 * @param tile_to	ChessTile enum
 * @param piece_from	ChessPiece enum
 * @param piece_to	ChessPiece enum
 * @return TRUE if the move is added, FALSE otherwise
 */
s8 move_save_add(ChessMoveList **lst, u32 id, ChessTile tile_from, ChessTile tile_to, ChessPiece piece_from, ChessPiece piece_to) {
	MoveSave		*m = malloc(sizeof(MoveSave));
	ChessMoveList	*node = NULL;

	if (!m) {
		return (FALSE);
	}

	m->id = id;
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
		printf("Move id: %d, from: %s, to: %s, piece from: %s, piece to: %s\n", m->id, TILE_TO_STRING(m->tile_from), TILE_TO_STRING(m->tile_to), chess_piece_to_string(m->piece_from), chess_piece_to_string(m->piece_to));
		tmp = tmp->next;
	}
}