#pragma once
#include "Move.h"
#include "nnue_eval.h"

#include <chrono>
#include <cstring>
#include <vector>


#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define copy_board()                                                           \
	uint64_t bitboards_copy[12], occupancies_copy[3], curr_zobrist_hash_copy;  \
	int		 side_copy, en_passant_copy, castle_copy, half_moves_copy;         \
	memcpy(bitboards_copy, bitboards, 96);                                     \
	memcpy(occupancies_copy, occupancies, 24);                                 \
	side_copy = side, en_passant_copy = en_passant_sq,                         \
	castle_copy = castle_rights;                                               \
	curr_zobrist_hash_copy = curr_zobrist_hash;                                \
	half_moves_copy = half_moves;

// restore board state
#define take_back()                                                            \
	memcpy(bitboards, bitboards_copy, 96);                                     \
	memcpy(occupancies, occupancies_copy, 24);                                 \
	side = side_copy, en_passant_sq = en_passant_copy,                         \
	castle_rights = castle_copy;                                               \
	curr_zobrist_hash = curr_zobrist_hash_copy;                                \
	half_moves = half_moves_copy;

// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) (move & 0x100000)

// extract double pawn push flag
#define get_move_double(move) (move & 0x200000)

// extract enpassant flag
#define get_move_enpassant(move) (move & 0x400000)

// extract castling flag
#define get_move_castling(move) (move & 0x800000)

// extract capture piece flag
#define get_move_capture_piece(move) ((move & 0xf000000) >> 24)

// Flip square to the other side
#define FLIP(sq) ((sq) ^ 56)

#define MAX_PLY 192

#define hash_flag_exact 0

#define hash_flag_alpha 1

#define hash_flag_beta 2

#define hash_size 0xa000000

#define no_hash_found 100000

std::vector<std::string> split(std::string &fen);

typedef struct {
	// moves
	int moves[256];

	// move count
	int count;
} moves;
class Board
{
	public:
	// Constructor for board
	Board(std::string &fen, bool nnue);

	// Init functions for the constructor
	void init(std::string &fen);
	void init_tables(bool nnue);
	void init_magic_numbers();

	// Populates moves that the user inputs
	bool populate_move(Move &move);

	// Checks the validity of a move
	int check_validity(uint64_t move);

	// Makes a move on the current board
	int make_move(uint64_t move, int move_flag);

	// Updates the move log and the fen notations
	void update_log(uint64_t move);

	// This function is only called when a move is made by the user
	// or when the computer finally decides on the best move
	// This is not called in the search functions
	void update_game_state();

	// Mask attacks for each piece before computing them
	uint64_t mask_pawn_attacks(int side, int square);
	uint64_t mask_knight_attacks(int square);
	uint64_t mask_bishop_attacks(int sq);
	uint64_t mask_rook_attacks(int sq);
	uint64_t mask_king_attacks(int square);

	// Computes attack tables for all pieces
	void	 compute_attack_tables();
	void	 compute_sliding_tables();
	uint64_t set_occupancy(int index, int bits, uint64_t attack_mask);

	// Magic numbers generation functions
	uint64_t find_magic_number(int square, int relevant_bits, int flag);
	uint64_t get_magic_number();
	uint64_t random_uint64();
	uint64_t bishop_attacks_on_the_fly(int square, uint64_t block);
	uint64_t rook_attacks_on_the_fly(int square, uint64_t block);

	// This is a pointer to a method assigned in the find magic function
	uint64_t (Board::*attack_function)(int square, uint64_t block);

