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
	// Flags for castle/en_passant/promotion
	int flags;
	// Piece moved (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order
	int piece_moved;
	// Piece captured (0 - 5) indicate white pieces in order
	// of PNBRQK, (6 - 11) are black pieces in the same order
	// value is -1 if no piece exists
	int piece_captured;

	int promotion_piece;

	int move_id;

	Move(int square_from, int square_to);
	Move(int square_from, int square_to, int flags, int piece_moved, int piece_captured, int promotion_piece);
	std::string to_fen(int diff);
	bool operator==(const Move &rhs);
	static Move clone(Move *move);
};