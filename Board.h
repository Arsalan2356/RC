#pragma once
#include <bitset>
#include <SDL2/SDL.h>
#include "Move.h"
#include <vector>

class Board
{
private:
	// Bitset stores positions of each piece of both colors in an array
	// The bits are stored in a way that the left black rook ends up on the first bit
	// So, in terms of squares, that would be A8
	// So, 0 in the bitset maps to square A8 on the board and
	// 63 in the bitset maps to square H1

	// Stores the bitsets of all white pieces
	// The order they are stored in is PNBRQK (0 - 5)
	uint64_t white_pieces[6];
	uint64_t all_white_pieces;
	// Stores the bitsets of all black pieces
	// The order they are stored in is PNBRQK (0 - 5)
	uint64_t black_pieces[6];
	uint64_t all_black_pieces;

	// Stores the bitset that shows all occupied squares
	// This is used as an occupancy for the bishops
	// and rooks
	uint64_t all_pieces;

	// Textures of the pieces, (0 - 5) are the white pieces,
	// (6 - 11) are the black pieces, both are in the order
	// PNBRQK
	SDL_Texture *piece_textures[12];

	// Renderer used to display the pieces
	// and the board
	SDL_Renderer *curr_renderer;

	// Checks whose turn it is to move.
	// True if white's turn and False if
	// black's turn

	// Castle rights are stored in the order they are produced in the FEN
	// 0 - White King Side
	// 1 - White Queen Side
	// 2 - Black King Side
	// 3 - Black Queen Side
	std::bitset<4> castle_rights = {0};

	// Number of half moves since the last pawn advance
	// or piece capture
	int half_moves;

	// Turn number, calculated by getting the length of the movelog
	int turn_number;

	// Attack tables for all pieces
	// 0 for white pieces and 1 for black pieces
	// The to table tells you which positions a piece can attack
	// The from table tells you where a position can be attacked from
	// Each piece has their own to and from table and so does each color
	// To move along a bitboard
	// (-)1 moves you along the board horizontally (left and right) (file +- 1)
	//   -8 moves you one square up (rank - 1)
	//    8 moves you one square down (rank + 1)
	//   -9 moves you one square up and left (rank - 1, file - 1), diagonal
	//    9 moves you one square down and right (rank + 1, file + 1), diagonal
	//   -7 moves you one square up and right (rank - 1, file + 1), diagonal
	//    7 moves you one square down and left (rank + 1, file - 1), diagonal

	// 0 - White Pawn attacks
	// 1 - Knight attacks
	// 2 - King attacks
	// 3 - Black Pawn attacks
	// Other pieces are implemented in their own specific tables
	uint64_t attack_tables_from[4][64] = {0};

	const int bishop_bit_counts[64] = {
		6, 5, 5, 5, 5, 5, 5, 6,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		6, 5, 5, 5, 5, 5, 5, 6};

	const int rook_bit_counts[64] = {
		12, 11, 11, 11, 11, 11, 11, 12,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		12, 11, 11, 11, 11, 11, 11, 12};

	int en_passant_sq = -1;

	std::vector<Move> moves = {};
	std::vector<Move> move_log = {};
	std::vector<Move> move_log_fen = {};

public:
	Board(SDL_Renderer *renderer);
	Board(SDL_Renderer *renderer, const std::string &fen);
	int calculate_flags(Move *move);
	bool check_validity(Move *move);
	bool populate_move(Move *move);
	void make_move(Move *move);
	void undo_move();
	void draw_board(int square_size);
	void draw_pieces(int square_size);
	void compute_attack_tables();
	void compute_sliding_tables();
	uint64_t find_magic_number(int square, int relevant_bits, int flag);
	void init_magic_numbers();
	uint64_t get_magic_number();
	uint64_t random_uint64();
	uint64_t bishop_attacks(int square, uint64_t block);
	uint64_t rook_attacks(int square, uint64_t block);
	uint64_t (Board::*attack_function)(int square, uint64_t block);
	uint64_t set_occupancy(int index, int bits, uint64_t attack_mask);
	uint64_t get_bishop_attacks(int square, uint64_t occupancy);
	uint64_t get_rook_attacks(int square, uint64_t occupancy);
	uint64_t get_queen_attacks(int square, uint64_t occupancy);
	bool square_under_attack(int square, bool white);
	uint64_t side_attack_squares(bool white);
	void generate_all_moves();
	int generate_legal_moves();
	void generate_pawn_moves(bool white);
	void generate_knight_moves(bool white);
	void generate_bishop_moves(bool white, bool is_queen);
	void generate_queen_moves(bool white);
	void generate_rook_moves(bool white, bool is_queen);
	void generate_king_moves(bool white);
	bool in_check();
	int get_piece(int square);
	uint64_t bishop_mask(int sq);
	uint64_t rook_mask(int sq);
	std::vector<std::vector<Move>> possible_moves_log;

	uint64_t bishop_attacks_table[64][512] = {0};
	uint64_t rook_attacks_table[64][4096] = {0};
	uint64_t bishop_magic_numbers[64] = {0};
	uint64_t rook_magic_numbers[64] = {0};
	bool white_to_move;
};