#include "queue.h"
#include "lexer.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/*
	EXPR = ASSIGN | EXPR , ASSIGN
	ASSIGN = CONDI | UNARY "= *= /= %= += -= <<= >>= &= ^= |=" ASSIGN
	CONDI = LOG_OR | LOG_OR ? EXPR : CONDI
	LOG_OR = LOG_AND | LOG_OR '||' LOG_AND
	LOG_AND = IOR | LOG_AND && IOR
	IOR = XOR | IOR '|' XOR
	XOR = AND | XOR ^ AND
	AND = EQU | AND & EQU
	EQU = RELA | EQU "== !=" RELA
	RELA = SHIFT | RELA "< > <= >=" SHIFT
	SHIFT = ADD | SHIFT "<< >>" ADD
	ADD = MUL | ADD "+ -" MUL
	MUL = CAST | MUL "* / %" CAST
	CAST = UNARY | ( type-name ) CAST
	UNARY = POST | "++ --" UNARY | "& * + - ~ !" CAST | "sizeof" UNARY | sizeof ( typename )
	POST = PRIM | POST [ EXPR ] | POST ( argument-expression-list(opt) )
		| POST . IDENTIFIER | POST -> IDENTIFIER | POST "++ --" | ----initializer-list--------
	PRIM = IDENTIFIER | CONSTANT | STR_LIT | ( EXPR )
*/

/*
	EXPR = ASSIGN EXPR_REST
	EXPR_REST = NIL | ',' ASSIGN EXPR_REST
	ASSIGN = CONDI | UNARY "= *= /= %= += -= <<= >>= &= ^= |=" ASSIGN
	CONDI = LOG_OR | LOG_OR ? EXPR : CONDI

	LOG_OR = LOG_AND LOG_OR_REST
	LOG_OR_REST = '||' LOG_AND LOG_OR_REST | NIL

	LOG_AND = IOR LOG_AND_REST
	LOG_AND_REST = '&&' IOR LOG_AND_REST | NIL

	IOR = XOR IOR_REST
	IOR_REST = '|' XOR IOR_REST | NIL

	XOR = AND XOR_REST
	XOR_REST = '^' AND XOR_REST | NIL

	AND = EQU AND_REST
	AND_REST = '&' EQU AND_REST | NIL

	EQU = RELA EQU_REST
	EQU_REST = '==' RELA EQU_REST | '!=' RELA EQU_REST | NIL

	RELA = SHIFT RELA_REST
	RELA_REST = "< > <= >=" SHIFT RELA_REST

	SHIFT = ADD SHIFT_REST
	SHIFT_REST = "<< >>" ADD SHIFT_REST | NIL

	ADD = MUL ADD_REST
	ADD_REST = "+ -" MUL ADD_REST | NIL

	MUL = CAST MUL_REST
	MUL_REST = "* / %" CAST MUL_REST | NIL

	CAST = UNARY | ( type-name ) CAST

	UNARY = POST | "++ --" UNARY | "& * + - ~ !" CAST | "sizeof" UNARY | sizeof ( typename )

	POST = PRIM POST_REST
	POST_REST = '[' EXPR ']' POST_REST | '(' argument-expression-list(opt) ')' POST_REST |
		'.' IDENTIFIER POST_REST | '->' IDENTIFIER POST_REST | "++ --" POST_REST | NIL

	PRIM = IDENTIFIER | CONSTANT | STR_LIT | '(' EXPR ')'
*/

ast_t root;

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

ast_t *add(void);
ast_t *primary(void)
{
	if (gl_queue->back(gl_queue).type == L_PARA) {
		match_and_pop(L_PARA);
		ast_t *res = add();
		match_and_pop(R_PARA);
		return res;
	} else {
		ast_t *node = new_node(AS_INT_LIT, NULL);
		node->lli = gl_queue->back(gl_queue).lli_data;
		match_and_pop(NUMBER);
		return node;
	}
}

ast_t *unary(void)
{
	if (gl_queue->back(gl_queue).type == PLUS) {
		match_and_pop(PLUS);
		ast_t *node = new_node(AS_POS, NULL);
		return add_son(&node, unary());
	} else if (gl_queue->back(gl_queue).type == MINUS) {
		match_and_pop(MINUS);
		ast_t *node = new_node(AS_NEG, NULL);
		return add_son(&node, unary());
	} else {
		return primary();
	}
}

ast_t *mul_rest(ast_t *uu)
{
	if (gl_queue->back(gl_queue).type == MUL) {
		match_and_pop(MUL);
		ast_t *node = new_node(AS_MUL, NULL);
		add_son(&node, uu);
		add_son(&node, unary());
		return mul_rest(node);
	} else if (gl_queue->back(gl_queue).type == DIVIDE) {
		match_and_pop(DIVIDE);
		ast_t *node = new_node(AS_DIVIDE, NULL);
		add_son(&node, uu);
		add_son(&node, unary());
		return mul_rest(node);
	} else {
		return uu;
	}
}

ast_t *mul(void)
{
	return mul_rest(unary());
}

ast_t *add_rest(ast_t *mm)
{
	if (gl_queue->back(gl_queue).type == PLUS) {
		match_and_pop(PLUS);
		ast_t *node = new_node(AS_ADD, NULL);
		add_son(&node, mm);
		add_son(&node, mul());
		return add_rest(node);
	} else if (gl_queue->back(gl_queue).type == MINUS) {
		match_and_pop(MINUS);
		ast_t *node = new_node(AS_MINUS, NULL);
		add_son(&node, mm);
		add_son(&node, mul());
		return add_rest(node);
	} else {
		return mm;
	}
}

ast_t *add(void)
{
	return add_rest(mul());
}

ast_t *expr(void)
{
	ast_t *node = new_node(AS_EXPR, NULL);
	add_son(&node, add());
	match_and_pop(SEMI);
	return node;
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
		ast_t *res = expr();
		print_all(res);
		remove_all(res);
	}
	return 0;
}
