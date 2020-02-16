CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -ggdb -I./include

.PHONY: all clean
all: main
main: main.o parser/parser.o lexer/lexer.o \
	queue/queue.o ast/ast.o strlist/strlist.o \
	map_stack/map_stack.o map/map.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
clean:
	rm -f main
	find . -name \*.o -type f -delete
