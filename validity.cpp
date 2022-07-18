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
	move->move_id = 0;
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

	move->move_id = (move->square_from | (move->square_to << 6) | (move->flags << 12) | (move->piece_moved << 15) | ((move->piece_captured + 1) << 19) | ((move->promotion_piece + 1) << 23));
	return true;
}

bool Board::check_validity(uint64_t move_id)
{
	for (auto i = moves.begin(); i != moves.end(); i++)
	{
		uint64_t move = *i;
		if (move == (move_id))
		{
			return true;
		}
	}
	return false;
}

void Board::make_move(uint64_t move_id)
{
	Move move = Move(move_id);
	int square_from = move.square_from;
	int square_to = move.square_to;
	int piece_moved = move.piece_moved;
	uint64_t *pieces = (white_to_move ? white_pieces : black_pieces);
	uint64_t *captured_pieces = (move.piece_captured < 6 ? white_pieces : black_pieces);

	int piece_captured = move.piece_captured;

	pieces[piece_moved] &= ~(1ULL << square_from);
	pieces[piece_moved] |= (1ULL << square_to);
	if (piece_captured != -1)
	{
		captured_pieces[piece_captured % 6] &= ~(1ULL << square_to);
	}

	white_to_move = !white_to_move;
	move_index++;
	for (int i = 0; i < 6; i++)
	{
		board_states[move_index][i] = white_pieces[i];
		board_states[move_index][i + 6] = black_pieces[i];
	}
	board_states[move_index][12] = (white_to_move ? 1 : 0);
	board_states[move_index][13] = castle_rights.to_ulong();
	board_states[move_index][14] = en_passant_sq;

	move_log[move_index] = move.move_id;
	move_log_fen[move_index] = move.to_fen(3);
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
	if (move_index != 0)
	{
		move_log[move_index] = -1;
		move_log_fen[move_index] = "";
		move_index -= 1;
		for (int i = 0; i < 6; i++)
		{
			white_pieces[i] = board_states[move_index][i];
			black_pieces[i] = board_states[move_index][i + 6];
		}
		white_to_move = (board_states[move_index][12] == 1 ? true : false);
		castle_rights = board_states[move_index][13];
		en_passant_sq = board_states[move_index][14];

		moves = possible_moves_log[move_index];
		all_white_pieces = 0;
		all_black_pieces = 0;
		for (int i = 0; i < 6; i++)
		{
			all_white_pieces |= white_pieces[i];
			all_black_pieces |= black_pieces[i];
		}
		all_pieces = all_white_pieces | all_black_pieces;
	}
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

	// What squares white is attacking
	// Check if black is in check here
	if (!white)
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
	// What squares black is attacking
	// Check if white is in check here
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
	return (square_under_attack(__builtin_ffsll((white_to_move ? black_pieces[5] : white_pieces[5])) - 1, !white_to_move));
}