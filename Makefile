engine:
	gcc -Wall board.c ai.c engine.c -o engine 

clean:
	rm -f *.o engine iolog.txt xboard.debug
