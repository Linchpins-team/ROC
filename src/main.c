#include "ast.h"
#include "queue.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

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
		puts("No filename entered");
		usage();
		return 1;
	}
	source = fopen(argv[1], "r");
	if (source == (void *)0) {
		puts("Cannot open this file");
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
