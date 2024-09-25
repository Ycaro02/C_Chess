#ifndef FEN_NOTATION_H
#define FEN_NOTATION_H


typedef struct s_piece_to_fen {
	ChessPiece	piece;
	char		fen;
} PieceToFen;

typedef struct s_fen_format {
	char	board[8][9];		// Each row of the board
	char	color_turn[2];		// W or B
	char	castling[5];		// KQkq
	char	en_passant[3];		// Square of the en passant target, or "-" for none
	u8		halfmove;			// Number of halfmoves since the last capture or pawn advance
	char	*fullmove;			// The number of the full move
} FenFormat;

#define UNKOWN_PIECE	'?'
#define EMPTY_PIECE		' '

char *build_FEN_notation(SDLHandle *h);

#endif