#include "Board.h"

void Board::generate_moves(moves *move_list)
{
	// init move count
	move_list->count = 0;

	// define source & target squares
	int source_square, target_square;

	// define current piece's bitboard copy & it's attacks
	uint64_t bitboard, attacks;

	// loop over all the bitboards
	for (int piece = P; piece <= k; piece++)
	{
		// init piece bitboard copy
		bitboard = bitboards[piece];

		// generate white pawns & white king castling moves
		if (side == white)
		{
			// pick up white pawn bitboards index
			if (piece == P)
			{
				// loop over white pawns within white pawn bitboard
				while (bitboard)
				{
					// init source square
					source_square = get_ls1b_index(bitboard);

					// init target square
					target_square = source_square - 8;

					// generate quiet pawn moves
					if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
					{
						// pawn promotion
						if (source_square >= a7 && source_square <= h7)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, Q, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, R, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, B, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, N, 0, 0, 0, 0, 12));
						}

						else
						{
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, 12));

							// two squares ahead pawn move
							if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
								add_move(move_list, Move::create_id(source_square, target_square - 8, piece, 0, 0, 1, 0, 0, 12));
						}
					}

					// init pawn attacks bitboard
					attacks = pawn_attacks[side][source_square] & occupancies[black];

					// generate pawn captures
					while (attacks)
					{
						// init target square
						target_square = get_ls1b_index(attacks);

						int target_piece = 12;

						for (int bb_piece = 0; bb_piece < 6; bb_piece++)
						{
							int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
							// if there's a piece on the target square
							if (get_bit(bitboards[piece_pos], target_square))
							{
								// remove it from corresponding bitboard
								target_piece = piece_pos;
								break;
							}
						}
						// pawn promotion
						if (source_square >= a7 && source_square <= h7)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, Q, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, R, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, B, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, N, 1, 0, 0, 0, target_piece));
						}

						else
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

						// pop ls1b of the pawn attacks
						pop_bit(attacks, target_square);
					}

					// generate enpassant captures
					if (en_passant_sq != no_sq)
					{
						// lookup pawn attacks and bitwise AND with enpassant square (bit)
						uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_sq);

						// make sure enpassant capture available
						if (enpassant_attacks)
						{
							// init enpassant capture target square
							int target_enpassant = get_ls1b_index(enpassant_attacks);
							add_move(move_list, Move::create_id(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, (side == white ? 6 : 0)));
						}
					}

					// pop ls1b from piece bitboard copy
					pop_bit(bitboard, source_square);
				}
			}

			// castling moves
			if (piece == K)
			{
				// king side castling is available
				if (castle_rights & wk)
				{
					// make sure square between king and king's rook are empty
					if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
					{
						// make sure king and the f1 squares are not under attacks
						if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
							add_move(move_list, Move::create_id(e1, g1, piece, 0, 0, 0, 0, 1, 12));
					}
				}

				// queen side castling is available
				if (castle_rights & wq)
				{
					// make sure square between king and queen's rook are empty
					if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
					{
						// make sure king and the d1 squares are not under attacks
						if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
							add_move(move_list, Move::create_id(e1, c1, piece, 0, 0, 0, 0, 1, 12));
					}
				}
			}
		}

		// generate black pawns & black king castling moves
		else
		{
			// pick up black pawn bitboards index
			if (piece == p)
			{
				// loop over white pawns within white pawn bitboard
				while (bitboard)
				{
					// init source square
					source_square = get_ls1b_index(bitboard);

					// init target square
					target_square = source_square + 8;

					// generate quiet pawn moves
					if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
					{
						// pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, q, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, r, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, b, 0, 0, 0, 0, 12));
							add_move(move_list, Move::create_id(source_square, target_square, piece, n, 0, 0, 0, 0, 12));
						}

						else
						{
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, 12));

							// two squares ahead pawn move
							if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
								add_move(move_list, Move::create_id(source_square, target_square + 8, piece, 0, 0, 1, 0, 0, 12));
						}
					}

					// init pawn attacks bitboard
					attacks = pawn_attacks[side][source_square] & occupancies[white];

					// generate pawn captures
					while (attacks)
					{
						// init target square
						target_square = get_ls1b_index(attacks);

						int target_piece = 12;

						for (int bb_piece = 0; bb_piece < 6; bb_piece++)
						{
							int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
							// if there's a piece on the target square
							if (get_bit(bitboards[piece_pos], target_square))
							{
								// remove it from corresponding bitboard
								target_piece = piece_pos;
								break;
							}
						}

						// pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, q, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, r, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, b, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, n, 1, 0, 0, 0, target_piece));
						}

						else
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

						// pop ls1b of the pawn attacks
						pop_bit(attacks, target_square);
					}

					// generate enpassant captures
					if (en_passant_sq != no_sq)
					{
						// lookup pawn attacks and bitwise AND with enpassant square (bit)
						uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_sq);

						// make sure enpassant capture available
						if (enpassant_attacks)
						{
							// init enpassant capture target square
							int target_enpassant = get_ls1b_index(enpassant_attacks);
							add_move(move_list, Move::create_id(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, (side == white ? 6 : 0)));
						}
					}

					// pop ls1b from piece bitboard copy
					pop_bit(bitboard, source_square);
				}
			}

			// castling moves
			if (piece == k)
			{
				// king side castling is available
				if (castle_rights & bk)
				{
					// make sure square between king and king's rook are empty
					if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
					{
						// make sure king and the f8 squares are not under attacks
						if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
							add_move(move_list, Move::create_id(e8, g8, piece, 0, 0, 0, 0, 1, 12));
					}
				}

				// queen side castling is available
				if (castle_rights & bq)
				{
					// make sure square between king and queen's rook are empty
					if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
					{
						// make sure king and the d8 squares are not under attacks
						if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
							add_move(move_list, Move::create_id(e8, c8, piece, 0, 0, 0, 0, 1, 12));
					}
				}
			}
		}

		// generate knight moves
		if ((side == white) ? piece == N : piece == n)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}

					// quiet move
					if (target_piece == 12)
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, target_piece));

					else
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate bishop moves
		if ((side == white) ? piece == B : piece == b)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}
					// quiet move
					if (target_piece == 12)
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, target_piece));

					else
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate rook moves
		if ((side == white) ? piece == R : piece == r)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}
					// quiet move
					if (target_piece == 12)
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, target_piece));

					else
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate queen moves
		if ((side == white) ? piece == Q : piece == q)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}

					// quiet move
					if (target_piece == 12)
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, target_piece));

					else
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate king moves
		if ((side == white) ? piece == K : piece == k)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}

					// quiet move
					if (target_piece == 12)
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 0, 0, 0, 0, target_piece));

					else
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}
	}
}

