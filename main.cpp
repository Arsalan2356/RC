#include "Board.h"
#include "Move.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

void draw_board(SDL_Renderer &renderer, int square_size);
void init_pieces(SDL_Renderer &renderer);
void draw_piece(SDL_Renderer &renderer, int square_size, uint64_t board, int piece_pos);
void highlight_last_move(SDL_Renderer &renderer, uint64_t move, int square_size);
void highlight_attacked_squares(SDL_Renderer &renderer, int pos, moves *player_moves);
void draw_circle(SDL_Renderer &renderer, int center_x, int center_y, int radius);
void draw_half_circle(SDL_Renderer &renderer, int center_x, int center_y, int radius1, int radius2);
const int WIDTH = 512,
		  HEIGHT = 512;
const int BOARD_SIZE = 8;
const int SQUARE_SIZE = WIDTH / BOARD_SIZE;
static SDL_Texture *piece_textures[12];

int main(int argc, char *argv[])
{
	std::string fen = "";
	bool player_one = true;
	bool player_two = false;
	if (argc != 1)
	{
		std::string input = "";
		for (int i = 1; i < argc; i++)
		{
			input += std::string(argv[i]) + " ";
		}

		std::vector<std::string> v = split(input);
		auto it = v.begin();
		int index = 0;
		if ((it = (std::find(v.begin(), v.end(), "-f"))) != v.end())
		{
			index = it - v.begin();
			fen = v[index + 1] + " " + v[index + 2] + " " + v[index + 3] + " " + v[index + 4] + " " + v[index + 5] + " " + v[index + 6];
		}
		if ((it = (std::find(v.begin(), v.end(), "-p"))) != v.end())
		{
			index = it - v.begin();
			std::string player = v[index + 1];
			if (player == "w")
			{
				player_one = true;
				player_two = false;
			}
			else if (player == "b")
			{
				player_one = false;
				player_two = true;
			}
			else if (player == "n")
			{
				player_one = false;
				player_two = false;
			}
			else if (player == "y")
			{
				player_one = true;
				player_two = true;
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

	bool human_turn = (board->side == board->white ? player_one : player_two);
	if (human_turn)
	{
		board->generate_moves(board->player_moves);
	}

	while (running)
	{
		human_turn = (board->side == board->white ? player_one : player_two);
		if (human_turn)
		{
			board->generate_moves(board->player_moves);
		}
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

					if (square_selected[0] != -1 && square_selected[1] != -1 && human_turn && !board->is_checkmate && !board->is_stalemate)
					{
						if (square_selected[0] != square_selected[1])
						{
							Move move = Move(square_selected[0], square_selected[1]);

							if (board->populate_move(move))
							{
								if (board->check_validity(move.move_id))
								{
									board->update_log(move.move_id);

									board->make_move(move.move_id, 0);

									board->update_game_state();
								}
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
					human_turn = (board->side == board->white ? player_one : player_two);
					break;
				case SDLK_n:
					std::cout << board->player_moves->count << "\n";
				default:
					break;
				}

				break;

			default:
				break;
			}
		}

		if (!human_turn && !board->is_checkmate && !board->is_stalemate)
		{
			uint64_t move = board->search_position(8);
			if (!player_one && !player_two)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			}

			board->update_log(move);
			board->make_move(move, 0);
			board->update_game_state();
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Place the chess board along with pieces into the buffer
		draw_board(*renderer, SQUARE_SIZE);
		if (board->move_log[board->move_index] != -1)
		{
			highlight_last_move(*renderer, board->move_log[board->move_index], SQUARE_SIZE);
		}

		if (square_selected[0] != -1)
		{
			int piece = -1;
			int start = (board->side == 0 ? 0 : 6);
			int end = (board->side == 0 ? 6 : 12);
			for (int i = start; i < end; i++)
			{
				if (get_bit(board->bitboards[i], square_selected[0]))
				{
					piece = i;
					break;
				}
			}
			if (piece != -1)
				highlight_attacked_squares(*renderer, square_selected[0], board->player_moves);
		}

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
			SDL_SetRenderDrawColor(&renderer, 238, 238, 212, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(&renderer, 115, 150, 92, 255);
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

void highlight_last_move(SDL_Renderer &renderer, uint64_t move, int square_size)
{
	int square_from = get_move_source(move);
	int square_to = get_move_target(move);

	SDL_SetRenderDrawColor(&renderer, 182, 202, 71, 255);

	int x_from = square_from % 8;
	int y_from = square_from / 8;

	int x_to = square_to % 8;
	int y_to = square_to / 8;

	SDL_Rect square_from_rect = {x_from * square_size, y_from * square_size, square_size, square_size};

	SDL_Rect square_to_rect = {x_to * square_size, y_to * square_size, square_size, square_size};

	SDL_RenderFillRect(&renderer, &square_from_rect);

	SDL_RenderFillRect(&renderer, &square_to_rect);
}

void highlight_attacked_squares(SDL_Renderer &renderer, int pos, moves *player_moves)
{
	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 15);

	for (int count = 0; count < player_moves->count; count++)
	{
		if (get_move_source(player_moves->moves[count]) == pos)
		{
			int target_pos = get_move_target(player_moves->moves[count]);
			int x = target_pos % 8 * SQUARE_SIZE;
			int y = target_pos / 8 * SQUARE_SIZE;

			if (get_move_capture(player_moves->moves[count]))
			{
				draw_half_circle(renderer, x + SQUARE_SIZE / 2, y + SQUARE_SIZE / 2, SQUARE_SIZE / 2 - 7, SQUARE_SIZE / 2);
			}
			else
			{
				draw_circle(renderer, x + SQUARE_SIZE / 2, y + SQUARE_SIZE / 2, SQUARE_SIZE / 2 - 18);
			}
		}
	}
}

void draw_circle(SDL_Renderer &renderer, int center_x, int center_y, int radius)
{
	SDL_SetRenderDrawBlendMode(&renderer, SDL_BLENDMODE_BLEND);
	for (int w = 0; w < radius * 2; w++)
	{
		for (int h = 0; h < radius * 2; h++)
		{
			int dx = radius - w; // horizontal offset
			int dy = radius - h; // vertical offset
			if ((dx * dx + dy * dy) <= (radius * radius))
			{
				SDL_RenderDrawPoint(&renderer, center_x + dx, center_y + dy);
			}
		}
	}
}

void draw_half_circle(SDL_Renderer &renderer, int center_x, int center_y, int radius1, int radius2)
{
	SDL_SetRenderDrawBlendMode(&renderer, SDL_BLENDMODE_BLEND);
	for (int w = 0; w < radius2 * 2; w++)
	{
		for (int h = 0; h < radius2 * 2; h++)
		{
			int dx = radius2 - w; // horizontal offset
			int dy = radius2 - h; // vertical offset
			if ((dx * dx + dy * dy) <= (radius2 * radius2) && (dx * dx + dy * dy) >= (radius1 * radius1))
			{
				SDL_RenderDrawPoint(&renderer, center_x + dx, center_y + dy);
			}
		}
	}
}