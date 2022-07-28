#include "Board.h"

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

		int temp_mg_score = mg_score;

		// loop over pieces within a bitboard
		while (bitboard)
		{
			// init piece
			piece = bb_piece;

			int num_pawns;

			// init square
			square = get_ls1b_index(bitboard);
			switch (piece)
			{
			case P:
				num_pawns = count_bits(bitboards[P] & file_masks[square]);

				if (num_pawns > 1)
				{
					mg_score += num_pawns * double_pawn_penalty;

					eg_score += num_pawns * double_pawn_penalty;
				}

				if (bitboards[P] & isolated_pawn_masks[square] == 0)
				{
					mg_score += isolated_pawn_penalty;
					eg_score += isolated_pawn_penalty;
				}
				else
				{
					mg_score += count_bits(bitboards[P] & isolated_pawn_masks[square]) * (-isolated_pawn_penalty);
					eg_score += 1.1 * count_bits(bitboards[P] & isolated_pawn_masks[square]) * (-isolated_pawn_penalty);
				}

				if (white_passed_pawn_masks[square] & bitboards[p] == 0)
				{
					mg_score += passed_pawn_bonus[7 - (square / 8)];
					eg_score += (passed_pawn_bonus[7 - (square / 8)] * 1.2);
				}
				break;
			case N:
				break;
			case B:
				mg_score += count_bits(get_bishop_attacks(square, occupancies[both]));
				eg_score += count_bits(get_bishop_attacks(square, occupancies[both])) * 1.1;
				break;
			case R:

				if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
				{
					mg_score += open_file_score;
					eg_score += open_file_score;
				}
				else if (bitboards[P] & file_masks[square] == 0)
				{
					mg_score += semi_open_file_score;
					eg_score += semi_open_file_score;
				}

				break;
			case Q:
				mg_score += count_bits(get_queen_attacks(square, occupancies[both]));
				eg_score += count_bits(get_queen_attacks(square, occupancies[both])) * 1.1;
				break;
			case K:
				if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
				{
					mg_score += open_file_score;
					eg_score += open_file_score;
				}

				if (bitboards[P] & file_masks[square] == 0)
				{
					mg_score += semi_open_file_score;
					eg_score += semi_open_file_score;
				}
				break;
			case p:
				num_pawns = count_bits(bitboards[p] & file_masks[square]);

				if (num_pawns > 1)
				{
					mg_score += num_pawns * double_pawn_penalty;

					eg_score += num_pawns * double_pawn_penalty;
				}

				if (bitboards[p] & isolated_pawn_masks[square] == 0)
				{
					mg_score += isolated_pawn_penalty;
					eg_score += isolated_pawn_penalty;
				}
				else
				{
					mg_score += count_bits(bitboards[p] & isolated_pawn_masks[square]) * (-isolated_pawn_penalty);
					eg_score += 1.1 * count_bits(bitboards[p] & isolated_pawn_masks[square]) * (-isolated_pawn_penalty);
				}

				if (black_passed_pawn_masks[square] & bitboards[P] == 0)
				{
					mg_score += passed_pawn_bonus[7 - (square / 8)];
					eg_score += (passed_pawn_bonus[7 - (square / 8)] * 1.2);
				}
				break;
			case n:
				break;
			case b:
				mg_score += count_bits(get_bishop_attacks(square, occupancies[both]));
				eg_score += count_bits(get_bishop_attacks(square, occupancies[both])) * 1.1;
				break;
			case r:
				if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
				{
					mg_score += open_file_score;
					eg_score += open_file_score;
				}
				else if (bitboards[p] & file_masks[square] == 0)
				{
					mg_score += semi_open_file_score;
					eg_score += semi_open_file_score;
				}
				break;
			case q:
				mg_score += count_bits(get_queen_attacks(square, occupancies[both]));
				eg_score += count_bits(get_queen_attacks(square, occupancies[both])) * 1.1;
				break;
			case k:
				if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
				{
					mg_score += open_file_score;
					eg_score += open_file_score;
				}

				if (bitboards[p] & file_masks[square] == 0)
				{
					mg_score += semi_open_file_score;
					eg_score += semi_open_file_score;
				}
				break;
			default:
				break;
			}

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
				mg_score += mg_piece_scores[piece - 6][FLIP(square)];
				eg_score += eg_piece_scores[piece - 6][FLIP(square)];
			}

			phase -= phase_inc[piece % 6];

			// pop ls1b
			pop_bit(bitboard, square);
		}
	}

	int curr_phase = (phase * 256 + (24 / 2)) / 24;
	int score = ((mg_score * (256 - curr_phase)) + (eg_score * curr_phase)) / 256;

	return (side == white ? score : -score);
}

__attribute__((hot)) uint64_t Board::search_position(int depth)
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

		score = negamax(alpha, beta, current_depth);

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

