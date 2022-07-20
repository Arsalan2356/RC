FILES = main.cpp validity.cpp generation.cpp init.cpp compute_tables.cpp magic.cpp perft.cpp Move.cpp

all: release debug


release: $(FILES)
	g++ -std=c++20 -Wall -Wextra -mpopcnt -Ofast -static-libgcc -static-libstdc++ -I src/include -L src/lib $(FILES) -o ./bin/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image



debug: $(FILES)
	g++ -std=c++20 -Wall -Wextra -mpopcnt -g -ggdb -static-libgcc -static-libstdc++ -I src/include -L src/lib $(FILES) -o ./debug/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image