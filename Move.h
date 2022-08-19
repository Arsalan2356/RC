#pragma once
#include <iostream>

// TODO
// Delete this class and move all functionality away from it
// It's just a waste of resources to keep calling this class instead
// of using the macros and ids
// Only need to really re-implement the create_id and to_pgn functions

class Move
{
	public:
	// Square that the piece moved from
	int square_from;
	// Square that the piece moved to
	int square_to;
	// Piece moved (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order
	int piece;
	// Piece captured (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order

	int promoted;

	int capture_flag;
	int double_flag;
	int en_passant_flag;
	int castle_flag;

	int move_id;

	int capture_piece;

	Move(uint64_t move_id);
	Move(int square_from, int square_to);
	Move(int square_from, int square_to, int piece, int promoted,
		int capture_flag, int double_flag, int en_passant_flag, int castle_flag,
		int capture_piece);
	void			to_pgn(std::string &pgn, int diff);
	bool			operator==(const Move &rhs);
	static uint64_t create_id(int square_from, int square_to, int piece,
		int promoted, int capture_flag, int double_flag, int en_passant_flag,
		int castle_flag, int capture_piece);

	static std::string piece_names(int piece_num);
};