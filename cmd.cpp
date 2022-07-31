#include "Board.h"
#include "Move.h"
#include <iostream>
#include <sstream>
#include <string.h>

int main(int argc, char *argv[])
{
	bool use_nnue = true;
	if (argc != 1)
	{
		if (argv[1] == "-u")
		{
			if (argv[2] == "E")
			{
				use_nnue = false;
			}
			else if (argv[2] == "N")
			{
				use_nnue = true;
			}
		}
	}
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	Board *board = new Board(fen, true);
	std::cout << "NNUE has been loaded" << (use_nnue ? " and is being used." : ", but is not being used.") << "\n";
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
					if (inputs.size() > 3)
					{
						int depth = std::stoi(inputs[3]);
						if (inputs[2] == "both")
						{
							std::cout << "Engine Evaluation : "
									  << "\n";
							board->search_position(depth);
							std::cout << "------------------------"
									  << "\n";
							std::cout << "NNUE Evaluation : "
									  << "\n";
							board->search_position_nnue(depth);
							std::cout << "------------------------"
									  << "\n";
						}
						else if (inputs[2] == "engine")
						{
							board->search_position(depth);
						}
						else if (inputs[2] == "nnue")
						{
							board->search_position_nnue(depth);
						}
						else
						{
							(use_nnue ? board->search_position_nnue(depth) : board->search_position(depth));
						}
					}
					else
					{
						int depth = std::stoi(inputs[2]);
						(use_nnue ? board->search_position_nnue(depth) : board->search_position(depth));
					}
				}
				else if (inputs[1] == "eval")
				{
					if (inputs.size() > 2)
					{
						if (inputs[2] == "both")
						{
							std::cout << "Engine Evaluation : "
									  << "\n";
							std::cout << board->evaluate() << "\n";
							std::cout << "------------------------"
									  << "\n";
							std::cout << "NNUE Evaluation : "
									  << "\n";
							std::cout << board->nnue_eval() << "\n";
							std::cout << "------------------------"
									  << "\n";
						}
						else if (inputs[2] == "engine")
						{
							std::cout << board->evaluate() << "\n";
						}
						else if (inputs[3] == "nnue")
						{
							std::cout << board->nnue_eval() << "\n";
						}
						else
						{
							std::cout << (use_nnue ? board->nnue_eval() : board->evaluate()) << "\n";
						}
					}
					else
					{
						std::cout << (use_nnue ? board->nnue_eval() : board->evaluate()) << "\n";
					}
				}
			}
			else if (inputs[0] == "use")
			{
				if (inputs[1] == "nnue")
				{
					use_nnue = true;
					std::cout << "NNUE is now being used"
							  << "\n";
				}
				else if (inputs[1] == "engine")
				{
					use_nnue = false;
					std::cout << "NNUE is now disabled"
							  << "\n";
				}
			}

			else if (inputs[0] == "exit")
			{
				exit(0);
			}
	}
}
