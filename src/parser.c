#include "queue.h"
#include "lexer.h"
#include "aalloc.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	ARGU_LIST = ASSIGN ARGU_REST
	ARGU_REST = ',' ASSIGN ARGU_REST | NIL

	PRIM = IDENTIFIER | CONSTANT | STR_LIT | '(' EXPR ')'
*/

// return 1 when error occurs
static int match_and_pop(enum token c)
{
	if (gl_queue->back(gl_queue).type != c) {
		fprintf(stdout, "Expected Token: %d , but found Token: %d\n",
			c, gl_queue->back(gl_queue).type);
		exit(1);
	}
	gl_queue->pop(gl_queue);
	next();
	return 0;
}

static ast_t *expr(void);

static ast_t *identifier(void)
{
	ast_t *node = new_node(AS_ID, NULL);

	match_and_pop(ID);
	return node;
}

static ast_t *constant(void)
{
	ast_t *node = new_node(AS_INT_LIT, NULL);
	node->lli = gl_queue->back(gl_queue).lli_data;
	match_and_pop(NUMBER);
	return node;
}

static ast_t *primary(void)
{
	ast_t *node;
	switch(gl_queue->back(gl_queue).type) {
	case L_PARA:
		match_and_pop(L_PARA);
		node = expr();
		match_and_pop(R_PARA);
		return node;
	case ID:
		return identifier();
	case DQUOTE:
		/* TODO */
	default:
		return constant();
	}
}

static ast_t *assign(void);
static ast_t *argu_rest(ast_t *aa)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_ARGU_LIST, NULL);
		add_son(node, aa);
		add_son(node, assign());
		return argu_rest(node);
	default:
		return aa;
	}
}

static ast_t *argu_list(void)
{
	return argu_rest(assign());
}

static ast_t *post_rest(ast_t *pp)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case L_BRACK:
		match_and_pop(L_BRACK);
		node = new_node(AS_INDEX, NULL);
		add_son(node, pp);
		add_son(node, expr());
		match_and_pop(R_BRACK);
		return post_rest(node);
	case L_PARA:
		match_and_pop(L_PARA);
		node = new_node(AS_FUNC_CALL, NULL);
		add_son(node, pp);
		if (!(gl_queue->back(gl_queue).type == R_PARA)) {
			add_son(node, argu_list());
		}
		match_and_pop(R_PARA);
		return post_rest(node);
	case MEMBER:
		match_and_pop(MEMBER);
		node = new_node(AS_MEMBER, NULL);
		add_son(node, pp);
		add_son(node, identifier());
		return post_rest(node);
	case PTR_MEMBER:
		match_and_pop(PTR_MEMBER);
		node = new_node(AS_PTR_MEMBER, NULL);
		add_son(node, pp);
		add_son(node, identifier());
		return post_rest(node);
	case INC:
		match_and_pop(INC);
		node = new_node(AS_POST_INC, NULL);
		add_son(node, pp);
		return post_rest(node);
	case DEC:
		match_and_pop(DEC);
		node = new_node(AS_POST_DEC, NULL);
		add_son(node, pp);
		return post_rest(node);
	default:
		return pp;
	}
}

static ast_t *post(void)
{
	return post_rest(primary());
}

static ast_t *cast(void);
static ast_t *unary(void)
{
	ast_t *node;
	ast_t *ret;
	ret = new_node(AS_UNARY, NULL); /* FOR ASSIGNMENT */
	switch(gl_queue->back(gl_queue).type) {
	case INC:
		match_and_pop(INC);
		node = new_node(AS_PRE_INC, NULL);
		add_son(node, unary());
		break;
	case DEC:
		match_and_pop(DEC);
		node = new_node(AS_PRE_DEC, NULL);
		add_son(node, unary());
		break;
	case PLUS:
		match_and_pop(PLUS);
		node = new_node(AS_POS, NULL);
		add_son(node, cast());
		break;
	case MINUS:
		match_and_pop(MINUS);
		node = new_node(AS_NEG, NULL);
		add_son(node, cast());
		break;
	case AND:
		match_and_pop(AND);
		node = new_node(AS_ADDRESS_OF, NULL);
		add_son(node, cast());
		break;
	case MUL:
		match_and_pop(MUL);
		node = new_node(AS_VALUE_OF, NULL);
		add_son(node, cast());
		break;
	case COMPLEMENT:
		match_and_pop(COMPLEMENT);
		node = new_node(AS_COMPLEMENT, NULL);
		add_son(node, cast());
		break;
	case NOT:
		match_and_pop(NOT);
		node = new_node(AS_NOT, NULL);
		add_son(node, cast());
		break;
	case SIZEOF:
		/* FIXME: typename */
		match_and_pop(SIZEOF);
		node = new_node(AS_SIZEOF, NULL);
		add_son(node, unary());
		break;
	default:
		node = post();
		break;
	}
	return add_son(ret, node);
}

