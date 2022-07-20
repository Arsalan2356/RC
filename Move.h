#pragma once
#include <iostream>
#include <vector>

class Move
{
public:
	static const int CASTLE_MOVE = 1;
	static const int EN_PASSANT_MOVE = 2;
	static const int PROMOTION = 4;

	// Square that the piece moved from
	int square_from;
	// Square that the piece moved to
	int square_to;
	// Piece moved (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order
	int piece;
	// Piece captured (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order
	// value is -1 if no piece exists

	int promoted;

	int move_id;

	int capture_flag;
	int double_flag;
	int en_passant_flag;
	int castle_flag;

	Move(uint64_t move_id);
	Move(int square_from, int square_to);
	Move(int square_from, int square_to, int piece, int promoted, int capture_flag, int double_flag, int en_passant_flag, int castle_flag);
	std::string to_fen(int diff);
	bool operator==(const Move &rhs);
	static uint64_t create_id(int square_from, int square_to, int piece, int promoted, int capture_flag, int double_flag, int en_passant_flag, int castle_flag);
};