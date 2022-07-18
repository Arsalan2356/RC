#include "Board.h"

void Board::compute_attack_tables()
{
	for (int pos = 0; pos < 64; pos++)
	{
		// Pawn attack tables
		if (pos > 7 && pos < 56)
		{
			if (pos % 8 != 0)
			{
				attack_tables_from[0][pos] |= (1ULL << (pos - 9));
			}
			if (pos % 8 != 7)
			{
				attack_tables_from[0][pos] |= (1ULL << (pos - 7));
			}
		}
		if (pos > 7 && pos < 56)
		{
			if (pos % 8 != 0)
			{
				attack_tables_from[3][pos] |= (1ULL << (pos + 7));
			}
			if (pos % 8 != 7)
			{
				attack_tables_from[3][pos] |= (1ULL << (pos + 9));
			}
		}

		// Knight attack table
		if (pos % 8 != 7)
		{
			if (pos > 15)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos - 15));
			}

			if (pos < 47)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos + 17));
			}
		}
		if (pos % 8 != 0)
		{
			if (pos < 48)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos + 15));
			}

			if (pos > 16)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos - 17));
			}
		}
		if (pos % 8 < 6)
		{
			if (pos < 54)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos + 10));
			}

			if (pos > 7)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos - 6));
			}
		}
		if (pos % 8 > 1)
		{
			if (pos < 56)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos + 6));
			}
			if (pos > 7)
			{
				attack_tables_from[1][pos] |= (1ULL << (pos - 10));
			}
		}

		if (pos % 8 != 0)
		{
			if (pos / 8 > 0)
			{
				attack_tables_from[2][pos] |= (1ULL << (pos - 9));
			}
			attack_tables_from[2][pos] |= (1ULL << (pos - 1));
			if (pos / 8 < 7)
			{
				attack_tables_from[2][pos] |= (1ULL << (pos + 7));
			}
		}
		if (pos % 8 != 7)
		{
			if (pos / 8 > 0)
			{
				attack_tables_from[2][pos] |= (1ULL << (pos - 7));
			}
			attack_tables_from[2][pos] |= (1ULL << (pos + 1));
			if (pos / 8 < 7)
			{
				attack_tables_from[2][pos] |= (1ULL << (pos + 1));
			}
		}

		if (pos / 8 > 0)
		{
			attack_tables_from[2][pos] |= (1ULL << (pos - 8));
		}
		if (pos / 8 < 7)
		{
			attack_tables_from[2][pos] |= (1ULL << (pos + 8));
		}
	}
}

void Board::compute_sliding_tables()
{
	for (int square = 0; square < 64; square++)
	{
		uint64_t attack_mask_bishop = bishop_mask(square);
		uint64_t attack_mask_rook = rook_mask(square);

		int occupancy_indices_bishop = (1 << bishop_bit_counts[square]);
		int occupancy_indices_rook = (1 << rook_bit_counts[square]);

		for (int index = 0; index < occupancy_indices_rook; index++)
		{
			if (index < occupancy_indices_bishop)
			{
				uint64_t occupancy_bishop = set_occupancy(index, bishop_bit_counts[square], attack_mask_bishop);
				int magic_index_bishop = (occupancy_bishop * bishop_magic_numbers[square]) >> (64 - bishop_bit_counts[square]);
				bishop_attacks_table[square][magic_index_bishop] = bishop_attacks(square, occupancy_bishop);
			}

			uint64_t occupancy_rook = set_occupancy(index, rook_bit_counts[square], attack_mask_rook);
			int magic_index_rook = (occupancy_rook * rook_magic_numbers[square]) >> (64 - rook_bit_counts[square]);
			rook_attacks_table[square][magic_index_rook] = rook_attacks(square, occupancy_rook);
		}
	}
}

uint64_t Board::set_occupancy(int index, int bits, uint64_t attack_mask)
{
	uint64_t occupancy = 0ULL;

	for (int count = 0; count < bits; count++)
	{
		int sq = __builtin_ffsll(attack_mask) - 1;
		attack_mask &= ~(1ULL << sq);

		if (index & (1 << count))
		{
			occupancy |= (1ULL << sq);
		}
	}

	return occupancy;
}