void Board::add_move(moves *move_list, int move)
{
	// store move
	move_list->moves[move_list->count] = move;

	// increment move count
	move_list->count++;
}

void Board::generate_captures(moves *move_list)
{
	// init move count
	move_list->count = 0;

	// define source & target squares
	int source_square, target_square;

	// define current piece's bitboard copy & it's attacks
	uint64_t bitboard, attacks;

	// loop over all the bitboards
	for (int piece = P; piece <= k; piece++)
	{
		// init piece bitboard copy
		bitboard = bitboards[piece];

		// generate white pawns & white king castling moves
		if (side == white)
		{
			// pick up white pawn bitboards index
			if (piece == P)
			{
				// loop over white pawns within white pawn bitboard
				while (bitboard)
				{
					// init source square
					source_square = get_ls1b_index(bitboard);

					// init target square
					target_square = source_square - 8;

					// init pawn attacks bitboard
					attacks = pawn_attacks[side][source_square] & occupancies[black];

					// generate pawn captures
					while (attacks)
					{
						// init target square
						target_square = get_ls1b_index(attacks);

						int target_piece = 12;

						for (int bb_piece = 0; bb_piece < 6; bb_piece++)
						{
							int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
							// if there's a piece on the target square
							if (get_bit(bitboards[piece_pos], target_square))
							{
								// remove it from corresponding bitboard
								target_piece = piece_pos;
								break;
							}
						}

						// pawn promotion
						if (source_square >= a7 && source_square <= h7)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, Q, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, R, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, B, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, N, 1, 0, 0, 0, target_piece));
						}

						else
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

						// pop ls1b of the pawn attacks
						pop_bit(attacks, target_square);
					}

					// generate enpassant captures
					if (en_passant_sq != no_sq)
					{
						// lookup pawn attacks and bitwise AND with enpassant square (bit)
						uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_sq);

						// make sure enpassant capture available
						if (enpassant_attacks)
						{
							// init enpassant capture target square
							int target_enpassant = get_ls1b_index(enpassant_attacks);
							add_move(move_list, Move::create_id(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, (side == white ? 6 : 0)));
						}
					}

					// pop ls1b from piece bitboard copy
					pop_bit(bitboard, source_square);
				}
			}
		}

		// generate black pawns & black king castling moves
		else
		{
			// pick up black pawn bitboards index
			if (piece == p)
			{
				// loop over white pawns within white pawn bitboard
				while (bitboard)
				{
					// init source square
					source_square = get_ls1b_index(bitboard);

					// init target square
					target_square = source_square + 8;

					// init pawn attacks bitboard
					attacks = pawn_attacks[side][source_square] & occupancies[white];

					// generate pawn captures
					while (attacks)
					{
						// init target square
						target_square = get_ls1b_index(attacks);

						int target_piece = 12;

						for (int bb_piece = 0; bb_piece < 6; bb_piece++)
						{
							int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
							// if there's a piece on the target square
							if (get_bit(bitboards[piece_pos], target_square))
							{
								// remove it from corresponding bitboard
								target_piece = piece_pos;
								break;
							}
						}

						// pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
							add_move(move_list, Move::create_id(source_square, target_square, piece, q, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, r, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, b, 1, 0, 0, 0, target_piece));
							add_move(move_list, Move::create_id(source_square, target_square, piece, n, 1, 0, 0, 0, target_piece));
						}

						else
							// one square ahead pawn move
							add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

						// pop ls1b of the pawn attacks
						pop_bit(attacks, target_square);
					}

					// generate enpassant captures
					if (en_passant_sq != no_sq)
					{
						// lookup pawn attacks and bitwise AND with enpassant square (bit)
						uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_sq);

						// make sure enpassant capture available
						if (enpassant_attacks)
						{
							// init enpassant capture target square
							int target_enpassant = get_ls1b_index(enpassant_attacks);
							add_move(move_list, Move::create_id(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, (side == white ? 6 : 0)));
						}
					}

					// pop ls1b from piece bitboard copy
					pop_bit(bitboard, source_square);
				}
			}
		}

		// genarate knight moves
		if ((side == white) ? piece == N : piece == n)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}
					// quiet move
					if (get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate bishop moves
		if ((side == white) ? piece == B : piece == b)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}
					// quiet move
					if (get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate rook moves
		if ((side == white) ? piece == R : piece == r)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}
					// quiet move
					if (get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
						// capture move
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate queen moves
		if ((side == white) ? piece == Q : piece == q)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}

					// captures
					if (get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}

		// generate king moves
		if ((side == white) ? piece == K : piece == k)
		{
			// loop over source squares of piece bitboard copy
			while (bitboard)
			{
				// init source square
				source_square = get_ls1b_index(bitboard);

				// init piece attacks in order to get set of target squares
				attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

				// loop over target squares available from generated attacks
				while (attacks)
				{
					// init target square
					target_square = get_ls1b_index(attacks);

					int target_piece = 12;

					for (int bb_piece = 0; bb_piece < 6; bb_piece++)
					{
						int piece_pos = (side == white ? bb_piece + 6 : bb_piece);
						// if there's a piece on the target square
						if (get_bit(bitboards[piece_pos], target_square))
						{
							// remove it from corresponding bitboard
							target_piece = piece_pos;
							break;
						}
					}

					// quiet move
					if (get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
						add_move(move_list, Move::create_id(source_square, target_square, piece, 0, 1, 0, 0, 0, target_piece));

					// pop ls1b in current attacks set
					pop_bit(attacks, target_square);
				}

				// pop ls1b of the current piece bitboard copy
				pop_bit(bitboard, source_square);
			}
		}
	}
}