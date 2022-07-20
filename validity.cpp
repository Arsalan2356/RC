#include "Board.h"

bool Board::populate_move(Move *move)
{
	uint64_t *pieces = ((side == white) ? &bitboards[0] : &bitboards[6]);
	uint64_t *opp_pieces = ((side == white) ? &bitboards[6] : &bitboards[0]);
	bool found = false;
	move->piece = -1;
	move->move_id = 0;
	move->capture_flag = 0;
	for (int i = 0; i < 6; i++)
	{
		if (move->piece == -1 || move->capture_flag == 0)
		{
			if ((*(pieces + i) & (1ULL << move->square_from)) && move->piece == -1)
			{
				move->piece = ((side == white) ? i : i + 6);
				found = true;
			}

			if ((*(opp_pieces + i) & (1ULL << move->square_to)))
			{
				move->capture_flag = 1;
			}
		}
	}
	if (!found)
	{
		return false;
	}

	int promotion_piece = -1;
	if ((move->piece % 6 == 0) && (move->square_from / 8 == ((side == white) ? 1 : 6)))
	{
		if (promotion_piece == -1)
		{
			promotion_piece = ((side == white) ? 4 : 10);
		}
		move->promoted = promotion_piece;
	}

	if (move->piece % 6 == 5 && ((move->square_to - move->square_from == 2) || move->square_from - move->square_to == 2))
	{
		move->castle_flag = 1;
	}

	if (move->capture_flag == 0 && move->piece % 6 == 0 && move->square_to == en_passant_sq)
	{
		move->capture_flag = 1;
		move->en_passant_flag = 1;
	}

	move->move_id = ((move->square_from) | (move->square_to << 6) | (move->piece << 12) | (move->promoted << 16) | (move->capture_flag << 20) | (move->double_flag << 21) | (move->en_passant_flag << 22) | (move->castle_flag << 23));
	return true;
}

int Board::is_square_attacked(int square, int side)
{
	if ((side == white) && (pawn_attacks[black][square] & bitboards[P]))
		return 1;

	// attacked by black pawns
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p]))
		return 1;

	// attacked by knights
	if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n]))
		return 1;

	// attacked by bishops
	if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]))
		return 1;

	// attacked by rooks
	if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]))
		return 1;

	// attacked by bishops
	if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]))
		return 1;

	// attacked by kings
	if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))
		return 1;

	// by default return false
	return 0;
}

int Board::make_move(uint64_t move, int move_flag)
{
	// quiet moves
	if (move_flag == all_moves)
	{
		// preserve board state
		copy_board();

		// parse move
		int source_square = get_move_source(move);
		int target_square = get_move_target(move);
		int piece = get_move_piece(move);
		int promoted_piece = get_move_promoted(move);
		int capture = get_move_capture(move);
		int double_push = get_move_double(move);
		int enpass = get_move_enpassant(move);
		int castling = get_move_castling(move);

		// move piece
		pop_bit(bitboards[piece], source_square);
		set_bit(bitboards[piece], target_square);

		// handling capture moves
		if (capture)
		{
			// pick up bitboard piece index ranges depending on side
			int start_piece, end_piece;

			// white to move
			if (side == white)
			{
				start_piece = p;
				end_piece = k;
			}

			// black to move
			else
			{
				start_piece = P;
				end_piece = K;
			}

			// loop over bitboards opposite to the current side to move
			for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
			{
				// if there's a piece on the target square
				if (get_bit(bitboards[bb_piece], target_square))
				{
					// remove it from corresponding bitboard
					pop_bit(bitboards[bb_piece], target_square);
					break;
				}
			}
		}

		// handle pawn promotions
		if (promoted_piece)
		{
			// erase the pawn from the target square
			pop_bit(bitboards[(side == white) ? P : p], target_square);

			// set up promoted piece on chess board
			set_bit(bitboards[promoted_piece], target_square);
		}

		// handle enpassant captures
		if (enpass)
		{
			// erase the pawn depending on side to move
			(side == white) ? pop_bit(bitboards[p], target_square + 8) : pop_bit(bitboards[P], target_square - 8);
		}

		// reset enpassant square
		en_passant_sq = no_sq;

		// handle double pawn push
		if (double_push)
		{
			// set enpassant aquare depending on side to move
			(side == white) ? (en_passant_sq = target_square + 8) : (en_passant_sq = target_square - 8);
		}

		// handle castling moves
		if (castling)
		{
			// switch target square
			switch (target_square)
			{
			// white castles king side
			case (g1):
				// move H rook
				pop_bit(bitboards[R], h1);
				set_bit(bitboards[R], f1);
				break;

			// white castles queen side
			case (c1):
				// move A rook
				pop_bit(bitboards[R], a1);
				set_bit(bitboards[R], d1);
				break;

			// black castles king side
			case (g8):
				// move H rook
				pop_bit(bitboards[r], h8);
				set_bit(bitboards[r], f8);
				break;

			// black castles queen side
			case (c8):
				// move A rook
				pop_bit(bitboards[r], a8);
				set_bit(bitboards[r], d8);
				break;
			}
		}

		// update castling rights
		castle_rights &= castling_rights[source_square];
		castle_rights &= castling_rights[target_square];

		// reset occupancies
		memset(occupancies, 0ULL, 24);

		// loop over white pieces bitboards
		for (int bb_piece = P; bb_piece <= K; bb_piece++)
			// update white occupancies
			occupancies[white] |= bitboards[bb_piece];

		// loop over black pieces bitboards
		for (int bb_piece = p; bb_piece <= k; bb_piece++)
			// update black occupancies
			occupancies[black] |= bitboards[bb_piece];

		// update both sides occupancies
		occupancies[both] |= occupancies[white];
		occupancies[both] |= occupancies[black];

		// change side
		side ^= 1;

		// make sure that king has not been exposed into a check
		if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side))
		{
			// take move back
			take_back();

			// return illegal move
			return 0;
		}

		//
		else
			// return legal move
			return 1;
	}

	// capture moves
	else
	{
		// make sure move is the capture
		if (get_move_capture(move))
			make_move(move, all_moves);

		// otherwise the move is not a capture
		else
			// don't make it
			return 0;
	}
	return 1;
}