CC = g++
CFLAGS = -w -g  -lstdc++ -lm
SRCS      = $(wildcard *.cpp)
nothing: main.o lexer.o parser.o typecheck.o utils.o intermediate.o
	g++ $(CFLAGS) $(SRCS) -o nothing

.PHONY: clean
clean:
	rm *.out *.output *.o *.s nothing 