int Board::negamax(int alpha, int beta, int depth)
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

	int static_eval = evaluate();

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
		int score = -negamax(-beta, -beta + 1, depth - 1 - reduce);

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
				new_score = quiescence(alpha, beta);

				return (new_score > score) ? new_score : score;
			}
		}

		score += 175;

		if (score < beta && depth <= 2)
		{
			new_score = quiescence(alpha, beta);

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

			score = -negamax(-beta, -alpha, depth - 1);
		}
		else
		{
			if (moves_searched >= full_depth_moves && ply >= reduction_limit && in_check == 0 && get_move_capture(move_list->moves[count]) == 0 && get_move_promoted(move_list->moves[count]) == 0)
			{
				score = -negamax(-alpha - 1, -alpha, depth - 2);
			}
			else
			{
				score = alpha + 1;
			}

			if (score > alpha)
			{
				score = -negamax(-alpha - 1, -alpha, depth - 1);
				if ((score > alpha) && (score < beta))
				{
					score = -negamax(-beta, -alpha, depth - 1);
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

int Board::score_move(uint64_t move)
{
	if (score_pv)
	{
		if (pv_table[0][ply] == move)
		{
			score_pv = 0;
			return 900;
		}
	}

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

	if (ply > MAX_PLY - 1)
	{
		return evaluate();
	}

	// evaluate position
	int evaluation = evaluate();

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
		int score = -quiescence(-beta, -alpha);

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

int Board::sort_moves(moves *move_list, uint64_t best_move)
{
	// 4550ns on a 20 move list
	// move scores
	int move_scores[move_list->count];
	// score all the moves within a move list
	for (int count = 0; count < move_list->count; count++)
	{
		// score move
		if (best_move == move_list->moves[count])
		{
			move_scores[count] = 1200;
		}
		else
		{
			move_scores[count] = score_move(move_list->moves[count]);
		}
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

void Board::enable_pv(moves *move_list)
{
	follow_pv = 0;
	for (int count = 0; count < move_list->count; count++)
	{
		if (pv_table[0][ply] == move_list->moves[count])
		{
			score_pv = 1;
			follow_pv = 1;
		}
	}
}

void Board::reset_hashes()
{
	for (int i = 0; i < hash_size; i++)
	{
		tt_table[i].key = 0;
		tt_table[i].depth = 0;
		tt_table[i].flags = 0;
		tt_table[i].value = 0;
		tt_table[i].best_move = 0;
	}
}

int Board::read_hash_entry(int alpha, int beta, int depth, uint64_t *best_move)
{
	hash *hash_entry = &tt_table[curr_zobrist_hash % hash_size];

	if (hash_entry->key == curr_zobrist_hash)
	{
		if (hash_entry->depth >= depth)
		{
			int value = hash_entry->value;

			if (value < -48000)
			{
				value += ply;
			}
			else if (value > 48000)
			{
				value -= ply;
			}

			if (hash_entry->flags == hash_flag_exact)
			{
				return value;
			}
			else if (hash_entry->flags == hash_flag_alpha && value <= alpha)
			{
				return alpha;
			}
			else if (hash_entry->flags == hash_flag_beta && value >= beta)
			{
				return beta;
			}
		}

		*best_move = hash_entry->best_move;
	}

	return no_hash_found;
}

void Board::set_entry(int value, int depth, int flags, uint64_t best_move)
{
	hash *hash_entry = &tt_table[curr_zobrist_hash % hash_size];

	if (value < -48000)
	{
		value -= ply;
	}
	else if (value > 48000)
	{
		value += ply;
	}

	hash_entry->key = curr_zobrist_hash;
	hash_entry->depth = depth;
	hash_entry->flags = flags;
	hash_entry->value = value;
	hash_entry->best_move = best_move;
}

int Board::is_repetition()
{
	for (int i = 0; i < repetition_index; i++)
	{
		if (repetition_table[i] == curr_zobrist_hash)
		{
			return 1;
		}
	}
	return 0;
}

uint64_t Board::set_file_rank_mask(int file, int rank)
{
	uint64_t mask = 0ULL;

	for (int r = 0; r < 8; r++)
	{
		for (int f = 0; f < 8; f++)
		{
			int sq = r * 8 + f;
			if (f == file)
			{
				mask |= (1ULL << sq);
			}
			else if (r == rank)
			{
				mask |= (1ULL << sq);
			}
		}
	}

	return mask;
}

void Board::init_evaluation_masks()
{
	for (int r = 0; r < 8; r++)
	{
		for (int f = 0; f < 8; f++)
		{
			int sq = r * 8 + f;
			file_masks[sq] |= set_file_rank_mask(f, -1);
			rank_masks[sq] |= set_file_rank_mask(-1, r);

			isolated_pawn_masks[sq] |= set_file_rank_mask(f + 1, -1);
			isolated_pawn_masks[sq] |= set_file_rank_mask(f - 1, -1);

			white_passed_pawn_masks[sq] |= set_file_rank_mask(f - 1, -1);
			white_passed_pawn_masks[sq] |= set_file_rank_mask(f, -1);
			white_passed_pawn_masks[sq] |= set_file_rank_mask(f + 1, -1);

			for (int i = 0; i < (8 - r); i++)
			{
				white_passed_pawn_masks[sq] &= ~rank_masks[(7 - i) * 8 + f];
			}

			black_passed_pawn_masks[sq] |= set_file_rank_mask(f - 1, -1);
			black_passed_pawn_masks[sq] |= set_file_rank_mask(f, -1);
			black_passed_pawn_masks[sq] |= set_file_rank_mask(f + 1, -1);

			for (int j = 0; j < r + 1; j++)
			{
				white_passed_pawn_masks[sq] &= ~rank_masks[j * 8 + f];
			}
		}
	}
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