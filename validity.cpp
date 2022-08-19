#include "Board.h"

bool Board::populate_move(Move &move) {
	uint64_t* pieces = ((side == white) ? &bitboards[0] : &bitboards[6]);
	uint64_t* opp_pieces = ((side == white) ? &bitboards[6] : &bitboards[0]);
	bool	  found = false;
	move.piece = -1;
	move.promoted = 0;
	move.capture_flag = 0;
	move.double_flag = 0;
	move.castle_flag = 0;
	move.en_passant_flag = 0;
	move.move_id = 0;
	move.capture_piece = 12;
	for (int i = 0; i < 6; i++) {
		if (move.piece == -1 || move.capture_flag == 0) {
			if ((*(pieces + i) & (1ULL << move.square_from)) &&
				move.piece == -1) {
				move.piece = ((side == white) ? i : i + 6);
				found = true;
			}

			if ((*(opp_pieces + i) & (1ULL << move.square_to))) {
				move.capture_flag = 1;
				move.capture_piece = (side == white ? i + 6 : i);
			}
		}
	}
	if (!found) {
		return false;
	}

	int promotion_piece = -1;
	if ((move.piece % 6 == 0) &&
		(move.square_from / 8 == ((side == white) ? 1 : 6)))
	{
		if (promotion_piece == -1) {
			promotion_piece = ((side == white) ? 4 : 10);
		}
		move.promoted = promotion_piece;
	}

	if (move.piece % 6 == 5 &&
		((move.square_to - move.square_from == 2) ||
			(move.square_from - move.square_to == 2)))
	{
		move.castle_flag = 1;
	}

	if (move.capture_flag == 0 && move.piece % 6 == 0 &&
		(move.square_to == en_passant_sq))
	{
		move.capture_flag = 1;
		move.en_passant_flag = 1;
		move.capture_piece = (side == white ? 6 : 0);
	}

	if (move.piece % 6 == 0) {
		if (side == white && (move.square_from - move.square_to == 16)) {
			move.double_flag = 1;
		}
		if (side == black && (move.square_to - move.square_from == 16)) {
			move.double_flag = 1;
		}
	}

	move.move_id = ((move.square_from) | (move.square_to << 6) |
		(move.piece << 12) | (move.promoted << 16) | (move.capture_flag << 20) |
		(move.double_flag << 21) | (move.en_passant_flag << 22) |
		(move.castle_flag << 23) | (move.capture_piece << 24));
	return true;
}

int Board::is_square_attacked(int square, int side) {
	if (square < 0 || square > 63) {
		return 0;
	}

	if ((side == white) && (pawn_attacks[black][square] & bitboards[P]))
		return 1;

	// attacked by black pawns
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p]))
		return 1;

	// attacked by knights
	if (knight_attacks[square] &
		((side == white) ? bitboards[N] : bitboards[n]))
		return 1;

	// attacked by bishops
	if (get_bishop_attacks(square, occupancies[both]) &
		((side == white) ? bitboards[B] : bitboards[b]))
		return 1;

	// attacked by rooks
	if (get_rook_attacks(square, occupancies[both]) &
		((side == white) ? bitboards[R] : bitboards[r]))
		return 1;

	// attacked by bishops
	if (get_queen_attacks(square, occupancies[both]) &
		((side == white) ? bitboards[Q] : bitboards[q]))
		return 1;

	// attacked by kings
	if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))
		return 1;

	// by default return false
	return 0;
}

