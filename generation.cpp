#include "Board.h"
#include <algorithm>
#include <functional>

bool Board::invalid_move(uint64_t move_id)
{
	bool ret_val = false;
	make_move(move_id);
	if (in_check())
	{
		ret_val = true;
	}
	undo_move();

	return ret_val;
}

int Board::generate_legal_moves()
{
	generate_all_moves();

	std::erase_if(moves, [this](uint64_t move_id)
				  { return this->invalid_move(move_id); });

	for (uint64_t move : moves)
		std::cout << move << "\n";

	possible_moves_log[move_index] = moves;

	if (moves.empty())
	{
		return -1;
	}
	return 1;
}

void Board::generate_all_moves()
{
	if (!moves.empty())
		moves.clear();

	generate_pawn_moves(white_to_move);
	generate_knight_moves(white_to_move);
	generate_bishop_moves(white_to_move, false);
	generate_rook_moves(white_to_move, false);
	generate_queen_moves(white_to_move);
	generate_king_moves(white_to_move);
}

void Board::generate_pawn_moves(bool white)
{
	// All pawn moves

	// Choose correct table to look for pieces depending on color
	uint64_t temp_val = (white ? white_pieces[0] : black_pieces[0]);
	// Initialize bit position for pawns
	int pos = -1;
	signed int double_push_offset = (white ? -16 : 16);
	signed int push_offset = double_push_offset / 2;
	int piece_moved = (white ? 0 : 6);
	int double_push_square = (white ? 6 : 1);
	int promotion_square = (white ? 1 : 6);
	int piece = (white ? 0 : 3);

	while (temp_val != 0)
	{
		// Offset finding the first non-zero bit
		int offset = __builtin_ffsll(temp_val);
		// Adding to position to keep track of bit positions by square
		pos += offset;
		// Check if the pawn is on the correct rank for a double push
		if (((pos / 8) == double_push_square) && (!(all_pieces & (1ULL << (pos + double_push_offset)))) && (!(all_pieces & (1ULL << (pos + push_offset)))))
		{
			moves.push_back(Move::create_id(pos, pos + double_push_offset, 0, piece_moved, -1, -1));
			en_passant_sq = pos + push_offset;
		}
		// Check if a pawn can push one square forward
		if ((!(all_pieces & (1ULL << (pos + push_offset)))) && (white ? pos >= 8 : pos < 56))
		{
			if ((pos / 8) == promotion_square)
			{
				for (int i = 1; i < 5; i++)
				{
					moves.push_back(Move::create_id(pos, pos + push_offset, 0, piece_moved, -1, (white ? i : i + 6)));
				}
			}
			else
			{
				moves.push_back(Move::create_id(pos, pos + push_offset, 0, piece_moved, -1, -1));
			}
		}
		// Pawn captures from specific attack table
		uint64_t temp_attack_table = attack_tables_from[piece][pos];
		// Initializing capture bit position
		int pos_capture = -1;
		while (temp_attack_table != 0)
		{
			// Set capture offset by finding first non-zero bit in capture table
			int capture_offset = __builtin_ffsll(temp_attack_table);
			// Adding to capture bit position to keep track of capture positions by square
			pos_capture += capture_offset;
			// Loop through all opposing pieces to get the piece that's being captured at
			// the current capture bit

			// Get opposite side's pieces by reference
			// This only checks the conditional once instead of 6 times
			uint64_t *opp_piece = (white ? &black_pieces[0] : &white_pieces[0]);
			for (int i = 0; i < 6; i++)
			{
				if (*(opp_piece + i) & (1ULL << pos_capture))
				{
					moves.push_back(Move::create_id(pos, pos_capture, 0, piece_moved, (white ? i + 6 : i), -1));
					break;
				}
			}
			temp_attack_table >>= capture_offset;
		}
		// En Passant
		uint64_t opp_pawns = (white ? black_pieces[0] : white_pieces[0]);
		if ((en_passant_sq != -1) && ((1ULL << en_passant_sq) & attack_tables_from[piece][pos]) && ((1ULL << (en_passant_sq + (white ? -8 : 8))) & opp_pawns))
		{
			moves.push_back(Move::create_id(pos, pos_capture, Move::EN_PASSANT_MOVE, piece_moved, (white ? 6 : 0), -1));
		}

		temp_val >>= offset;
	}
}

void Board::generate_knight_moves(bool white)
{

	uint64_t temp_val = (white ? white_pieces[1] : black_pieces[1]);
	int pos = -1;
	int piece_moved = (white ? 1 : 7);
	while (temp_val != 0)
	{
		int offset = __builtin_ffsll(temp_val);
		pos += offset;
		uint64_t temp_attack_table = attack_tables_from[1][pos];
		int move_pos = -1;
		while (temp_attack_table != 0)
		{
			int move_offset = __builtin_ffsll(temp_attack_table);
			move_pos += move_offset;
			bool capture = false;
			uint64_t *opp_piece = (white ? &black_pieces[0] : &white_pieces[0]);
			for (int i = 0; i < 6; i++)
			{
				if ((1ULL << move_pos) & *(opp_piece + i))
				{
					moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, (white ? i + 6 : i), -1));
					capture = true;
					break;
				}
			}
			if (!(capture) && (!((1ULL << move_pos) & all_pieces)))
			{
				moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, -1, -1));
			}
			temp_attack_table >>= move_offset;
		}
		temp_val >>= offset;
	}
}

