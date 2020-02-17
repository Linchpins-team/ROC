#include "ast.h"
#include "queue.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define GREEN "\x1b[;32;1m"
#define RED "\x1b[;31;1m"
#define RESET "\x1b[0m"

static void panic(const char *msg)
{
	fprintf(stdout, "cc: " RED "fatal error" RESET ": %s\n", msg);
	exit(1);
}

#undef GREEN
#undef RED
#undef RESET

static void clean_up(void)
{
	fclose(source);
	exit_lexer();
}

static void signal_clean_up(int signum)
{
	exit(1);
}

static void usage(void)
{
	puts("arth Filename");
	puts("File: Every Expression should be end in a semicolon; there\n"
		"could be more than one expression in a file.");
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		panic("no input file");
		usage();
		return 1;
	}

	source_filename = argv[1];
	source = fopen(argv[1], "r");
	if (source == (void *)0) {
		panic(strerror(errno));
		return 1;
	}

	signal(SIGINT, signal_clean_up);
	atexit(clean_up);
	init_lexer();
	next();
	while (gl_queue->back(gl_queue).type != END) {
		ast_t *res = translation_unit();
		print_ast(res);
		remove_ast(res);
	}
	return 0;
}
