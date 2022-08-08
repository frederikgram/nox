CC = g++
CFLAGS = -w -g  -lstdc++ -lm
SRCS      = $(wildcard *.cpp)
nothing: main.o lexer.o
	g++ $(CFLAGS) $(SRCS) -o nothing


debug: CFLAGS += -DDEBUG
debug: nothing
.PHONY: clean
clean:
	rm *.out *.output *.o *.s nothing 