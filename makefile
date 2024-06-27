CC=gcc
FLAGS= -Wall -Wextra -pedantic
MAIN=editor

all: $(MAIN)

$(MAIN): main.o editor.o
	$(CC) -o $(MAIN) main.o editor.o -lncurses

main.o: main.c editor.h
	$(CC) -c main.c $(FLAGS)

editor.o: editor.c editor.h 
	$(CC) -c editor.c $(FLAGS)

clean:
	rm -f *.o *.gch *~

purge:
	rm -f *.o *.gch *~ $(MAIN)
