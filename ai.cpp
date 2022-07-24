#include "Board.h"
#include <chrono>

int Board::evaluate()
{
	// Mid game evaluation score
	int mg_score = 0;

	// End game evaluation score
	int eg_score = 0;

	// phase
	int phase = 24;

	// current pieces bitboard copy
	uint64_t bitboard;

	// init piece & square
	int piece, square;

	// loop over piece bitboards
	for (int bb_piece = P; bb_piece <= k; bb_piece++)
	{
		// init piece bitboard copy
		bitboard = bitboards[bb_piece];

		// loop over pieces within a bitboard
		while (bitboard)
		{
			// init piece
			piece = bb_piece;

			// init square
			square = __builtin_ffsll(bitboard) - 1;
			// square = get_ls1b_index(bitboard);

			// score material weights
			mg_score += mg_value[piece];
			eg_score += eg_value[piece];

			if (bb_piece < 6)
			{
				mg_score += mg_piece_scores[piece][square];
				eg_score += eg_piece_scores[piece][square];
			}
			else
			{
				mg_score -= mg_piece_scores[piece - 6][FLIP(square)];
				eg_score -= eg_piece_scores[piece - 6][FLIP(square)];
			}

			phase -= phase_inc[piece % 6];

			// pop ls1b
			pop_bit(bitboard, square);
		}
	}

	int curr_phase = (phase * 256 + (24 / 2)) / 24;
	int score = ((mg_score * (256 - phase)) + (eg_score * phase)) / 256;

	// return final evaluation based on side
	return (side == white) ? score : -score;
}

void Board::search_position(int depth)
{
	// Clear helper data structures for search and nodes
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));
	nodes = 0ULL;

	// Search (1) = 0.0265 ms, Nodes 21
	// n -> n + 1 : mx exec time, Nodes : old_nodes -> new_nodes
	// n is the depth, m is the time ratio of exec time from the previous time,
	// old_nodes is the number of nodes without pvs search, new_nodes is the
	// number of nodes with pvs search
	// 1 -> 2 : 1.4x exec time, Nodes: 185 -> 118
	// 2 -> 3 : 4.07x exec time, Nodes : 2069 -> 695
	// 3 -> 4 : 5.91x exec time, Nodes : 15698 -> 3385
	// 4 -> 5 : 3.7x exec time, Nodes : 145,105 -> 17860
	// 5 -> 6 : 8.57x exec time, Nodes : 975,967 -> 109,626
	// 6 -> 7 : 5.2x exec time, Nodes : 8,752,070 -> 885,138
	// 7 -> 8 : 16.25x exec time, Nodes : 52,415,362 -> 9,807,072
	// 8 -> 9 : 23.08x exec time, Nodes : 438,805,075 -> 259,059,846
	// 9 -> 10 : 40.62x exec time, Nodes : 10,458,246,220
	// Depth 10, Nodes : 10,458,246,220
	// Search (9) = 54996.4 ms
	// Search (10) = 2,233,790 ms = 2233.8 s = 37.23 mins = 0.62 hrs
	// Average (excluding outliers) : 7.552x exec time from n -> n + 1 search depth
	// Node growth : ~7.5 times from n -> n + 1 search depth from (0 - 7)
	// Node growth : ~27 times from n -> n + 1 search depth from (8 - 10)

	auto t1 = std::chrono::high_resolution_clock::now();
	int score = negamax(-50000, 50000, depth);
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << "\n";
	Move move_x = Move(pv_table[0][0]);
	std::string fen;
	move_x.to_fen(fen, 3);
	std::cout << "Best Move : " << fen << " Score : " << score << " Nodes : " << nodes << " Depth : " << depth << "\n";
}

void Board::iterative_deepening(int depth)
{
	// Clear helper data structures for search and nodes
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));
	nodes = 0ULL;

	// Search (1) = 0.0265 ms, Nodes 21
	// n -> n + 1 : mx exec time, Nodes : old_nodes -> new_nodes
	// n is the depth, m is the time ratio of exec time from the previous time,
	// old_nodes is the number of nodes without pvs search, new_nodes is the
	// number of nodes with pvs search
	// 1 -> 2 : 1.4x exec time, Nodes: 185 -> 118
	// 2 -> 3 : 4.07x exec time, Nodes : 2069 -> 695
	// 3 -> 4 : 5.91x exec time, Nodes : 15698 -> 3385
	// 4 -> 5 : 3.7x exec time, Nodes : 145,105 -> 17860
	// 5 -> 6 : 8.57x exec time, Nodes : 975,967 -> 109,626
	// 6 -> 7 : 5.2x exec time, Nodes : 8,752,070 -> 885,138
	// 7 -> 8 : 16.25x exec time, Nodes : 52,415,362 -> 9,807,072
	// 8 -> 9 : 23.08x exec time, Nodes : 438,805,075 -> 259,059,846
	// 9 -> 10 : 40.62x exec time, Nodes : 10,458,246,220
	// Depth 10, Nodes : 10,458,246,220
	// Search (9) = 54996.4 ms
	// Search (10) = 2,233,790 ms = 2233.8 s = 37.23 mins = 0.62 hrs
	// Average (excluding outliers) : 7.552x exec time from n -> n + 1 search depth
	// Node growth : ~7.5 times from n -> n + 1 search depth from (0 - 7)
	// Node growth : ~27 times from n -> n + 1 search depth from (8 - 10)

	// iterative deepening
	for (int current_depth = 1; current_depth <= depth; current_depth++)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		int score = negamax(-50000, 50000, current_depth);
		auto t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> ms_double = t2 - t1;
		std::cout << ms_double.count() << "\n";
		Move move_x = Move(pv_table[0][0]);
		std::string fen;
		move_x.to_fen(fen, 3);
		std::cout << "Best Move : " << fen << " Score : " << score << " Nodes : " << nodes << " Depth : " << depth << "\n";
	}
}

