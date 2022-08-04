CC = g++
CFLAGS = -g -DDEBUG -lstdc++ -lm 
SRCS      = $(wildcard *.cpp)
nothing: main.o lexer.o
	g++ $(CFLAGS) $(SRCS) -o nothing 

.PHONY: clean
clean:
	rm *.out *.output *.o *.s nothing 