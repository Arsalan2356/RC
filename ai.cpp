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
	// find best move within a given position
	int score = negamax(-50000, 50000, depth);
	if (best_move)
	{
		Move move_x = Move(best_move);
		std::string fen;
		move_x.to_fen(fen, 3);
		std::cout << "Best Move : " << fen << " Score : " << score << " Nodes : " << nodes << " Depth :" << depth << "\n";
	}
}

int Board::negamax(int alpha, int beta, int depth)
{
	// recursion escape condition
	if (depth == 0)
		// run quiescence search
		return quiescence(alpha, beta);

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

	// best move so far
	int best_sofar;

	// old value of alpha
	int old_alpha = alpha;

	// create move list instance
	moves move_list[1];

	// generate moves
	generate_moves(move_list);

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
			// node (move) fails high
			return beta;
		}

		// found a better move
		if (score > alpha)
		{
			// PV node (move)
			alpha = score;

			// if root move
			if (ply == 0)
				// associate best move with the best score
				best_sofar = move_list->moves[count];
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

	// found better move
	if (old_alpha != alpha)
		// init best move
		best_move = best_sofar;

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
	// TODO
	// Change to a faster/more efficient sorting algorithm
	// This one currently takes
	// 500ns on a 30 move list
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

	return 1;
}