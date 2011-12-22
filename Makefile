all: engine

engine: board.o engine.o ai.o
	gcc -Wall board.o engine.o ai.o -o engine

board.o: board.c
	gcc -Wall -c board.c

engine.o: engine.c
	gcc -Wall -c engine.c

ai.o: ai.c
	gcc -Wall -c ai.c

clean:
	rm -f *.o engine iolog.txt xboard.debug