int Board::make_move(uint64_t move, int move_flag) {
	// quiet moves
	if (move_flag == all_moves) {
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
		int capture_piece = get_move_capture_piece(move);

		// move piece
		pop_bit(bitboards[piece], source_square);
		set_bit(bitboards[piece], target_square);
		// Remove zobrist key from source pos
		curr_zobrist_hash ^= zobrist_keys[piece][source_square];
		// Remove zobrist key from target pos
		curr_zobrist_hash ^= zobrist_keys[piece][target_square];

		half_moves++;

		if (piece == P || piece == p) {
			half_moves = 0;
		}

		// handling capture moves
		if (capture) {
			pop_bit(bitboards[capture_piece], target_square);
			curr_zobrist_hash ^= zobrist_keys[capture_piece][target_square];
			half_moves = 0;
		}

		// handle pawn promotions
		if (promoted_piece) {
			// erase the pawn from the target square
			pop_bit(bitboards[(side == white) ? P : p], target_square);
			curr_zobrist_hash ^=
				zobrist_keys[(side == white) ? P : p][target_square];

			// set up promoted piece on chess board
			set_bit(bitboards[promoted_piece], target_square);
			curr_zobrist_hash ^= zobrist_keys[promoted_piece][target_square];
		}

		// handle enpassant captures
		if (enpass) {
			// erase the pawn depending on side to move
			(side == white) ? pop_bit(bitboards[p], target_square + 8)
							: pop_bit(bitboards[P], target_square - 8);
			curr_zobrist_hash ^=
				zobrist_keys[(side == white) ? p : P]
							[(side == white) ? target_square + 8
											 : target_square - 8];
		}

		// reset enpassant square
		en_passant_sq = no_sq;

		// handle double pawn push
		if (double_push) {
			// set enpassant aquare depending on side to move
			(side == white) ? (en_passant_sq = target_square + 8)
							: (en_passant_sq = target_square - 8);
			curr_zobrist_hash ^= en_passant_zobrist[en_passant_sq];
		}

		// handle castling moves
		if (castling) {
			// switch target square
			switch (target_square) {
			// white castles king side
			case (g1):
				// move H rook
				pop_bit(bitboards[R], h1);
				set_bit(bitboards[R], f1);
				curr_zobrist_hash ^= zobrist_keys[R][h1];
				curr_zobrist_hash ^= zobrist_keys[R][f1];
				break;

			// white castles queen side
			case (c1):
				// move A rook
				pop_bit(bitboards[R], a1);
				set_bit(bitboards[R], d1);
				curr_zobrist_hash ^= zobrist_keys[R][a1];
				curr_zobrist_hash ^= zobrist_keys[R][d1];
				break;

			// black castles king side
			case (g8):
				// move H rook
				pop_bit(bitboards[r], h8);
				set_bit(bitboards[r], f8);
				curr_zobrist_hash ^= zobrist_keys[r][h8];
				curr_zobrist_hash ^= zobrist_keys[r][f8];
				break;

			// black castles queen side
			case (c8):
				// move A rook
				pop_bit(bitboards[r], a8);
				set_bit(bitboards[r], d8);
				curr_zobrist_hash ^= zobrist_keys[r][a8];
				curr_zobrist_hash ^= zobrist_keys[r][d8];
				break;
			}
		}

		curr_zobrist_hash ^= castle_zobrist[castle_rights];

		// update castling rights
		castle_rights &= castling_rights[source_square];
		castle_rights &= castling_rights[target_square];

		curr_zobrist_hash ^= castle_zobrist[castle_rights];

		// reset occupancies
		memset(occupancies, 0ULL, 24);

		// loop over white pieces bitboards
		for (int bb_piece = P; bb_piece <= K; bb_piece++) {
			// update white occupancies
			occupancies[white] |= bitboards[bb_piece];

			// update black occupancies
			occupancies[black] |= bitboards[bb_piece + 6];
		}

		occupancies[both] = occupancies[white] | occupancies[black];

		// change side
		side ^= 1;

		curr_zobrist_hash ^= zobrist_side_key;

		// make sure that king has not been exposed into a check
		if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k])
											   : get_ls1b_index(bitboards[K]),
				side))
		{
			// take move back
			take_back();

			// return illegal move
			return 0;
		}

		//
		else {
			// return legal move
			return 1;
		}
	}

	// capture moves
	else {
		// make sure move is a capture
		if (get_move_capture(move)) make_move(move, all_moves);

		// otherwise the move is not a capture
		else
			// don't make it
			return 0;
	}
	return 1;
}

void Board::update_log(uint64_t move) {
	Move move_x = Move(move);
	move_log[move_index] = move;
	std::string pgn = "";

	move_x.to_pgn(pgn, diff_calc(move));
	move_log_pgn[move_index] = pgn;
	std::cout << pgn << "\n";
}

void Board::update_game_state() {
	moves move_list[1];
	generate_moves(move_list);

	int legal_moves = 0;

	for (int i = 0; i < move_list->count; i++) {

		copy_board();

		if (make_move(move_list->moves[i], 0) != 0) {
			legal_moves++;
			take_back();
			break;
		}

		take_back();
	}

	if (legal_moves == 0) {
		if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[K])
											   : get_ls1b_index(bitboards[k]),
				side ^ 1))
		{
			is_checkmate = true;
			return;
		}
		else {
			is_stalemate = true;
			return;
		}
	}
	else {
		is_checkmate = false;
		is_stalemate = false;
	}

	repetition_index++;
	repetition_table[repetition_index] = curr_zobrist_hash;
	return;
}

int Board::diff_calc(uint64_t move) {
	int		 diff = 0;
	int		 piece_moved = get_move_piece(move);
	uint64_t temp_val = bitboards[piece_moved];
	int		 square_from = get_move_source(move);
	int		 rank_diff = -1;
	int		 file_diff = -1;
	while (temp_val) {
		int		 pos = get_ls1b_index(temp_val);
		uint64_t attacks = 0ULL;
		if (pos != square_from) {
			switch (piece_moved % 6) {
			case P:
				if (piece_moved < 6) {
					attacks = pawn_attacks[white][pos];
				}
				else {
					attacks = pawn_attacks[black][pos];
				}
				break;
			case N: attacks = knight_attacks[pos]; break;
			case B: attacks = get_bishop_attacks(pos, occupancies[both]); break;
			case R: attacks = get_rook_attacks(pos, occupancies[both]); break;
			case Q: attacks = get_queen_attacks(pos, occupancies[both]); break;
			case K: attacks = king_attacks[pos]; break;
			default: break;
			}
		}

		if (get_bit(attacks, get_move_target(move))) {
			if ((pos % 8 != square_from) && (pos / 8 == square_from / 8)) {
				file_diff = 1;
			}
			else if ((pos / 8 != square_from / 8) &&
				(pos % 8 == square_from % 8)) {
				rank_diff = 1;
			}
		}

		pop_bit(temp_val, pos);
	}

	if (rank_diff == 1 && file_diff == 1) {
		diff = 3;
	}
	else if (file_diff == 1) {
		diff = 1;
	}
	else if (rank_diff == 1) {
		diff = 2;
	}

	return diff;
}

int Board::check_validity(uint64_t move) {
	moves move_list[1];
	generate_moves(move_list);
	for (int count = 0; count < move_list->count; count++) {
		if (move == move_list->moves[count]) {
			return 1;
		}
	}

	return 0;
}

void Board::legalize_player_moves(
	moves* player_moves, moves* pseudo_legal_moves) {
	player_moves->count = 0;
	for (int i = 0; i < pseudo_legal_moves->count; i++) {
		copy_board();
		if (make_move(pseudo_legal_moves->moves[i], all_moves)) {
			player_moves->moves[player_moves->count] =
				pseudo_legal_moves->moves[i];
			player_moves->count++;
		}
		take_back();
	}
}