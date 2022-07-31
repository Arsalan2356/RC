#include "Board.h"

__attribute__((hot)) uint64_t Board::search_position_nnue(int depth)
{
	if (is_checkmate)
	{
		return 0;
	}

	// Clear helper data structures for search and nodes
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));
	nodes = 0ULL;

	follow_pv = 0;
	score_pv = 0;

	// Search (1) = 0.0303 ms, Nodes 21 -> 24 -> 24
	// n -> n + 1 : mx exec time, Nodes : old_nodes -> new_nodes
	// n is the depth, m is the time ratio of exec time from the previous time,
	// old_nodes is the number of nodes without pvs search, new_nodes is the
	// number of nodes with pvs search
	// 1 -> 2 : 0.71x exec time, Nodes: 185 -> 118 -> 227 -> 88, 0.0972
	// 2 -> 3 : 1.38x exec time, Nodes : 2,069 -> 695 -> 1,074 -> 675, 0.4144
	// 3 -> 4 : 1.34x exec time, Nodes : 15,698 -> 3,385 -> 2,155 -> 3102, 1.6299
	// 4 -> 5 : 10.2x exec time, Nodes : 145,105 -> 17,860 -> 20,323 -> 10424, 5.7277
	// 5 -> 6 : 4.02x exec time, Nodes : 975,967 -> 109,626 -> 134,527 -> 29911, 16.9431
	// 6 -> 7 : 6.63x exec time, Nodes : 8,752,070 -> 885,138 -> 601,644 -> 72657, 43.0369
	// 7 -> 8 : 4.48x exec time, Nodes : 52,415,362 -> 9,807,072 -> 3,389,330 -> 287702, 172.391
	// 8 -> 9 : 4.45x exec time, Nodes : 438,805,075 -> 259,059,846 -> 17,674,933 -> 2558589, 1443.59
	// 9 -> 10 : 4.01x exec time, Nodes : 10,458,246,220 -> 30,307,907 -> 10807362, 6007.48
	// 10 -> 11 : nx exec time, Nodes : 159356873, 85289.2
	// Depth 10, Nodes : 10,458,246,220 -> 30,307,907 -> 10807362
	// Search (9) = 54,996.4 ms -> 7,665.84 ms -> 1443.59 ms
	// Search (10) = 2,233,790 ms = 2233.8 s = 37.23 mins = 0.62 hrs -> 31,177 ms -> 6007.48
	// Average (excluding outliers) : ~5x exec time from n -> n + 1 search depth
	// Node growth : ~5 times from n -> n + 1

	auto t1 = std::chrono::high_resolution_clock::now();

	int alpha = -50000;
	int beta = 50000;

	int score = 0;

	// iterative deepening
	for (int current_depth = 1; current_depth <= depth; current_depth++)
	{
		// Enable follow_pv flag
		follow_pv = 1;

		score = negamax_nnue(alpha, beta, current_depth);

		if ((score <= alpha) || (score >= beta))
		{
			alpha = -50000;
			beta = 50000;
			continue;
		}

		alpha = score - 50;
		beta = score + 50;
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << "\n";

	Move move_x = Move(pv_table[0][0]);
	std::string pgn = "";
	move_x.to_pgn(pgn, diff_calc(pv_table[0][0]));
	if (score > -49000 && score < -48000)
	{
		std::cout << "Best Move : " << pgn << "\n"
				  << "Eval : Mate in " << (score + 49000) / 2 - 1 << "\n"
				  << "Nodes : " << nodes << "\n"
				  << "Depth : " << depth << "\n";
	}
	else if (score > 48000 && score < 49000)
	{
		std::cout << "Best Move : " << pgn << "\n"
				  << "Eval : Mate in " << (49000 - score) / 2 + 1 << "\n"
				  << "Nodes : " << nodes << "\n"
				  << "Depth : " << depth << "\n";
	}
	else
	{
		std::cout << "Best Move : " << pgn << "\n"
				  << "Eval : " << score << "\n"
				  << "Nodes : " << nodes << "\n"
				  << "Depth : " << depth << "\n";
	}

	return pv_table[0][0];
}

