#include "Board.h"
#include <SDL2/SDL_image.h>

std::vector<std::string> split(const std::string &fen);
int fen_to_sq(std::string fen);

Board::Board(SDL_Renderer &renderer, const std::string &fen)
{
	srand(time(0));

	for (int i = 0; i < 12; i++)
	{
		std::string file_name = "./images/" + std::to_string(i) + ".png";
		piece_textures[i] = IMG_LoadTexture(&renderer, file_name.c_str());
	}

	std::vector<std::string> v = split(fen);
	int pos = 0;

	for (char &c : v[0])
	{
		if (isdigit(c) == 0)
		{

			switch (std::tolower(c, std::locale()))
			{
			case 'p':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[0] |= (1ULL << pos);
				}
				else
				{
					white_pieces[0] |= (1ULL << pos);
				}
				break;
			case 'n':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[1] |= (1ULL << pos);
				}
				else
				{
					white_pieces[1] |= (1ULL << pos);
				}
				break;
			case 'b':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[2] |= (1ULL << pos);
				}
				else
				{
					white_pieces[2] |= (1ULL << pos);
				}
				break;
			case 'r':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[3] |= (1ULL << pos);
				}
				else
				{
					white_pieces[3] |= (1ULL << pos);
				}
				break;
			case 'q':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[4] |= (1ULL << pos);
				}
				else
				{
					white_pieces[4] |= (1ULL << pos);
				}
				break;
			case 'k':
				if (std::tolower(c, std::locale()) == c)
				{
					black_pieces[5] |= (1ULL << pos);
				}
				else
				{
					white_pieces[5] |= (1ULL << pos);
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

	all_white_pieces = 0;
	for (int i = 0; i < 6; i++)
	{
		all_white_pieces |= white_pieces[i];
	}

	all_black_pieces = 0;
	for (int i = 0; i < 6; i++)
	{
		all_black_pieces |= black_pieces[i];
	}

	all_pieces = all_white_pieces | all_black_pieces;

	white_to_move = (v[1] == "w");

	castle_rights.set(0, ((v[2].find("K") != std::string::npos)));
	castle_rights.set(1, ((v[2].find("Q") != std::string::npos)));
	castle_rights.set(2, ((v[2].find("k") != std::string::npos)));
	castle_rights.set(3, ((v[2].find("q") != std::string::npos)));

	if (v[3] != "-")
	{
		en_passant_sq = fen_to_sq(v[3]);
	}

	half_moves = std::stoi(v[4]);

	move_index = std::stoi(v[5]) - 1;

	for (int i = 0; i < 6; i++)
	{
		board_states[move_index][i] = white_pieces[i];
		board_states[move_index][i + 6] = black_pieces[i];
	}
	board_states[move_index][12] = (white_to_move ? 1 : 0);
	board_states[move_index][13] = castle_rights.to_ulong();
	board_states[move_index][14] = en_passant_sq;

	std::cout << "Computing attack tables for non-sliding pieces"
			  << "\n";
	compute_attack_tables();
	std::cout << "Done"
			  << "\n";
	std::cout << "Initializing Magic Numbers"
			  << "\n";
	init_magic_numbers();
	std::cout << "Done"
			  << "\n";
	std::cout << "Computing attack tables for sliding pieces"
			  << "\n";
	compute_sliding_tables();
	std::cout << "Done"
			  << "\n";

	generate_legal_moves();
};

Board::Board(SDL_Renderer &renderer) : Board::Board(renderer, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
{
	return;
}

int fen_to_sq(std::string fen)
{
	int rank = (int)(8 - (fen[1] - '0'));
	int file = (int)(fen[0] - 'a');

	return rank * 8 + file;
}

int Board::get_piece(int square)
{
	for (int i = 0; i < 12; i++)
	{
		uint64_t piece = (i < 6 ? white_pieces[i] : black_pieces[i - 6]);
		if (piece && (1ULL << square))
		{
			return i;
		}
	}
	return -1;
}