	// Hardcoded magic numbers for bishops and rooks
	uint64_t bishop_magic_numbers[64] = { 0x40040844404084ULL,
		0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
		0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL,
		0x3c0808410220200ULL, 0x4050404440404ULL, 0x21001420088ULL,
		0x24d0080801082102ULL, 0x1020a0a020400ULL, 0x40308200402ULL,
		0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
		0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL,
		0x4002801a02003ULL, 0x85040820080400ULL, 0x810102c808880400ULL,
		0xe900410884800ULL, 0x8002020480840102ULL, 0x220200865090201ULL,
		0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
		0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL,
		0x1c004001012080ULL, 0x8004200962a00220ULL, 0x8422100208500202ULL,
		0x2000402200300c08ULL, 0x8646020080080080ULL, 0x80020a0200100808ULL,
		0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
		0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL,
		0x1840060a44020800ULL, 0x90080104000041ULL, 0x201011000808101ULL,
		0x1a2208080504f080ULL, 0x8012020600211212ULL, 0x500861011240000ULL,
		0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
		0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL,
		0x4010801011c04ULL, 0xa010109502200ULL, 0x4a02012000ULL,
		0x500201010098b028ULL, 0x8040002811040900ULL, 0x28000010020204ULL,
		0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL };
	uint64_t rook_magic_numbers[64] = { 0x8a80104000800020ULL,
		0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
		0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL,
		0x2080088004402900ULL, 0x800098204000ULL, 0x2024401000200040ULL,
		0x100802000801000ULL, 0x120800800801000ULL, 0x208808088000400ULL,
		0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
		0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL,
		0x140848010000802ULL, 0x481828014002800ULL, 0x8094004002004100ULL,
		0x4010040010010802ULL, 0x20008806104ULL, 0x100400080208000ULL,
		0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
		0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL,
		0x80004600042881ULL, 0x4040008040800020ULL, 0x440003000200801ULL,
		0x4200011004500ULL, 0x188020010100100ULL, 0x14800401802800ULL,
		0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
		0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL,
		0x100021010009ULL, 0x2002080100110004ULL, 0x202008004008002ULL,
		0x20020004010100ULL, 0x2048440040820001ULL, 0x101002200408200ULL,
		0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
		0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL,
		0x44440041009200ULL, 0x280001040802101ULL, 0x2100190040002085ULL,
		0x80c0084100102001ULL, 0x4024081001000421ULL, 0x20030a0244872ULL,
		0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL };

	// Gets attacks for each of the sliding pieces
	uint64_t get_bishop_attacks(int square, uint64_t occupancy);
	uint64_t get_rook_attacks(int square, uint64_t occupancy);
	uint64_t get_queen_attacks(int square, uint64_t occupancy);

	// Checks if a square is attacked by the opposite side
	int is_square_attacked(int square, int side);

	// Move generation functions
	void generate_moves(moves* move_list);
	void generate_captures(moves* move_list);
	void add_move(moves* move_list, int move);

	// Perft (Performance Test) Functions
	uint64_t nodes = 0ULL;
	void	 perft(int depth);
	void	 perft_depth(int depth);
	void	 perft_divide(int depth);

	// Side whose turn it is to move
	int side;
	enum { white, black, both };

	static int get_ls1b_index(uint64_t board);
	static int count_bits(uint64_t board);

	// Square names to numbers for positions for readability
	enum {
		a8,
		b8,
		c8,
		d8,
		e8,
		f8,
		g8,
		h8,
		a7,
		b7,
		c7,
		d7,
		e7,
		f7,
		g7,
		h7,
		a6,
		b6,
		c6,
		d6,
		e6,
		f6,
		g6,
		h6,
		a5,
		b5,
		c5,
		d5,
		e5,
		f5,
		g5,
		h5,
		a4,
		b4,
		c4,
		d4,
		e4,
		f4,
		g4,
		h4,
		a3,
		b3,
		c3,
		d3,
		e3,
		f3,
		g3,
		h3,
		a2,
		b2,
		c2,
		d2,
		e2,
		f2,
		g2,
		h2,
		a1,
		b1,
		c1,
		d1,
		e1,
		f1,
		g1,
		h1,
		no_sq
	};

