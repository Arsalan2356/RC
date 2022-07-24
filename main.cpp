#include "Board.h"
#include "Move.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <chrono>
#include <iostream>

void draw_board(SDL_Renderer &renderer, int square_size);
void init_pieces(SDL_Renderer &renderer);
void draw_piece(SDL_Renderer &renderer, int square_size, uint64_t board, int piece_pos);

const int WIDTH = 512,
		  HEIGHT = 512;
const int BOARD_SIZE = 8;
const int SQUARE_SIZE = WIDTH / BOARD_SIZE;
static SDL_Texture *piece_textures[12];

int main(int argc, char *argv[])
{
	std::string fen = "";
	if (argc != 1)
	{
		if (strcmp(argv[1], "-f") == 0)
		{
			if (argc > 2)
			{
				fen = argv[2];
			}
			else
			{
				std::cout << "-f requires a parameter."
						  << "\n"
						  << "Input has been ignored and fen has been initialized to: "
						  << "\n"
						  << "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
			}
		}
	}
	if (strcmp(fen.c_str(), "") == 0)
	{
		fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	}

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

	if (window == NULL)
	{
		std::cout << "Could not create window" << SDL_GetError() << "\n";
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL)
	{
		std::cout << "Could not create renderer" << SDL_GetError() << "\n";
		return 1;
	}

	bool running = true;
	SDL_Event event;
	SDL_RenderClear(renderer);
	int square_selected[2] = {-1, -1};
	auto t1 = std::chrono::high_resolution_clock::now();
	Board *board = new Board(fen);
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << "\n";
	init_pieces(*renderer);
	while (running)
	{
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				int x, y;
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					x = event.button.x / SQUARE_SIZE;
					y = event.button.y / SQUARE_SIZE;
					if (square_selected[0] == -1)
					{
						square_selected[0] = (x + y * 8);
					}
					else
					{
						square_selected[1] = (x + y * 8);
					}

					if (square_selected[0] != -1 && square_selected[1] != -1)
					{
						if (square_selected[0] != square_selected[1])
						{
							Move move = Move(square_selected[0], square_selected[1]);

							if (board->populate_move(move))
							{
								board->make_move(move.move_id, 0);
								board->update_log(move.move_id);
							}
						}
						square_selected[0] = -1;
						square_selected[1] = -1;
					}
					break;
				case SDL_BUTTON_RIGHT:
					square_selected[0] = -1;
					square_selected[1] = -1;
					break;
				default:
					break;
				}
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case (SDLK_s):
					t1 = std::chrono::high_resolution_clock::now();
					board->search_position(8);
					t2 = std::chrono::high_resolution_clock::now();
					ms_double = t2 - t1;
					std::cout << ms_double.count() << "\n";
					break;
				case (SDLK_r):
					board->init(fen);
					break;
				default:
					break;
				}

				break;

			default:
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Place the chess board along with pieces into the buffer
		draw_board(*renderer, SQUARE_SIZE);
		for (int i = 0; i < 12; i++)
		{
			draw_piece(*renderer, SQUARE_SIZE, board->bitboards[i], i);
		}
		// Update the window with everything stored in the buffer

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

void draw_board(SDL_Renderer &renderer, int square_size)
{
	for (int pos = 0; pos < 64; pos++)
	{
		int x = pos % 8;
		int y = pos / 8;
		SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
		if ((x + y) % 2 == 0)
		{
			SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(&renderer, 0, 102, 0, 255);
		}
		SDL_RenderFillRect(&renderer, &rect);
	}
};

void draw_piece(SDL_Renderer &renderer, int square_size, uint64_t board, int piece_pos)
{
	uint64_t temp_val;
	int pos = -1;

	temp_val = board;
	pos = -1;
	while (temp_val != 0)
	{
		int offset = __builtin_ffsll(temp_val);
		pos += offset;
		int x, y;
		x = pos % 8;
		y = pos / 8;

		SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
		SDL_RenderCopy(&renderer, piece_textures[piece_pos], NULL, &rect);
		temp_val >>= offset;
	}
};

void init_pieces(SDL_Renderer &renderer)
{
	for (int i = 0; i < 12; i++)
	{
		std::string file_name = "./images/" + std::to_string(i) + ".png";
		piece_textures[i] = IMG_LoadTexture(&renderer, file_name.c_str());
	}
}