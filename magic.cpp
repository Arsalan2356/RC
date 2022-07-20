#include "Board.h"

void Board::init_magic_numbers()
{
	for (int square = 0; square < 64; square++)
	{
		bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], 2);

		rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], 3);
	}
}

uint64_t Board::find_magic_number(int square, int relevant_bits, int flag)
{
	uint64_t occupancies_magic[4096] = {0ULL};

	uint64_t attacks[4096] = {0ULL};

	uint64_t used_attacks[4096];

	uint64_t attack_mask = (flag == 2 ? mask_bishop_attacks(square) : mask_rook_attacks(square));

	int occupancy_indices = (1 << relevant_bits);

	attack_function = (flag == 2 ? &bishop_attacks_on_the_fly : &rook_attacks_on_the_fly);

	for (int index = 0; index < occupancy_indices; index++)
	{
		occupancies_magic[index] = set_occupancy(index, relevant_bits, attack_mask);

		attacks[index] = (this->*(this->attack_function))(square, occupancies_magic[index]);
		// attacks[index] = (flag == 2 ? bishop_attacks(square, occupancies[index]) : rook_attacks(square, occupancies[index]));
	}

	for (int random_count = 0; random_count < 100000000; random_count++)
	{
		uint64_t magic_number = get_magic_number();

		if (__builtin_popcountll((magic_number * attack_mask) & 0xFF00000000000000) < 6)
			continue;

		int index, fail;

		memset(used_attacks, 0ULL, sizeof(used_attacks));

		for (index = 0, fail = 0; !fail && index < occupancy_indices; index++)
		{
			int magic_index = (int)((occupancies_magic[index] * magic_number) >> (64 - relevant_bits));

			if (used_attacks[magic_index] == 0)
			{
				used_attacks[magic_index] = attacks[index];
			}
			else if (used_attacks[magic_index] != attacks[index])
			{
				fail = 1;
			}
		}

		if (!fail)
		{
			return magic_number;
		}
	}

	std::cout << "Magic Number not found"
			  << "\n";
	return 0;
}

uint64_t Board::get_magic_number()
{
	return random_uint64() & random_uint64() & random_uint64();
}

uint64_t Board::random_uint64()
{
	uint64_t u1, u2, u3, u4;
	u1 = (uint64_t)(rand()) & 0xFFFF;
	u2 = (uint64_t)(rand()) & 0xFFFF;
	u3 = (uint64_t)(rand()) & 0xFFFF;
	u4 = (uint64_t)(rand()) & 0xFFFF;

	return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t Board::bishop_attacks_on_the_fly(int square, uint64_t block)
{
	uint64_t result = 0ULL;
	int rk = square / 8, fl = square % 8, r, f;
	for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
	{
		result |= (1ULL << (f + r * 8));
		if (block & (1ULL << (f + r * 8)))
			break;
	}
	for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
	{
		result |= (1ULL << (f + r * 8));
		if (block & (1ULL << (f + r * 8)))
			break;
	}
	for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
	{
		result |= (1ULL << (f + r * 8));
		if (block & (1ULL << (f + r * 8)))
			break;
	}
	for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
	{
		result |= (1ULL << (f + r * 8));
		if (block & (1ULL << (f + r * 8)))
			break;
	}
	return result;
}

uint64_t Board::rook_attacks_on_the_fly(int square, uint64_t block)
{
	uint64_t result = 0ULL;
	int rk = square / 8, fl = square % 8, r, f;
	for (r = rk + 1; r <= 7; r++)
	{
		result |= (1ULL << (fl + r * 8));
		if (block & (1ULL << (fl + r * 8)))
			break;
	}
	for (r = rk - 1; r >= 0; r--)
	{
		result |= (1ULL << (fl + r * 8));
		if (block & (1ULL << (fl + r * 8)))
			break;
	}
	for (f = fl + 1; f <= 7; f++)
	{
		result |= (1ULL << (f + rk * 8));
		if (block & (1ULL << (f + rk * 8)))
			break;
	}
	for (f = fl - 1; f >= 0; f--)
	{
		result |= (1ULL << (f + rk * 8));
		if (block & (1ULL << (f + rk * 8)))
			break;
	}
	return result;
}