	// Array to calculate new castling rights
	// after each move
	const int castling_rights[64] = { 7, 15, 15, 15, 3, 15, 15, 11, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14

	};

	// Enum for pieces
	// White is (0 - 5)
	// Black is (6 - 11)
	enum { P, N, B, R, Q, K, p, n, b, r, q, k };

	// Enum for whether to check for all moves or captures
	enum { all_moves, only_captures };

	// Bitboards for all pieces
	// Same order as the enum
	uint64_t bitboards[12];

	// Occupancies for both sides
	// Same order as the side enums
	uint64_t occupancies[3];

	// Castle rights are stored in the order they are produced in the FEN
	// 1 - White King Side
	// 2 - White Queen Side
	// 4 - Black King Side
	// 8 - Black Queen Side
	int castle_rights = 0;
	enum { wk = 1, wq = 2, bk = 4, bq = 8 };

	// Number of half moves since the last pawn advance
	// or piece capture
	int half_moves;

	// Move index
	int move_index = 0;

	uint64_t pawn_attacks[2][64];

	// knight attacks table [square]
	uint64_t knight_attacks[64];

	// king attacks table [square]
	uint64_t king_attacks[64];

	// bishop attack masks
	uint64_t bishop_masks[64];

	// rook attack masks
	uint64_t rook_masks[64];

	// bishop attacks table [square][occupancies]
	uint64_t bishop_attacks[64][512];

	// rook attacks rable [square][occupancies]
	uint64_t rook_attacks[64][4096];

	// Relevant bits for bishops
	// These are the possible moves from each position
	const int bishop_relevant_bits[64] = {
		6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6

	};

	// Relevant bits for rooks
	// These are the possible moves from each position
	const int rook_relevant_bits[64] = { 12, 11, 11, 11, 11, 11, 11, 12, 11, 10,
		10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
		10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10,
		10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12

	};

	int en_passant_sq = no_sq;

	// Move log and Move log in pgn notation
	uint64_t	move_log[512] = { 0 };
	std::string move_log_pgn[512];
	// Disambiguating moves for pgn notation
	int diff_calc(uint64_t move);

	// not-A file constant
	static const uint64_t not_a_file = 18374403900871474942ULL;

	// not-H file constant
	static const uint64_t not_h_file = 9187201950435737471ULL;

	// not-HG file constant
	static const uint64_t not_hg_file = 4557430888798830399ULL;

	// not-AB file constant
	static const uint64_t not_ab_file = 18229723555195321596ULL;

	// 7th row constant
	static const uint64_t row_7 = 71776119061217280ULL;

	// 1st row constant
	static const uint64_t row_1 = 65280ULL;

	// Evaluation function
	int evaluate();

	// Midgame and Endgame values for each piece
	int mg_value[12] = { 82, 337, 365, 477, 1025, 0, -82, -337, -365, -477,
		-1025, 0 };
	int eg_value[12] = { 94, 281, 297, 512, 936, 0, -94, -281, -297, -512, -936,
		0 };

	// Phase increments for each piece disregarding color
	int phase_inc[6] = { 0, 1, 1, 2, 4, 0 };

	// Pawn positions table mid game and endgame
	int mg_pawn_table[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,				  //
		98, 134, 61, 95, 68, 126, 34, -11,	  //
		-6, 7, 26, 31, 65, 56, 25, -20,		  //
		-14, 13, 6, 21, 23, 12, 17, -23,	  //
		-27, -2, -5, 12, 17, 6, 10, -25,	  //
		-26, -4, -4, -10, 3, 3, 33, -12,	  //
		-35, -1, -20, -23, -15, 24, 38, -22,  //
		0, 0, 0, 0, 0, 0, 0, 0,				  //
	};

