#include "Board.h"
#include <SDL2/SDL.h>
#include <iostream>

bool Board::populate_move(Move *move)
{
	uint64_t *pieces = (white_to_move ? &white_pieces[0] : &black_pieces[0]);
	uint64_t *opp_pieces = (white_to_move ? &black_pieces[0] : &white_pieces[0]);
	bool found = false;
	move->piece_moved = -1;
	move->piece_captured = -1;
	move->flags = 0;
	for (int i = 0; i < 6; i++)
	{
		if (move->piece_moved == -1 || move->piece_captured == -1)
		{
			if ((*(pieces + i) & (1ULL << move->square_from)) && move->piece_moved == -1)
			{
				move->piece_moved = (white_to_move ? i : i + 6);
				found = true;
			}

			if ((*(opp_pieces + i) & (1ULL << move->square_to)))
			{
				move->piece_captured = (white_to_move ? i + 6 : i);
			}
		}
	}
	if (!found)
	{
		return false;
	}

	int promotion_piece = -1;
	if ((move->piece_moved % 6 == 0) && (move->square_from / 8 == (white_to_move ? 1 : 6)))
	{
		std::cin >> promotion_piece;
		if (promotion_piece == -1)
		{
			promotion_piece = (white_to_move ? 4 : 10);
		}
		move->flags |= Move::PROMOTION;
	}

	move->promotion_piece = promotion_piece;

	if (move->piece_moved % 6 == 5 && ((move->square_to - move->square_from == 2) || move->square_from - move->square_to == 3))
	{
		move->flags |= Move::CASTLE_MOVE;
	}

	if (move->piece_captured == -1 && move->piece_moved % 6 == 0 && move->square_to == en_passant_sq)
	{
		move->piece_captured = (white_to_move ? 6 : 0);
		move->flags |= Move::EN_PASSANT_MOVE;
	}
	return true;
}

bool Board::check_validity(Move *move)
{
	for (auto i = moves.begin(); i != moves.end(); i++)
	{
		Move temp_move = *(&*i);
		if (temp_move == (*move))
		{
			return true;
		}
	}
	return false;
}

void Board::make_move(Move *move)
{
	int square_from = move->square_from;
	int square_to = move->square_to;
	int piece_moved = move->piece_moved;
	uint64_t *pieces = (white_to_move ? white_pieces : black_pieces);
	uint64_t *captured_pieces = (move->piece_captured < 6 ? white_pieces : black_pieces);

	int piece_captured = move->piece_captured;

	pieces[piece_moved] &= ~(1ULL << square_from);
	pieces[piece_moved] |= (1ULL << square_to);
	if (piece_captured != -1)
	{
		captured_pieces[piece_captured % 6] &= ~(1ULL << square_to);
	}

	white_to_move = !white_to_move;
	move_log.push_back(Move::clone(move));
	all_white_pieces = 0;
	all_black_pieces = 0;
	for (int i = 0; i < 6; i++)
	{
		all_white_pieces |= white_pieces[i];
		all_black_pieces |= black_pieces[i];
	}
	all_pieces = all_white_pieces | all_black_pieces;
};

void Board::undo_move()
{
	Move move = move_log.back();
	int square_from = move.square_from;
	int square_to = move.square_to;
	int piece_moved = move.piece_moved;
	uint64_t *pieces = (white_to_move ? white_pieces : black_pieces);
	uint64_t *captured_pieces = (move.piece_captured < 6 ? white_pieces : black_pieces);

	int piece_captured = move.piece_captured;

	pieces[piece_moved] |= (1ULL << square_from);
	pieces[piece_moved] &= ~(1ULL << square_to);
	if (piece_captured != -1)
	{
		captured_pieces[piece_captured] |= (1ULL << square_to);
	}

	white_to_move = !white_to_move;
	if (!move_log.empty())
		move_log.pop_back();
	if (!possible_moves_log.empty())
		possible_moves_log.pop_back();

	moves = possible_moves_log.back();
	all_white_pieces = 0;
	all_black_pieces = 0;
	for (int i = 0; i < 6; i++)
	{
		all_white_pieces |= white_pieces[i];
		all_black_pieces |= black_pieces[i];
	}
	all_pieces = all_white_pieces | all_black_pieces;
}

uint64_t Board::side_attack_squares(bool white)
{
	uint64_t result = 0;

	for (int i = 0; i < 64; i++)
	{
		if (square_under_attack(i, white))
			result |= (1ULL << i);
	}

	return result;
}

bool Board::square_under_attack(int square, bool white)
{

	uint64_t occupancy = all_pieces;

	if (white)
	{
		if (attack_tables_from[3][square] & white_pieces[0])
			return true;
		if (attack_tables_from[1][square] & white_pieces[1])
			return true;
		if (get_bishop_attacks(square, occupancy) & white_pieces[2])
			return true;
		if (get_rook_attacks(square, occupancy) & white_pieces[3])
			return true;
		if (get_queen_attacks(square, occupancy) & white_pieces[4])
			return true;
		if (attack_tables_from[2][square] & white_pieces[5])
			return true;
	}
	else
	{
		if (attack_tables_from[0][square] & black_pieces[0])
			return true;
		if (attack_tables_from[1][square] & black_pieces[1])
			return true;
		if (get_bishop_attacks(square, occupancy) & black_pieces[2])
			return true;
		if (get_rook_attacks(square, occupancy) & black_pieces[3])
			return true;
		if (get_queen_attacks(square, occupancy) & black_pieces[4])
			return true;
		if (attack_tables_from[2][square] & black_pieces[5])
			return true;
	}
	return false;
}

bool Board::in_check()
{
	return (square_under_attack(__builtin_ffsll((white_to_move ? white_pieces[5] : black_pieces[5])) - 1, white_to_move));
}