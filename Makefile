CC = gcc
CFLAGS = -std=c11 -g -DDEBUG


nothing: main.o lexer.o
	gcc $(CFLAGS) main.c lexer.c -o nothing

.PHONY: clean
clean:
	rm *.out *.output *.o *.s nothing 