	int eg_pawn_table[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,					 //
		178, 173, 158, 134, 147, 132, 165, 187,	 //
		94, 100, 85, 67, 56, 53, 82, 84,		 //
		32, 24, 13, 5, -2, 4, 17, 17,			 //
		13, 9, -3, -7, -7, -8, 3, -1,			 //
		4, 7, -6, 1, 0, -5, -1, -8,				 //
		13, 8, 8, 10, 13, 0, 2, -7,				 //
		0, 0, 0, 0, 0, 0, 0, 0,					 //
	};

	// Knight positions table mid game and endgame
	int mg_knight_table[64] = {
		-167, -89, -34, -49, 61, -97, -15, -107,  //
		-73, -41, 72, 36, 23, 62, 7, -17,		  //
		-47, 60, 37, 65, 84, 129, 73, 44,		  //
		-9, 17, 19, 53, 37, 69, 18, 22,			  //
		-13, 4, 16, 13, 28, 19, 21, -8,			  //
		-23, -9, 12, 10, 19, 17, 25, -16,		  //
		-29, -53, -12, -3, -1, 18, -14, -19,	  //
		-105, -21, -58, -33, -17, -28, -19, -23,  //
	};

	int eg_knight_table[64] = {
		-58, -38, -13, -28, -31, -27, -63, -99,	 //
		-25, -8, -25, -2, -9, -25, -24, -52,	 //
		-24, -20, 10, 9, -1, -9, -19, -41,		 //
		-17, 3, 22, 22, 22, 11, 8, -18,			 //
		-18, -6, 16, 25, 16, 17, 4, -18,		 //
		-23, -3, -1, 15, 10, -3, -20, -22,		 //
		-42, -20, -10, -5, -2, -20, -23, -44,	 //
		-29, -51, -23, -15, -22, -18, -50, -64,	 //
	};

	// Bishop positions table mid game and endgame
	int mg_bishop_table[64] = {
		-29, 4, -82, -37, -25, -42, 7, -8,		//
		-26, 16, -18, -13, 30, 59, 18, -47,		//
		-16, 37, 43, 40, 35, 50, 37, -2,		//
		-4, 5, 19, 50, 37, 37, 7, -2,			//
		-6, 13, 13, 26, 34, 12, 10, 4,			//
		0, 15, 15, 15, 14, 27, 18, 10,			//
		4, 15, 16, 0, 7, 21, 33, 1,				//
		-33, -3, -14, -21, -13, -12, -39, -21,	//
	};

	int eg_bishop_table[64] = {
		-14, -21, -11, -8, -7, -9, -17, -24,  //
		-8, -4, 7, -12, -3, -13, -4, -14,	  //
		2, -8, 0, -1, -2, 6, 0, 4,			  //
		-3, 9, 12, 9, 14, 10, 3, 2,			  //
		-6, 3, 13, 19, 7, 10, -3, -9,		  //
		-12, -3, 8, 10, 13, 3, -7, -15,		  //
		-14, -18, -7, -1, 4, -9, -15, -27,	  //
		-23, -9, -23, -5, -9, -16, -5, -17,	  //
	};

	// Rook positions table mid game and endgame
	int mg_rook_table[64] = {
		32, 42, 32, 51, 63, 9, 31, 43,		 //
		27, 32, 58, 62, 80, 67, 26, 44,		 //
		-5, 19, 26, 36, 17, 45, 61, 16,		 //
		-24, -11, 7, 26, 24, 35, -8, -20,	 //
		-36, -26, -12, -1, 9, -7, 6, -23,	 //
		-45, -25, -16, -17, 3, 0, -5, -33,	 //
		-44, -16, -20, -9, -1, 11, -6, -71,	 //
		-19, -13, 1, 17, 16, 7, -37, -26,	 //
	};

	int eg_rook_table[64] = {
		13, 10, 18, 15, 12, 12, 8, 5,	  //
		11, 13, 13, 11, -3, 3, 8, 3,	  //
		7, 7, 7, 5, 4, -3, -5, -3,		  //
		4, 3, 13, 1, 2, 1, -1, 2,		  //
		3, 5, 8, 4, -5, -6, -8, -11,	  //
		-4, 0, -5, -1, -7, -12, -8, -16,  //
		-6, -6, 0, 2, -9, -9, -11, -3,	  //
		-9, 2, 3, -1, -5, -13, 4, -20,	  //
	};