static ast_t *cast(void)
{
	switch(gl_queue->back(gl_queue).type) {
	case L_PARA:
		/* FIXME type name */
	default:
		return unary();
	}
}

static ast_t *mul_rest(ast_t *cc)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case MUL:
		match_and_pop(MUL);
		node = new_node(AS_MUL, NULL);
		add_son(node, cc);
		add_son(node, cast());
		return mul_rest(node);
	case DIVIDE:
		match_and_pop(DIVIDE);
		node = new_node(AS_DIVIDE, NULL);
		add_son(node, cc);
		add_son(node, cast());
		return mul_rest(node);
	case MOD:
		match_and_pop(MOD);
		node = new_node(AS_MOD, NULL);
		add_son(node, cc);
		add_son(node, cast());
		return mul_rest(node);
	default:
		return cc;
	}
}

static ast_t *mul(void)
{
	return mul_rest(cast());
}

static ast_t *add_rest(ast_t *mm)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case PLUS:
		match_and_pop(PLUS);
		node = new_node(AS_ADD, NULL);
		add_son(node, mm);
		add_son(node, mul());
		return add_rest(node);
	case MINUS:
		match_and_pop(MINUS);
		node = new_node(AS_MINUS, NULL);
		add_son(node, mm);
		add_son(node, mul());
		return add_rest(node);
	default:
		return mm;
	}
}

static ast_t *add(void)
{
	return add_rest(mul());
}

static ast_t *shift_rest(ast_t *aa)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case SHL:
		match_and_pop(SHL);
		node = new_node(AS_SHL, NULL);
		add_son(node, aa);
		add_son(node, add());
		return shift_rest(node);
	case SHR:
		match_and_pop(SHR);
		node = new_node(AS_SHR, NULL);
		add_son(node, aa);
		add_son(node, add());
		return shift_rest(node);
	default:
		return aa;
	}
}

static ast_t *shift(void)
{
	return shift_rest(add());
}

static ast_t *rela_rest(ast_t *ss)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case LT:
		match_and_pop(LT);
		node = new_node(AS_LT, NULL);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case GT:
		match_and_pop(GT);
		node = new_node(AS_GT, NULL);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case LE:
		match_and_pop(LE);
		node = new_node(AS_LE, NULL);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case GE:
		match_and_pop(GE);
		node = new_node(AS_GE, NULL);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	default:
		return ss;
	}
}

static ast_t *rela(void)
{
	return rela_rest(shift());
}

static ast_t *equ_rest(ast_t *rr)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case EQ:
		match_and_pop(EQ);
		node = new_node(AS_EQ, NULL);
		add_son(node, rr);
		add_son(node, rela());
		return equ_rest(node);
	case NEQ:
		match_and_pop(NEQ);
		node = new_node(AS_NEQ, NULL);
		add_son(node, rr);
		add_son(node, rela());
		return equ_rest(node);
	default:
		return rr;
	}
}

static ast_t *equ(void)
{
	return equ_rest(rela());
}

static ast_t *and_rest(ast_t *ee)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case AND:
		match_and_pop(AND);
		node = new_node(AS_AND, NULL);
		add_son(node, ee);
		add_son(node, equ());
		return and_rest(node);
	default:
		return ee;
	}
}

static ast_t *and(void)
{
	return and_rest(equ());
}

static ast_t *xor_rest(ast_t *aa)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case XOR:
		match_and_pop(XOR);
		node = new_node(AS_XOR, NULL);
		add_son(node, aa);
		add_son(node, and());
		return xor_rest(node);
	default:
		return aa;
	}
}

