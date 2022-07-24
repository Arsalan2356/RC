#include "Board.h"

void Board::init_zobrist()
{
	for (int piece = P; piece <= k; piece++)
	{
		for (int square = 0; square < 64; square++)
		{
			zobrist_keys[piece][square] = random_uint64();
			if (piece == k)
				en_passant_zobrist[square] = random_uint64();
		}
	}

	for (int i = 0; i < 16; i++)
	{
		castle_zobrist[i] = random_uint64();
	}

	zobrist_side_key = random_uint64();
}
