#include "queue.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/*
	EXPR = ADD ';'
	ADD = MUL | ADD '+' MUL | ADD '-' MUL
	MUL = UNARY | MUL '*' UNARY | MUL '/' UNARY
	UNARY = PRIM | '+' PRIM | '-' PRIM
	PRIM = NUM | '(' ADD ')'
*/
/*
	EXPR = ADD ';'

	ADD = MUL ADD_REST
	ADD_REST = NIL | '+' MUL ADD_REST | '-' MUL ADD_REST

	MUL = UNARY MUL_REST
	MUL_REST = NIL | '*' UNARY MUL_REST | '/' UNARY MUL_REST

	UNARY = PRIM | '+' UNARY | '-' UNARY
	PRIM = NUM | '(' ADD ')'
*/

// return 1 when error occurs
int match_and_pop(enum token c)
{
	if (gl_queue->back(gl_queue).type != c) {
		printf("Expected Token %d, but found %d\n", c, gl_queue->back(gl_queue).type);
		exit(1);
	}
	gl_queue->pop(gl_queue);
	next();
	return 0;
}

long long int add(void);

long long int primary()
{
	if (gl_queue->back(gl_queue).type == L_PARA) {
		match_and_pop(L_PARA);
		long long int result = add();
		match_and_pop(R_PARA);
		return result;
	} else {
		long long int result = gl_queue->back(gl_queue).lli_data;
		match_and_pop(NUMBER);
		return result;
	}
}
long long int unary(void)
{
	if (gl_queue->back(gl_queue).type == PLUS) {
		match_and_pop(PLUS);
		return unary();
	} else if (gl_queue->back(gl_queue).type == MINUS) {
		match_and_pop(MINUS);
		return -unary();
	}
	return primary();
}

long long int mul_rest(long long int uu)
{
	if (gl_queue->back(gl_queue).type == MUL) {
		match_and_pop(MUL);
		return mul_rest(uu * unary());
	} else if (gl_queue->back(gl_queue).type == DIVIDE) {
		match_and_pop(DIVIDE);
		return mul_rest(uu / unary());
	}
	return uu;
}

long long int mul(void)
{
	return mul_rest(unary());
}

long long int add_rest(long long int mm)
{
	if (gl_queue->back(gl_queue).type == PLUS) {
		match_and_pop(PLUS);
		return add_rest(mm + mul());
	} else if (gl_queue->back(gl_queue).type == MINUS) {
		match_and_pop(MINUS);
		return add_rest(mm - mul());
	}
	return mm;
}

long long int add(void)
{
	return add_rest(mul());
}

long long int expr(void)
{
	long long int result = add(); match_and_pop(SEMI); return result;
}

void clean_up(void)
{
	fclose(source);
	clear_queue(gl_queue);
}

void signal_clean_up(__attribute__((unused)) int signum)
{
	exit(1);
}

void usage(void)
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

	init_queue(&gl_queue);
	signal(SIGINT, signal_clean_up);
	atexit(clean_up);
	next();
	while (gl_queue->back(gl_queue).type != END) {
		printf("%lld\n", expr());
	}
	return 0;
}
