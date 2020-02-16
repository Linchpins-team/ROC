CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -ggdb -I./include

.PHONY: all clean
all: main
main: main.o parser/parser.o lexer/lexer.o queue/queue.o ast/ast.o strlist/strlist.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
clean:
	rm main
	find . -name \*.o -type f -delete