int Board::negamax(int alpha, int beta, int depth)
{
	pv_length[ply] = ply;

	// recursion escape condition
	if (depth == 0)
		// run quiescence search
		return quiescence(alpha, beta);

	//
	if (ply > MAX_PLY - 1)
	{
		return evaluate();
	}

	// increment nodes count
	nodes++;

	// is king in check
	int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) : get_ls1b_index(bitboards[k]),
									  side ^ 1);

	// increase search depth if the king has been exposed into a check
	if (in_check)
		depth++;

	// legal moves counter
	int legal_moves = 0;

	// create move list instance
	moves move_list[1];

	// generate moves
	generate_moves(move_list);

	// for (int i = 0; i < move_list->count; i++)
	// {
	// 	std::cout << move_list->moves[i] << "\n";
	// }

	// sort moves
	sort_moves(move_list);

	// loop over moves within a movelist
	for (int count = 0; count < move_list->count; count++)
	{
		// preserve board state
		copy_board();

		// increment ply
		ply++;

		// make sure to make only legal moves
		if (make_move(move_list->moves[count], all_moves) == 0)
		{
			// decrement ply
			ply--;

			// skip to next move
			continue;
		}

		// increment legal moves
		legal_moves++;

		// score current move
		int score = -negamax(-beta, -alpha, depth - 1);

		// decrement ply
		ply--;

		// take move back
		take_back();

		// fail-hard beta cutoff
		if (score >= beta)
		{
			if (get_move_capture(move_list->moves[count]) == 0)
			{
				killer_moves[1][ply] = killer_moves[0][ply];
				killer_moves[0][ply] = move_list->moves[count];
			}

			// node (move) fails high
			return beta;
		}

		// found a better move
		if (score > alpha)
		{
			if (get_move_capture(move_list->moves[count]) == 0)
			{
				history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;
			}
			// PV node (move)
			alpha = score;

			pv_table[ply][ply] = move_list->moves[count];

			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
			{
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			pv_length[ply] = pv_length[ply + 1];
		}
	}

	// we don't have any legal moves to make in the current postion
	if (legal_moves == 0)
	{
		// king is in check
		if (in_check)
			// return mating score (assuming closest distance to mating position)
			return -49000 + ply;

		// king is not in check
		else
			// return stalemate score
			return 0;
	}

	// node (move) fails low
	return alpha;
}

int Board::score_move(uint64_t move)
{
	// score capture move
	if (get_move_capture(move))
	{
		int target_piece = get_move_capture_piece(move);
		// score move by MVV LVA lookup [source piece][target piece]
		return mvv_lva[get_move_piece(move)][target_piece];
	}

	// score quiet move
	else
	{
		// Score first killer move

		if (killer_moves[0][ply] == move)
		{
			return 90;
		}
		// Score second killer move

		else if (killer_moves[1][ply] == move)
		{
			return 80;
		}
		else
		{
			return history_moves[get_move_piece(move)][get_move_target(move)];
		}
	}

	return 0;
}

int Board::quiescence(int alpha, int beta)
{
	// increment nodes count
	nodes++;

	// evaluate position
	int evaluation = evaluate();

	// fail-hard beta cutoff
	if (evaluation >= beta)
	{
		// node (move) fails high
		return beta;
	}

	int delta = 975;

	if (side == white)
	{
		if (bitboards[0] & row_7)
		{
			delta += 775;
		}
	}
	else
	{
		if (bitboards[6] & row_1)
		{
			delta += 775;
		}
	}

	if (evaluation < alpha - delta)
	{
		return alpha;
	}

	// found a better move
	if (evaluation > alpha)
	{
		// PV node (move)
		alpha = evaluation;
	}

	// create move list instance
	moves move_list[1];

	// generate moves
	generate_captures(move_list);

	// sort moves
	sort_moves(move_list);

	// loop over moves within a movelist
	for (int count = 0; count < move_list->count; count++)
	{

		// preserve board state
		copy_board();

		// increment ply
		ply++;

		// make sure to make only legal moves
		if (make_move(move_list->moves[count], all_moves) == 0)
		{
			// decrement ply
			ply--;

			// skip to next move
			continue;
		}

		// score current move
		int score = -quiescence(-beta, -alpha);

		// decrement ply
		ply--;

		// take move back
		take_back();

		// fail-hard beta cutoff
		if (score >= beta)
		{
			// node (move) fails high
			return beta;
		}

		// found a better move
		if (score > alpha)
		{
			// PV node (move)
			alpha = score;
		}
	}

	// node (move) fails low
	return alpha;
}

int Board::sort_moves(moves *move_list)
{
	// 4550ns on a 20 move list
	// move scores
	int move_scores[move_list->count];
	// score all the moves within a move list
	for (int count = 0; count < move_list->count; count++)
	{
		// score move
		move_scores[count] = score_move(move_list->moves[count]);
	}

	int i = 1;
	while (i < move_list->count)
	{

		int score = move_scores[i];
		uint64_t move = move_list->moves[i];
		int j = i - 1;
		while (j >= 0 && move_scores[j] < score)
		{
			move_scores[j + 1] = move_scores[j];
			move_list->moves[j + 1] = move_list->moves[j];
			j--;
		}
		move_scores[j + 1] = score;
		move_list->moves[j + 1] = move;
		i++;
	}

	// for (int i = 0; i < move_list->count; i++)
	// {
	// 	std::cout << "Move: " << move_list->moves[i] << " Score: " << move_scores[i] << "\n";
	// }
	// exit(0);
	return 1;
}