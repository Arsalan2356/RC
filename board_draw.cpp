#include "Board.h"
#include <sstream>

void Board::draw_board(int square_size)
{
	for (int pos = 0; pos < 64; pos++)
	{
		int x = pos % 8;
		int y = pos / 8;
		SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
		if ((x + y) % 2 == 0)
		{
			SDL_SetRenderDrawColor(curr_renderer, 255, 255, 255, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(curr_renderer, 0, 102, 0, 255);
		}
		SDL_RenderFillRect(curr_renderer, &rect);
	}
	draw_pieces(square_size);
};

void Board::draw_pieces(int square_size)
{
	uint64_t temp_val;
	int pos = -1;
	for (int i = 0; i < 6; i++)
	{
		temp_val = white_pieces[i];
		pos = -1;
		while (temp_val != 0)
		{
			int offset = __builtin_ffsll(temp_val);
			// int offset = temp_bitset._Find_first();
			pos += offset;
			int x, y;
			x = pos % 8;
			y = pos / 8;

			SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
			SDL_RenderCopy(curr_renderer, piece_textures[i], NULL, &rect);
			temp_val >>= offset;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		temp_val = black_pieces[i];
		pos = -1;
		while (temp_val != 0)
		{
			int piece_pos = i + 6;
			int offset = __builtin_ffsll(temp_val);
			// int offset = temp_bitset._Find_first();
			pos += offset;
			int x, y;
			x = pos % 8;
			y = pos / 8;

			SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
			SDL_RenderCopy(curr_renderer, piece_textures[piece_pos], NULL, &rect);
			temp_val >>= offset;
		}
	}
};

std::vector<std::string> split(const std::string &fen)
{
	std::vector<std::string> v;
	std::string buf;
	std::stringstream ss(fen);
	while (ss >> buf)
	{
		v.push_back(buf);
	}
	return v;
}