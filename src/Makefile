CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -ggdb

.PHONY: all clean
all: arth
arth: arthmetic.o lexer.o queue.o rbtree/node.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
clean:
	rm -rf *.o arth
