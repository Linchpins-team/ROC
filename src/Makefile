CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -ggdb

.PHONY: all clean
all: main
main: main.o parser.o lexer.o queue.o ast.o strlist.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
clean:
	rm -rf *.o main
