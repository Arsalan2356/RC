#include "Board.h"

uint64_t Board::mask_pawn_attacks(int side, int square) {
	// result attacks bitboard
	uint64_t attacks = 0ULL;

	// piece bitboard
	uint64_t bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// white pawns
	if (!side) {
		// generate pawn attacks
		if ((bitboard >> 7) & Board::not_a_file) attacks |= (bitboard >> 7);
		if ((bitboard >> 9) & Board::not_h_file) attacks |= (bitboard >> 9);
	}

	// black pawns
	else {
		// generate pawn attacks
		if ((bitboard << 7) & Board::not_h_file) attacks |= (bitboard << 7);
		if ((bitboard << 9) & Board::not_a_file) attacks |= (bitboard << 9);
	}

	// return attack map
	return attacks;
}

uint64_t Board::mask_knight_attacks(int square) {
	// result attacks bitboard
	uint64_t attacks = 0ULL;

	// piece bitboard
	uint64_t bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// generate knight attacks
	if ((bitboard >> 17) & Board::not_h_file) attacks |= (bitboard >> 17);
	if ((bitboard >> 15) & Board::not_a_file) attacks |= (bitboard >> 15);
	if ((bitboard >> 10) & Board::not_hg_file) attacks |= (bitboard >> 10);
	if ((bitboard >> 6) & Board::not_ab_file) attacks |= (bitboard >> 6);
	if ((bitboard << 17) & Board::not_a_file) attacks |= (bitboard << 17);
	if ((bitboard << 15) & Board::not_h_file) attacks |= (bitboard << 15);
	if ((bitboard << 10) & Board::not_ab_file) attacks |= (bitboard << 10);
	if ((bitboard << 6) & Board::not_hg_file) attacks |= (bitboard << 6);

	// return attack map
	return attacks;
}

uint64_t Board::mask_king_attacks(int square) {
	// result attacks bitboard
	uint64_t attacks = 0ULL;

	// piece bitboard
	uint64_t bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// generate king attacks
	if (bitboard >> 8) attacks |= (bitboard >> 8);
	if ((bitboard >> 9) & Board::not_h_file) attacks |= (bitboard >> 9);
	if ((bitboard >> 7) & Board::not_a_file) attacks |= (bitboard >> 7);
	if ((bitboard >> 1) & Board::not_h_file) attacks |= (bitboard >> 1);
	if (bitboard << 8) attacks |= (bitboard << 8);
	if ((bitboard << 9) & Board::not_a_file) attacks |= (bitboard << 9);
	if ((bitboard << 7) & Board::not_h_file) attacks |= (bitboard << 7);
	if ((bitboard << 1) & Board::not_a_file) attacks |= (bitboard << 1);

	// return attack map
	return attacks;
}

uint64_t Board::mask_bishop_attacks(int square) {
	// result attacks bitboard
	uint64_t attacks = 0ULL;

	// init ranks & files
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant bishop occupancy bits
	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
		attacks |= (1ULL << (r * 8 + f));

	// return attack map
	return attacks;
}

uint64_t Board::mask_rook_attacks(int square) {
	// result attacks bitboard
	uint64_t attacks = 0ULL;

	// init ranks & files
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant rook occupancy bits
	for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
	for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
	for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
	for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

	// return attack map
	return attacks;
}

void Board::compute_attack_tables() {
	for (int square = 0; square < 64; square++) {
		// init pawn attacks
		pawn_attacks[white][square] = mask_pawn_attacks(white, square);
		pawn_attacks[black][square] = mask_pawn_attacks(black, square);

		// init knight attacks
		knight_attacks[square] = mask_knight_attacks(square);

		// init king attacks
		king_attacks[square] = mask_king_attacks(square);
	}
}

void Board::compute_sliding_tables() {
	for (int square = 0; square < 64; square++) {
		// init bishop & rook masks
		bishop_masks[square] = mask_bishop_attacks(square);
		rook_masks[square] = mask_rook_attacks(square);

		// init current mask
		uint64_t attack_mask_bishop = bishop_masks[square];
		uint64_t attack_mask_rook = rook_masks[square];

		// init relevant occupancy bit count
		int relevant_bits_count_bishop =
			__builtin_popcountll(attack_mask_bishop);
		int relevant_bits_count_rook = __builtin_popcountll(attack_mask_rook);

		// init occupancy indicies
		int occupancy_indices = (1 << relevant_bits_count_rook);

		// loop over occupancy indicies
		for (int index = 0; index < occupancy_indices; index++) {
			// init current occupancy variation
			if (index < (1 << relevant_bits_count_bishop)) {
				uint64_t occupancy_bishop = set_occupancy(
					index, relevant_bits_count_bishop, attack_mask_bishop);

				// init magic index
				int magic_index_bishop =
					(occupancy_bishop * bishop_magic_numbers[square]) >>
					(64 - bishop_relevant_bits[square]);

				// init bishop attacks
				bishop_attacks[square][magic_index_bishop] =
					bishop_attacks_on_the_fly(square, occupancy_bishop);
			}

			// init current occupancy variation
			uint64_t occupancy_rook = set_occupancy(
				index, relevant_bits_count_rook, attack_mask_rook);

			// init magic index
			int magic_index_rook =
				(occupancy_rook * rook_magic_numbers[square]) >>
				(64 - rook_relevant_bits[square]);

			// init rook attacks
			rook_attacks[square][magic_index_rook] =
				rook_attacks_on_the_fly(square, occupancy_rook);
		}
	}
}

uint64_t Board::set_occupancy(
	int index, int bits_in_mask, uint64_t attack_mask) {
	// occupancy map
	uint64_t occupancy = 0ULL;

	// loop over the range of bits within attack mask
	for (int count = 0; count < bits_in_mask; count++) {
		// get LS1B index of attacks mask
		int square = Board::get_ls1b_index(attack_mask);

		// pop LS1B in attack map
		pop_bit(attack_mask, square);

		// make sure occupancy is on board
		if (index & (1 << count))
			// populate occupancy map
			occupancy |= (1ULL << square);
	}

	// return occupancy map
	return occupancy;
}

uint64_t Board::get_bishop_attacks(int square, uint64_t occupancy) {
	// get bishop attacks assuming current board occupancy
	occupancy &= bishop_masks[square];
	occupancy *= bishop_magic_numbers[square];
	occupancy >>= 64 - bishop_relevant_bits[square];

	// return bishop attacks
	return bishop_attacks[square][occupancy];
}

uint64_t Board::get_rook_attacks(int square, uint64_t occupancy) {
	// get rook attacks assuming current board occupancy
	occupancy &= rook_masks[square];
	occupancy *= rook_magic_numbers[square];
	occupancy >>= 64 - rook_relevant_bits[square];

	// return rook attacks
	return rook_attacks[square][occupancy];
}

uint64_t Board::get_queen_attacks(int square, uint64_t occupancy) {
	// init result attacks bitboard
	uint64_t queen_attacks = 0ULL;

	queen_attacks = get_bishop_attacks(square, occupancy) |
		get_rook_attacks(square, occupancy);

	// return queen attacks
	return queen_attacks;
}