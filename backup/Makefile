CC = gcc
CFLAGS = -std=c11 -g -DDEBUG 


nothing: main.o lexer.o
	gcc $(CFLAGS) main.cpp lexer.cpp parser.cpp symbols.cpp -o nothing -lm

.PHONY: clean
clean:
	rm *.out *.output *.o *.s nothing 