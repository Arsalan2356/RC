#include "Move.h"

std::string piece_names(int piece_num);

// Only constructor which takes in a square_from, a square_to, a flag variable with multiple values (see Move.h), a piece_moved
// (need to account for white/black), a piece_captured (-1 if none and needs to account for white/black), a promotion piece
// (-1 if none, not required to account for black (see to_pgn) but it's preferred for piece conversion)

Move::Move(uint64_t move_id)
{
	// 6 bits
	this->square_from = (move_id & 0x3f);
	// 6 bits
	this->square_to = (move_id & 0xfc0) >> 6;
	// 4 bits
	this->piece = (move_id & 0xf000) >> 12;
	// 4 bits
	this->promoted = (move_id & 0xf0000) >> 16;
	// 4 bits
	this->capture_flag = (move_id & 0x100000);
	this->double_flag = (move_id & 0x200000);
	this->en_passant_flag = (move_id & 0x400000);
	this->castle_flag = (move_id & 0x800000);
	// 4 bits
	this->capture_piece = ((move_id & 0xf000000) >> 24);
	this->move_id = move_id;
}

Move::Move(int square_from, int square_to)
{
	this->square_from = square_from;
	this->square_to = square_to;
}

Move::Move(int square_from, int square_to, int piece, int promoted, int capture_flag, int double_flag, int en_passant_flag, int castle_flag, int capture_piece)
{
	this->square_from = square_from;
	this->square_to = square_to;
	// Checks for promotion, en_passant, and castling
	this->piece = piece;
	this->promoted = promoted;
	this->capture_flag = capture_flag;
	this->double_flag = double_flag;
	this->en_passant_flag = en_passant_flag;
	this->castle_flag = castle_flag;
	this->move_id = (square_from) | (square_to << 6) | (piece << 12) | (promoted << 16) | (capture_flag << 20) | (double_flag << 21) | (en_passant_flag << 22) | (castle_flag << 23);
}

// diff defines whether a rank/file/position needs to specified when the move
// is displayed in the log
void Move::to_pgn(std::string &pgn, int diff)
{
	pgn = "";
	if (!(castle_flag))
	{
		if (piece % 6 != 0)
		{
			pgn += piece_names(piece);
			if (diff == 1)
				pgn += (char)(square_from % 8 + 97);
			else if (diff == 2)
				pgn += ((char)((8 - (square_from / 8)) + 48));
			else if (diff == 3)
			{
				pgn += ((char)((square_from % 8) + 97));
				pgn += ((char)((8 - (square_from / 8)) + 48));
			}
		}

		if (capture_flag)
		{
			if (piece % 6 == 0)
				pgn += (char)(square_from % 8 + 97);
			pgn += "x";
		}
		pgn += ((char)((square_to % 8) + 97));
		pgn += ((char)((8 - (square_to / 8)) + 48));
		if (piece == 0 && (promoted != 0))
			pgn += "=" + piece_names(promoted);
	}
	else
	{
		if (square_to - square_from == 2)
		{
			pgn += "O-O";
		}
		else
		{
			pgn += "O-O-O";
		}
	}

	return;
}

bool Move::operator==(const Move &right)
{
	return move_id == right.move_id;
}
uint64_t Move::create_id(int square_from, int square_to, int piece, int promoted, int capture_flag, int double_flag, int en_passant_flag, int castle_flag, int capture_piece)
{
	return ((square_from) | (square_to << 6) | (piece << 12) | (promoted << 16) | (capture_flag << 20) | (double_flag << 21) | (en_passant_flag << 22) | (castle_flag << 23) | (capture_piece << 24));
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
	else
		return " ";
}
