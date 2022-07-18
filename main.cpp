#include <iostream>
#include <SDL2/SDL.h>
#include "Board.h"
#include "Move.h"
#include <chrono>

const int WIDTH = 512, HEIGHT = 512;
const int BOARD_SIZE = 8;
const int SQUARE_SIZE = WIDTH / BOARD_SIZE;

int main(int argc, char *argv[])
{

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == NULL)
	{
		std::cout << "Could not create window" << SDL_GetError() << "\n";
		return 1;
	}

	bool running = true;
	SDL_Event event;
	SDL_RenderClear(renderer);
	int square_selected[2] = {-1, -1};
	auto t1 = std::chrono::high_resolution_clock::now();
	Board *board = new Board(renderer, "rnbqkbnr/ppp2ppp/8/8/8/2Ppp3/PPP1PPPP/RNBQKBNR w KQkq - 0 1");
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << "\n";
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

							if (board->populate_move(&move))
							{
								if (board->check_validity(&move))
								{
									board->make_move(&move);
									board->generate_legal_moves();
								}
							}
						}
						square_selected[0] = -1;
						square_selected[1] = -1;
					}
					break;
				case SDL_BUTTON_RIGHT:
					square_selected[0] = -1;
					break;
				default:
					break;
				}
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_1:
					break;

				default:
					break;
				}

				break;

			default:
				break;
			}
		}

		// Place the chess board along with pieces into the buffer
		board->draw_board(SQUARE_SIZE);

		// Update the window with everything stored in the buffer

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}