	// Queen positions table mid game and endgame
	int mg_queen_table[64] = {
		-28, 0, 29, 12, 59, 44, 43, 45,		  //
		-24, -39, -5, 1, -16, 57, 28, 54,	  //
		-13, -17, 7, 8, 29, 56, 47, 57,		  //
		-27, -27, -16, -16, -1, 17, -2, 1,	  //
		-9, -26, -9, -10, -2, -4, 3, -3,	  //
		-14, 2, -11, -2, -5, 2, 14, 5,		  //
		-35, -8, 11, 2, 8, 15, -3, 1,		  //
		-1, -18, -9, 10, -15, -25, -31, -50,  //
	};

	int eg_queen_table[64] = {
		-9, 22, 22, 27, 27, 19, 10, 20,			 //
		-17, 20, 32, 41, 58, 25, 30, 0,			 //
		-20, 6, 9, 49, 47, 35, 19, 9,			 //
		3, 22, 24, 45, 57, 40, 57, 36,			 //
		-18, 28, 19, 47, 31, 34, 39, 23,		 //
		-16, -27, 15, 6, 9, 17, 10, 5,			 //
		-22, -23, -30, -16, -16, -23, -36, -32,	 //
		-33, -28, -22, -43, -5, -32, -20, -41,	 //
	};

	// King positions table mid game and endgame
	int mg_king_table[64] = {
		-65, 23, 16, -15, -56, -34, 2, 13,		 //
		29, -1, -20, -7, -8, -4, -38, -29,		 //
		-9, 24, 2, -16, -20, 6, 22, -22,		 //
		-17, -20, -12, -27, -30, -25, -14, -36,	 //
		-49, -1, -27, -39, -46, -44, -33, -51,	 //
		-14, -14, -22, -46, -44, -30, -15, -27,	 //
		1, 7, -8, -64, -43, -16, 9, 8,			 //
		-15, 36, 12, -54, 8, -28, 24, 14,		 //
	};

	int eg_king_table[64] = {
		-74, -35, -18, -18, -11, 15, 4, -17,	//
		-12, 17, 14, 17, 17, 38, 23, 11,		//
		10, 17, 23, 15, 20, 45, 44, 13,			//
		-8, 22, 24, 27, 26, 33, 26, 3,			//
		-18, -4, 21, 24, 27, 23, 9, -11,		//
		-19, -3, 11, 21, 23, 16, 7, -9,			//
		-27, -11, 4, 13, 14, 4, -5, -17,		//
		-53, -34, -21, -11, -28, -14, -24, -43	//
	};

	// All midgame piece scores
	int* mg_piece_scores[6] = { mg_pawn_table, mg_knight_table, mg_bishop_table,
		mg_rook_table, mg_queen_table, mg_king_table

	};

	// All endgame piece scores
	int* eg_piece_scores[6] = { eg_pawn_table, eg_knight_table, eg_bishop_table,
		eg_rook_table, eg_queen_table, eg_king_table

	};

	int ply = 0;

	// Position Searching Functions
	int		 negamax(int alpha, int beta, int depth);
	uint64_t search_position(int depth);
	int		 quiescence(int alpha, int beta);

	// Most Valuable Victim - Least Valuable Attacker
	// MVV LVA [attacker][victim]
	int mvv_lva[12][12] = { 105, 205, 305, 405, 505, 605, 105, 205, 305, 405,
		505, 605, 104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
		103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603, 102, 202,
		302, 402, 502, 602, 102, 202, 302, 402, 502, 602, 101, 201, 301, 401,
		501, 601, 101, 201, 301, 401, 501, 601, 100, 200, 300, 400, 500, 600,
		100, 200, 300, 400, 500, 600,

		105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204,
		304, 404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403,
		503, 603, 103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602,
		102, 202, 302, 402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201,
		301, 401, 501, 601, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400,
		500, 600

	};

