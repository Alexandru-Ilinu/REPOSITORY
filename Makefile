all:
	gcc -Wall tetris.c -o tetris -lncurses
run:
	./tetris
clean:
	rm -f tetris
