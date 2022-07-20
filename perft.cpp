#include "Board.h"

std::string square_to_fen(int square);

// Perft Driver and Test functions
void Board::perft(int depth)
{
	// reccursion escape condition
	if (depth == 0)
	{
		// increment nodes count (count reached positions)
		nodes++;
		return;
	}

	// create move list instance
	moves_struct move_list[1];

	// generate moves
	generate_moves(move_list);

	// loop over generated moves
	for (int move_count = 0; move_count < move_list->count; move_count++)
	{
		// preserve board state
		copy_board();

		// make move
		if (!make_move(move_list->moves[move_count], all_moves))
			// skip to the next move
			continue;

		// call perft driver recursively
		perft(depth - 1);

		// take back
		take_back();
	}
}

void Board::perft_depth(int depth)
{
	for (int i = 1; i <= depth; i++)
	{
		nodes = 0ULL;
		perft(i);
		std::cout << "Depth " << i << ": " << nodes << "\n";
	}
}

void Board::perft_divide(int depth)
{
	moves_struct curr_moves;
	generate_moves(&curr_moves);
	unsigned long num_nodes = 0;
	for (int count = 0; count < curr_moves.count; count++)
	{
		Move move_in = Move(curr_moves.moves[count]);
		std::cout << square_to_fen(move_in.square_from) << square_to_fen(move_in.square_to);
		if (move_in.en_passant_flag)
		{
			std::cout << " e.p.";
		}
		if (move_in.promoted != 0)
		{
			switch (move_in.promoted % 6)
			{
			case 1:
				std::cout << "n";
				break;

			case 2:
				std::cout << "b";
				break;

			case 3:
				std::cout << "r";
				break;

			case 4:
				std::cout << "q";
				break;

			default:
				break;
			}
		}
		nodes = 0;
		copy_board();
		make_move(curr_moves.moves[count], 0);
		perft(depth - 1);
		std::cout << ": " << nodes << "\n";

		num_nodes += nodes;
		take_back();
	}
	std::cout << "Total Moves: " << num_nodes << "\n";
}

std::string square_to_fen(int square)
{
	std::string fen = "";
	fen += ((char)((square % 8) + 97));
	fen += ((char)((8 - (square / 8)) + 48));
	return fen;
}