	// Move Ordering
	int score_move(uint64_t move);
	int sort_moves(moves* move_list, uint64_t best_move);

	// Stores two killer moves for 192 ply
	int killer_moves[2][MAX_PLY];

	// Stores history moves for each piece and square combination
	int history_moves[12][64];

	// PV-tables, PV Searching
	int		 pv_length[MAX_PLY];
	uint64_t pv_table[MAX_PLY][MAX_PLY];
	int		 follow_pv, score_pv;
	void	 enable_pv(moves* move_list);

	// Number of moves searched to full depth
	const int full_depth_moves = 4;

	// Depth at which to start reducing
	const int reduction_limit = 3;

	// Depth to reduce by
	const int reduce = 2;

	// Zobrist Hashing
	uint64_t curr_zobrist_hash;
	uint64_t zobrist_keys[12][64];
	uint64_t en_passant_zobrist[64];
	uint64_t castle_zobrist[16];
	uint64_t zobrist_side_key;
	void	 init_zobrist();

	typedef struct tag_hash {
		uint64_t key;
		int		 depth;
		int		 flags;
		int		 value;
		uint64_t best_move;
	} hash;

	hash tt_table[hash_size];

	void reset_hashes();

	int	 read_hash_entry(int alpha, int beta, int depth, uint64_t* best_move);
	void set_entry(int value, int depth, int flags, uint64_t best_move);

	// Null Moves
	bool null_move_made = false;

	// Repetitions
	uint64_t repetition_table[512];
	int		 repetition_index;
	int		 is_repetition();

	// Evaluation Masks
	uint64_t file_masks[64] = { 0 };
	uint64_t rank_masks[64] = { 0 };
	uint64_t isolated_pawn_masks[64] = { 0 };
	uint64_t white_passed_pawn_masks[64] = { 0 };
	uint64_t black_passed_pawn_masks[64] = { 0 };
	uint64_t set_file_rank_mask(int file, int rank);
	void	 init_evaluation_masks();

	// Evaluation Bonuses and Penalties
	int double_pawn_penalty = -10;
	int isolated_pawn_penalty = -10;
	// passed pawn bonus
	const int passed_pawn_bonus[8] = { 0, 5, 15, 20, 50, 70, 110, 160 };
	// Open/Semi-Open File Bonuses
	const int semi_open_file_score = 10;
	const int open_file_score = 25;

	// Checkmate/Stalemate values
	bool is_checkmate;
	bool is_stalemate;

	// Player's possible moves for highlighting
	moves player_moves[1];

	// Legalizing their moves
	void legalize_player_moves(moves* player_moves, moves* pseudo_legal_moves);

	// NNUE pieces used by the Stockfish NNUE
	int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

	// Squares ordered for use in the Stockfish NNUE
	int nnue_squares[64] = { a1, b1, c1, d1, e1, f1, g1, h1, a2, b2, c2, d2, e2,
		f2, g2, h2, a3, b3, c3, d3, e3, f3, g3, h3, a4, b4, c4, d4, e4, f4, g4,
		h4, a5, b5, c5, d5, e5, f5, g5, h5, a6, b6, c6, d6, e6, f6, g6, h6, a7,
		b7, c7, d7, e7, f7, g7, h7, a8, b8, c8, d8, e8, f8, g8, h8

	};

	// Whether to use NNUE or not
	bool use_nnue = false;
	void enable_nnue(bool use_nnue);

	// All eval functions rewritten for the NNUE instead
	int		 nnue_eval();
	uint64_t search_position_nnue(int depth);
	int		 negamax_nnue(int alpha, int beta, int depth);
	int		 quiescence_nnue(int alpha, int beta);
};