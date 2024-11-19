all:
	g++ -I src/include -L src/lib -o main main.cpp Platform.cpp Chip8.cpp -lmingw32 -lSDL2main -lSDL2