#CC=~/emscripten/emcc
#CC=clang-3.2
CC=tcc
CFLAGS=$(shell sdl-config --cflags --libs) -lm $(shell pkg-config --libs --cflags glu)

main.html: main.c player.h particles.h defines.h
	$(CC) $(CFLAGS) -o main.html main.c

clean:
	rm -f main.html

.PHONY: clean
