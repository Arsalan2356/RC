FILES = main.cpp compute_tables.cpp magic.cpp zobrist.cpp Move.cpp init.cpp validity.cpp generation.cpp perft.cpp ai.cpp
CMD_FILES = cmd.cpp compute_tables.cpp magic.cpp zobrist.cpp Move.cpp init.cpp validity.cpp generation.cpp perft.cpp ai.cpp

all: release debug release_cmd debug_cmd


release: $(FILES)
	g++ -std=c++20 -Ofast -fstack-usage -foptimize-sibling-calls -I src/include -L src/lib $(FILES) -o ./bin/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

debug: $(FILES)
	g++ -std=c++20 -fstack-usage -foptimize-sibling-calls -g -ggdb -I src/include -L src/lib $(FILES) -o ./debug/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

release_cmd: $(CMD_FILES)
	g++ -std=c++20 -Ofast -fstack-usage -foptimize-sibling-calls $(CMD_FILES) -o ./bin/cmd.exe -lmingw32

debug_cmd: $(CMD_FILES)
	g++ -std=c++20 -fstack-usage -foptimize-sibling-calls -g -ggdb $(CMD_FILES) -o ./debug/cmd.exe -lmingw32
