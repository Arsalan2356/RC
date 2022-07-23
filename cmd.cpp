#include "Board.h"
#include "Move.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>

std::vector<std::string> split(std::string &line);

int main(int argc, char *argv[])
{
	std::string fen;
	if (argc > 1)
	{
		if (strcmp(argv[1], "-f") == 0)
		{
			if (argc > 2)
			{
				fen = argv[2];
			}
			else
			{
				std::cout << "-f requires a parameter."
						  << "\n"
						  << "Input has been ignored and fen has been initialized to: "
						  << "\n"
						  << "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
			}
		}
	}
	if (strcmp(fen.c_str(), "") == 0)
	{
		fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	}

	while (true)
	{
		std::string input;
		std::getline(std::cin, input);
		Board *board = new Board(fen);
		std::vector<std::string> inputs = split(input);
	}
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