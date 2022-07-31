FILES = main.cpp compute_tables.cpp magic.cpp zobrist.cpp Move.cpp init.cpp validity.cpp generation.cpp perft.cpp ./nnue/nnue.cpp ./nnue/misc.cpp nnue_eval.cpp nnue_search.cpp ai.cpp
CMD_FILES = cmd.cpp compute_tables.cpp magic.cpp zobrist.cpp Move.cpp init.cpp validity.cpp generation.cpp perft.cpp ./nnue/nnue.cpp ./nnue/misc.cpp nnue_eval.cpp nnue_search.cpp ai.cpp

all: release debug cmd cmd_debug


release: $(FILES)
	g++ -std=c++20 -Ofast -march=native -fno-signed-zeros -fno-trapping-math -frename-registers -foptimize-sibling-calls -I src/include -L src/lib $(FILES) -o ./bin/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

debug: $(FILES)
	g++ -std=c++20 -march=native -foptimize-sibling-calls -g -ggdb -I src/include -L src/lib $(FILES) -o ./debug/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

cmd: $(CMD_FILES)
	g++ -std=c++20 -Ofast -march=native -fno-signed-zeros -fno-trapping-math -frename-registers -foptimize-sibling-calls $(CMD_FILES) -o ./bin/cmd.exe -lmingw32

cmd_debug: $(CMD_FILES)
	g++ -std=c++20 -march=native -foptimize-sibling-calls -g -ggdb $(CMD_FILES) -o ./debug/cmd.exe -lmingw32
