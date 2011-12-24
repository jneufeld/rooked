all: engine

engine: board.o engine.o ai.o
	gcc -Wall -g board.o engine.o ai.o -o engine

board.o: board.c
	gcc -Wall -g -c board.c

engine.o: engine.c
	gcc -Wall -g -c engine.c

ai.o: ai.c
	gcc -Wall -g -c ai.c

clean:
	rm -f *.o engine iolog.txt xboard.debug