static ast_t *xor(void)
{
	return xor_rest(and());
}

static ast_t *or_rest(ast_t *xx)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case OR:
		match_and_pop(OR);
		node = new_node(AS_OR, NULL);
		add_son(node, xx);
		add_son(node, xor());
		return or_rest(node);
	default:
		return xx;
	}
}

static ast_t *or(void)
{
	return or_rest(xor());
}

static ast_t *log_and_rest(ast_t *oo)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case LOG_AND:
		match_and_pop(LOG_AND);
		node = new_node(AS_LOG_AND, NULL);
		add_son(node, oo);
		add_son(node, or());
		return log_and_rest(node);
	default:
		return oo;
	}
}

static ast_t *log_and(void)
{
	return log_and_rest(or());
}

static ast_t *log_or_rest(ast_t *ll)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case LOG_OR:
		match_and_pop(LOG_OR);
		node = new_node(AS_LOG_OR, NULL);
		add_son(node, ll);
		add_son(node, log_and());
		return log_or_rest(node);
	default:
		return ll;
	}
}

static ast_t *log_or(void)
{
	return log_or_rest(and());
}

static ast_t *condi(void)
{
	ast_t *lor;
	ast_t *node;
	lor = log_or();
	switch (gl_queue->back(gl_queue).type) {
	case QUESTION:
		match_and_pop(QUESTION);
		node = new_node(AS_CONDI, NULL);
		add_son(node, lor);
		add_son(node, expr());
		match_and_pop(COLON);
		return add_son(node, condi());
	default:
		return lor;
	}
}

static ast_t *assign(void)
{
	ast_t *node;
	ast_t *cnd;
	cnd = condi();
	switch (gl_queue->back(gl_queue).type) {
	case ASS:
	case MULASS:
	case DIVASS:
	case MODASS:
	case ADDASS:
	case SUBASS:
	case SHLASS:
	case SHRASS:
	case ANDASS:
	case XORASS:
	case ORASS:
		if (!(cnd->type == AS_UNARY)) {
			exit(1);
		}
		break;
	default:
		break;
	}

	switch (gl_queue->back(gl_queue).type) {
	case ASS:
		match_and_pop(ASS);
		node = new_node(AS_ASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case MULASS:
		match_and_pop(MULASS);
		node = new_node(AS_MULASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case DIVASS:
		match_and_pop(DIVASS);
		node = new_node(AS_DIVASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case MODASS:
		match_and_pop(MODASS);
		node = new_node(AS_MODASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case ADDASS:
		match_and_pop(ADDASS);
		node = new_node(AS_ADDASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case SUBASS:
		match_and_pop(SUBASS);
		node = new_node(AS_SUBASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case SHLASS:
		match_and_pop(SHLASS);
		node = new_node(AS_SHLASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case SHRASS:
		match_and_pop(SHRASS);
		node = new_node(AS_SHRASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case ANDASS:
		match_and_pop(ANDASS);
		node = new_node(AS_ANDASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case XORASS:
		match_and_pop(XORASS);
		node = new_node(AS_XORASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	case ORASS:
		match_and_pop(ORASS);
		node = new_node(AS_ORASS, NULL);
		add_son(node, cnd);
		return add_son(node, assign());
	default:
		return cnd;
	}
}


static ast_t *expr_rest(ast_t *aa)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_COMMA, NULL);
		add_son(node, aa);
		add_son(node, assign());
		return expr_rest(node);
	default:
		return aa;
	}
}

static ast_t *expr(void)
{
	ast_t *node = new_node(AS_EXPR, NULL);
	return add_son(node, expr_rest(assign()));
}

static ast_t *expr_stat(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case SEMI:
		match_and_pop(SEMI);
		node = new_node(AS_EXPR_NULL, NULL);
		return node;
	default:
		node = expr();
		match_and_pop(SEMI);
		return node;
	}
}

void clean_up(void)
{
	fclose(source);
	clear_queue(gl_queue);
	aalloc_delete();
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
	init_aalloc();
	atexit(clean_up);
	next();
	while (gl_queue->back(gl_queue).type != END) {
		ast_t *res = expr_stat();
		print_all(res);
	}
	return 0;
}
