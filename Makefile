FILES = main.cpp validity.cpp generation.cpp init.cpp board_draw.cpp compute_tables.cpp magic.cpp Move.cpp

all: release debug


release: $(FILES)
	g++ -mpopcnt -O3 -static-libgcc -static-libstdc++ -I src/include -L src/lib $(FILES) -o ./bin/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image



debug: $(FILES)
	g++ -mpopcnt -g -ggdb -static-libgcc -static-libstdc++ -I src/include -L src/lib $(FILES) -o ./debug/main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image