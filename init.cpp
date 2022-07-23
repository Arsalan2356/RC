#include "Board.h"
#include <sstream>
#include <string.h>
#include <vector>

std::vector<std::string> split(std::string &fen);
int fen_to_sq(std::string fen);

Board::Board(std::string &fen)
{
	init(fen);
};

void Board::init(std::string &fen)
{
	srand(time(0));

	castle_rights = 0;

	std::vector<std::string> v = split(fen);
	int pos = 0;

	for (int i = 0; i < 12; i++)
	{
		bitboards[i] = 0ULL;
	}

	for (int i = 0; i < 3; i++)
	{
		occupancies[i] = 0ULL;
	}

	half_moves = 0;
	move_index = 0;

	ply = 0;
	best_move = 0;

	for (char &c : v[0])
	{
		if (isdigit(c) == 0)
		{

			switch (std::tolower(c, std::locale()))
			{
			case 'p':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[6] |= (1ULL << pos);
				}
				else
				{
					bitboards[0] |= (1ULL << pos);
				}
				break;
			case 'n':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[7] |= (1ULL << pos);
				}
				else
				{
					bitboards[1] |= (1ULL << pos);
				}
				break;
			case 'b':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[8] |= (1ULL << pos);
				}
				else
				{
					bitboards[2] |= (1ULL << pos);
				}
				break;
			case 'r':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[9] |= (1ULL << pos);
				}
				else
				{
					bitboards[3] |= (1ULL << pos);
				}
				break;
			case 'q':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[10] |= (1ULL << pos);
				}
				else
				{
					bitboards[4] |= (1ULL << pos);
				}
				break;
			case 'k':
				if (std::tolower(c, std::locale()) == c)
				{
					bitboards[11] |= (1ULL << pos);
				}
				else
				{
					bitboards[5] |= (1ULL << pos);
				}
				break;
			default:
				break;
			}
			if (c != '/')
			{
				pos++;
			}
		}
		else
		{
			pos += ((int)c - 48);
		}
	}

	occupancies[white] = 0ULL;
	occupancies[black] = 0ULL;
	for (int i = 0; i < 6; i++)
	{
		occupancies[white] |= bitboards[i];
		occupancies[black] |= bitboards[i + 6];
	}

	occupancies[both] = occupancies[white] | occupancies[black];

	side = (v[1] == "w" ? 0 : 1);

	if ((v[2].find("K") != std::string::npos))
		castle_rights |= wk;
	if ((v[2].find("Q") != std::string::npos))
		castle_rights |= wq;
	if ((v[2].find("k") != std::string::npos))
		castle_rights |= bk;
	if ((v[2].find("q") != std::string::npos))
		castle_rights |= bq;

	if (v[3] != "-")
	{
		en_passant_sq = fen_to_sq(v[3]);
	}

	half_moves = std::stoi(v[4]);

	move_index = std::stoi(v[5]) - 1;

	std::cout << "Computing attack tables for non-sliding pieces"
			  << "\n";

	compute_attack_tables();
	std::cout << "Done"
			  << "\n";
	std::cout << "Initializing Magic Numbers"
			  << "\n";
	// init_magic_numbers();
	std::cout << "Done"
			  << "\n";
	std::cout << "Computing attack tables for sliding pieces"
			  << "\n";
	compute_sliding_tables();
	std::cout << "Done"
			  << "\n";
}

int fen_to_sq(std::string fen)
{
	int rank = (int)(8 - (fen[1] - '0'));
	int file = (int)(fen[0] - 'a');

	return rank * 8 + file;
}

std::vector<std::string> split(std::string &fen)
{
	std::vector<std::string> v;
	std::string buf;
	std::stringstream ss(fen);
	while (ss >> buf)
	{
		v.push_back(buf);
	}
	return v;
}

int Board::get_ls1b_index(uint64_t board)
{
	return __builtin_ffsll(board) - 1;
}

int Board::count_bits(uint64_t board)
{
	return __builtin_popcountll(board);
}