CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -ggdb

.PHONY: all clean
all: arth parse
arth: arthmetic.o lexer.o queue.o
	${CC} ${CFLAGS} $^ -o $@
parse: parser.o lexer.o queue.o sexp.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
clean:
	rm -rf *.o arth parse