void Board::generate_bishop_moves(bool white, bool is_queen)
{
	uint64_t occupancy = all_pieces;
	uint64_t temp_val = (is_queen ? (white ? white_pieces[4] : black_pieces[4]) : (white ? white_pieces[2] : black_pieces[2]));
	int pos = -1;
	int piece_moved = (is_queen ? (white ? 4 : 10) : (white ? 2 : 8));
	while (temp_val != 0)
	{
		int offset = __builtin_ffsll(temp_val);
		pos += offset;
		uint64_t temp_attack_table = get_bishop_attacks(pos, occupancy);
		int move_pos = -1;
		while (temp_attack_table != 0)
		{
			int move_offset = __builtin_ffsll(temp_attack_table);
			move_pos += move_offset;
			bool capture = false;
			uint64_t *opp_piece = (white ? &black_pieces[0] : &white_pieces[0]);
			for (int i = 0; i < 6; i++)
			{
				if ((1ULL << move_pos) & *(opp_piece + i))
				{
					capture = true;
					moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, (white ? i : i + 6), -1));
					break;
				}
			}
			if (!(capture) && (!((1ULL << move_pos) & all_pieces)))
				moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, -1, -1));

			temp_attack_table >>= move_offset;
		}
		temp_val >>= offset;
	}
}

void Board::generate_rook_moves(bool white, bool is_queen)
{
	uint64_t occupancy = all_pieces;
	uint64_t temp_val = (is_queen ? (white ? white_pieces[4] : black_pieces[4]) : (white ? white_pieces[3] : black_pieces[3]));
	int pos = -1;
	int piece_moved = (is_queen ? (white ? 4 : 10) : (white ? 3 : 9));
	while (temp_val != 0)
	{
		int offset = __builtin_ffsll(temp_val);
		pos += offset;
		uint64_t temp_attack_table = get_rook_attacks(pos, occupancy);
		int move_pos = -1;
		while (temp_attack_table != 0)
		{
			int move_offset = __builtin_ffsll(temp_attack_table);
			move_pos += move_offset;
			bool capture = false;
			uint64_t *opp_piece = (white ? &black_pieces[0] : &white_pieces[0]);
			for (int i = 0; i < 6; i++)
			{
				if ((1ULL << move_pos) & *(opp_piece + i))
				{
					capture = true;
					moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, (white ? i : i + 6), -1));
					break;
				}
			}
			if (!(capture) && (!((1ULL << move_pos) & all_pieces)))
				moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, -1, -1));

			temp_attack_table >>= move_offset;
		}
		temp_val >>= offset;
	}
}

void Board::generate_queen_moves(bool white)
{
	generate_bishop_moves(white, true);
	generate_rook_moves(white, true);
}

void Board::generate_king_moves(bool white)
{
	uint64_t temp_val = (white ? white_pieces[5] : black_pieces[5]);
	int pos = -1;
	int piece_moved = (white ? 5 : 11);
	bool castle_king_side = (white ? castle_rights[0] : castle_rights[2]);
	bool castle_queen_side = (white ? castle_rights[1] : castle_rights[3]);

	while (temp_val != 0)
	{
		int offset = __builtin_ffsll(temp_val);
		pos += offset;
		uint64_t temp_attack_table = attack_tables_from[2][pos];
		int move_pos = -1;
		while (temp_attack_table != 0)
		{
			int move_offset = __builtin_ffsll(temp_attack_table);
			move_pos += move_offset;
			bool capture = false;
			uint64_t *opp_piece = (white ? &black_pieces[0] : &white_pieces[0]);
			for (int i = 0; i < 6; i++)
			{
				if ((1ULL << move_pos) & *(opp_piece + i))
				{
					moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, (white ? i + 6 : i), -1));
					capture = true;
					break;
				}
			}
			if (!(capture) && (!((1ULL << move_pos) & all_pieces)))
			{
				moves.push_back(Move::create_id(pos, move_pos, 0, piece_moved, -1, -1));
			}
			temp_attack_table >>= move_offset;
		}

		if (castle_king_side && (all_pieces & (1ULL << (pos + 1))) && (all_pieces & (1ULL << (pos + 2))))
		{
			moves.push_back(Move::create_id(pos, pos + 2, Move::CASTLE_MOVE, piece_moved, -1, -1));
		}
		if (castle_queen_side && (all_pieces & (1ULL << (pos - 1))) && (all_pieces & (1ULL << (pos - 2))) && (all_pieces & (1ULL << (pos - 3))))
		{
			moves.push_back(Move::create_id(pos, pos - 3, Move::CASTLE_MOVE, piece_moved, -1, -1));
		}

		temp_val >>= offset;
	}
}

uint64_t Board::get_bishop_attacks(int square, uint64_t occupancy)
{
	occupancy &= bishop_mask(square);
	occupancy *= bishop_magic_numbers[square];
	occupancy >>= 64 - bishop_bit_counts[square];

	return bishop_attacks_table[square][occupancy];
}

uint64_t Board::get_rook_attacks(int square, uint64_t occupancy)
{
	occupancy &= rook_mask(square);
	occupancy *= rook_magic_numbers[square];
	occupancy >>= (64 - rook_bit_counts[square]);

	return rook_attacks_table[square][occupancy];
}

uint64_t Board::get_queen_attacks(int square, uint64_t occupancy)
{
	return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}