int Board::negamax_nnue(int alpha, int beta, int depth)
{
	pv_length[ply] = ply;

	// Static evaluation
	int score = 0;

	int hash_flag = hash_flag_alpha;

	uint64_t best_move = 0;

	if (ply && is_repetition() || half_moves > 50)
	{
		return 0;
	}

	int pv_node = (beta - alpha > 1);

	score = read_hash_entry(alpha, beta, depth, &best_move);

	if (ply && score != no_hash_found && (!pv_node))
	{
		return score;
	}

	// recursion escape condition
	if (depth == 0)
		// run quiescence_nnue search
		return quiescence_nnue(alpha, beta);

	//
	if (ply > MAX_PLY - 1)
	{
		return nnue_eval();
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

	int static_eval = nnue_eval();

	if (depth < 3 && !pv_node && !in_check && (abs(beta - 1) > -50000 + 100))
	{
		int eval_margin = 120 * depth;

		if (static_eval - eval_margin >= beta)
		{
			return (static_eval - eval_margin);
		}
	}

	// Null Move Pruning
	if (depth >= 3 && in_check == 0 && ply && !null_move_made)
	{
		copy_board();

		side ^= 1;

		curr_zobrist_hash ^= zobrist_side_key;

		if (en_passant_sq != no_sq)
		{
			curr_zobrist_hash ^= en_passant_zobrist[en_passant_sq];
		}

		en_passant_sq = no_sq;

		ply++;

		repetition_index++;
		repetition_table[repetition_index] = curr_zobrist_hash;

		null_move_made = true;
		int score = -negamax_nnue(-beta, -beta + 1, depth - 1 - reduce);

		ply--;

		repetition_index--;

		take_back();

		if (score >= beta)
		{
			return beta;
		}
	}
	else
	{
		if (null_move_made)
		{
			null_move_made = false;
		}
	}

	if (!pv_node && !in_check && depth <= 3)
	{
		score = static_eval + 125;

		int new_score = 0;

		if (score < beta)
		{
			if (depth == 1)
			{
				new_score = quiescence_nnue(alpha, beta);

				return (new_score > score) ? new_score : score;
			}
		}

		score += 175;

		if (score < beta && depth <= 2)
		{
			new_score = quiescence_nnue(alpha, beta);

			if (new_score < beta)
			{
				return (new_score > score) ? new_score : score;
			}
		}
	}

	// create move list instance
	moves move_list[1];

	// generate moves
	generate_moves(move_list);

	// for (int i = 0; i < move_list->count; i++)
	// {
	// 	std::cout << move_list->moves[i] << "\n";
	// }

	if (follow_pv)
	{
		enable_pv(move_list);
	}

	// sort moves
	sort_moves(move_list, best_move);

	int moves_searched = 0;

	// loop over moves within a movelist
	for (int count = 0; count < move_list->count; count++)
	{
		// preserve board state
		copy_board();

		// increment ply
		ply++;

		repetition_index++;
		repetition_table[repetition_index] = curr_zobrist_hash;

		// make sure to make only legal moves
		if (make_move(move_list->moves[count], all_moves) == 0)
		{
			// decrement ply
			ply--;

			repetition_index--;
			// skip to next move
			continue;
		}

		// increment legal moves
		legal_moves++;

		if (moves_searched == 0)
		{

			score = -negamax_nnue(-beta, -alpha, depth - 1);
		}
		else
		{
			if (moves_searched >= full_depth_moves && ply >= reduction_limit && in_check == 0 && get_move_capture(move_list->moves[count]) == 0 && get_move_promoted(move_list->moves[count]) == 0)
			{
				score = -negamax_nnue(-alpha - 1, -alpha, depth - 2);
			}
			else
			{
				score = alpha + 1;
			}

			if (score > alpha)
			{
				score = -negamax_nnue(-alpha - 1, -alpha, depth - 1);
				if ((score > alpha) && (score < beta))
				{
					score = -negamax_nnue(-beta, -alpha, depth - 1);
				}
			}
		}

		// decrement ply
		ply--;

		repetition_index--;
		// take move back
		take_back();

		moves_searched++;

		// found a better move
		if (score > alpha)
		{

			hash_flag = hash_flag_exact;

			best_move = move_list->moves[count];

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

			// fail-hard beta cutoff
			if (score >= beta)
			{

				set_entry(beta, depth, hash_flag_beta, best_move);

				if (get_move_capture(move_list->moves[count]) == 0)
				{
					killer_moves[1][ply] = killer_moves[0][ply];
					killer_moves[0][ply] = move_list->moves[count];
				}

				// node (move) fails high
				return beta;
			}
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

	set_entry(alpha, depth, hash_flag, best_move);

	// node (move) fails low
	return alpha;
}

int Board::quiescence_nnue(int alpha, int beta)
{
	// increment nodes count
	nodes++;

	if (ply > MAX_PLY - 1)
	{
		return nnue_eval();
	}

	// NNUE evaluate position
	int evaluation = nnue_eval();

	// fail-hard beta cutoff
	if (evaluation >= beta)
	{
		// node (move) fails high
		return beta;
	}

	// Delta Pruning
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

	if (evaluation > alpha)
	{
		alpha = evaluation;
	}

	// create move list instance
	moves move_list[1];

	// generate moves
	generate_captures(move_list);

	// sort moves
	sort_moves(move_list, 0);

	// loop over moves within a movelist
	for (int count = 0; count < move_list->count; count++)
	{

		// preserve board state
		copy_board();

		// increment ply
		ply++;

		repetition_index++;
		repetition_table[repetition_index] = curr_zobrist_hash;

		// make sure to make only legal moves
		if (make_move(move_list->moves[count], all_moves) == 0)
		{
			// decrement ply
			ply--;

			repetition_index--;
			// skip to next move
			continue;
		}

		// score current move
		int score = -quiescence_nnue(-beta, -alpha);

		// decrement ply
		ply--;

		repetition_index--;

		// take move back
		take_back();

		// found a better move
		if (score > alpha)
		{
			// PV node (move)
			alpha = score;
			// fail-hard beta cutoff
			if (score >= beta)
			{
				// node (move) fails high
				return beta;
			}
		}
	}

	// node (move) fails low
	return alpha;
}

int Board::nnue_eval()
{
	uint64_t bitboard;

	int piece, square;

	int pieces[33];

	int squares[33];

	int index = 2;

	for (int bb_piece = P; bb_piece <= k; bb_piece++)
	{
		bitboard = bitboards[bb_piece];

		while (bitboard)
		{
			piece = bb_piece;

			square = get_ls1b_index(bitboard);

			if (piece == K)
			{
				pieces[0] = nnue_pieces[piece];
				squares[0] = nnue_squares[square];
			}
			else if (piece == k)
			{
				pieces[1] = nnue_pieces[piece];
				squares[1] = nnue_squares[square];
			}

			else
			{
				pieces[index] = nnue_pieces[piece];
				squares[index] = nnue_squares[square];
				index++;
			}

			pop_bit(bitboard, square);
		}
	}

	pieces[index] = 0;
	squares[index] = 0;

	return (evaluate_nnue(side, pieces, squares) * (100 - half_moves) / 100);
}

void Board::enable_nnue(bool use_nnue)
{
	this->use_nnue = use_nnue;
}
