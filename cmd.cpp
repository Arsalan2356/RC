#include "Board.h"
#include "Move.h"
#include <iostream>
#include <sstream>
#include <string.h>

int main(int argc, char *argv[])
{
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	Board *board = new Board(fen);
	while (true)
	{
		std::string input;
		std::getline(std::cin, input);
		std::vector<std::string> inputs = split(input);
		if (inputs.size() > 0)
			if (inputs[0] == "position" && inputs[1] == "fen")
			{
				fen = inputs[2] + " " + inputs[3] + " " + inputs[4] + " " + inputs[5] + " " + inputs[6] + " " + inputs[7];
				board->init(fen);
			}
			else if (inputs[0] == "go")
			{
				if (inputs[1] == "perft")
				{
					if (inputs[2] == "divide")
					{
						int depth = std::stoi(inputs[3]);
						board->perft_divide(depth);
					}
					else if (inputs[2] == "depth")
					{
						int depth = std::stoi(inputs[3]);
						board->perft_depth(depth);
					}
					else
					{
						int depth = std::stoi(inputs[2]);
						board->perft_divide(depth);
					}
				}
				else if (inputs[1] == "search")
				{
					int depth = std::stoi(inputs[2]);
					board->search_position(depth);
				}
				else if (inputs[1] == "eval")
				{
					std::cout << board->evaluate() << "\n";
				}
			}
			else if (inputs[0] == "exit")
			{
				exit(0);
			}
	}
}
