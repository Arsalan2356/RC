#include "Move.h"

std::string piece_names(int piece_num);

// Only constructor which takes in a square_from, a square_to, a flag variable with multiple values (see Move.h), a piece_moved
// (need to account for white/black), a piece_captured (-1 if none and needs to account for white/black), a promotion piece
// (-1 if none, not required to account for black (see to_fen) but it's preferred for piece conversion)

Move::Move(uint64_t move_id)
{
	this->square_from = (move_id & 0x3f);
	this->square_to = (move_id & 0xfc0) >> 6;
	this->flags = (move_id & 0x7000) >> 12;
	this->piece_moved = (move_id & 0x78000) >> 15;
	this->piece_captured = ((move_id & 0x780000) >> 19) - 1;
	this->promotion_piece = ((move_id & 0x7800000) >> 23) - 1;
	this->move_id = move_id;
}

Move::Move(int square_from, int square_to)
{
	this->square_from = square_from;
	this->square_to = square_to;
}

Move::Move(int square_from, int square_to, int flags, int piece_moved, int piece_captured, int promotion_piece)
{
	this->square_from = square_from;
	this->square_to = square_to;
	// Checks for promotion, en_passant, and castling
	this->flags = flags;
	this->piece_moved = piece_moved;
	this->piece_captured = piece_captured;
	this->promotion_piece = promotion_piece;
	// square_from bits (0 - 5)
	// square_to bits (6 - 11)
	// flags (12 - 14)
	// piece_moved (15 - 18)
	// piece_captured (19 - 22)
	// promotion_piece (23 - 26)
	this->move_id = (square_from | (square_to << 6) | (flags << 12) | (piece_moved << 15) | ((piece_captured + 1) << 19) | ((promotion_piece + 1) << 23));
}

// diff defines whether a rank/file/position needs to specified when the move
// is displayed in the log
std::string Move::to_fen(int diff)
{
	std::string fen = "";
	if (!(flags & CASTLE_MOVE))
	{
		if (piece_moved % 6 != 0)
		{
			fen += piece_names(piece_moved);
			if (diff == 1)
				fen += (char)(square_from % 8 + 97);
			else if (diff == 2)
				fen += ((char)((8 - (square_from / 8)) + 48));
			else if (diff == 3)
			{
				fen += ((char)((square_from % 8) + 97));
				fen += ((char)((8 - (square_from / 8)) + 48));
			}
		}

		if (piece_captured != -1)
		{
			if (piece_moved % 6 == 0)
				fen += (char)(square_from % 8 + 97);
			fen += "x";
		}
		fen += ((char)((square_to % 8) + 97));
		fen += ((char)((8 - (square_to / 8)) + 48));
		if (piece_moved == 0 && (flags & PROMOTION))
			fen += "=" + piece_names(promotion_piece);
	}
	else
	{
		if (square_to - square_from == 2)
		{
			fen += "O-O";
		}
		else
		{
			fen += "O-O-O";
		}
	}

	return fen;
}

bool Move::operator==(const Move &right)
{
	return move_id == right.move_id;
}

Move Move::clone(Move *move)
{
	return Move(move->square_from, move->square_to, move->flags, move->piece_moved, move->piece_captured, move->promotion_piece);
}

uint64_t Move::create_id(int square_from, int square_to, int flags, int piece_moved, int piece_captured, int promotion_piece)
{
	return (square_from | (square_to << 6) | (flags << 12) | (piece_moved << 15) | ((piece_captured + 1) << 19) | ((promotion_piece + 1) << 23));
}

// Independent of color (no need to add 6 for black)
std::string piece_names(int piece_num)
{
	if (piece_num % 6 == 1)
	{
		return "N";
	}
	else if (piece_num % 6 == 2)
	{
		return "B";
	}
	else if (piece_num % 6 == 3)
	{
		return "R";
	}
	else if (piece_num % 6 == 4)
	{
		return "Q";
	}
	else if (piece_num % 6 == 5)
	{
		return "K";
	}
	else
		return " ";
}
