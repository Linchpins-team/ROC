#include "parser.h"
#include "queue.h"
#include "lexer.h"
#include "ast.h"
#include "map.h"
#include "map_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

/* DECLARATION
	DECLARATION = DECL_SPECI INIT_DECL_LIST(opt)
	DECL_SPECI = [STORAGE_CLASS_SPECI TYPE_SPECI TYPE_QUALIFIER] DECL_SPECI(opt)
	INIT_DECL_LIST = INIT_DECL | INIT_DECL_LIST ',' INIT_DECL
	INIT_DECL = DECL | DECL = INITALIZER

	STORAGE_CLASS_SPECI = [TYPEDEF EXTERN STATIC AUTO REGISTER]
	TYPE_SPECI = [void char short int long float double signed
			unsigned STRUCT_UNION_SPECI ENUM_SPECI TYPEDEF_NAME]

	STRUCT_UNION_SPECI = [struct union] ID(opt) { struct-declaration-list } |
				[struct union] ID
	struct-declaration-list = struct-declaration | struct-declaration-list struct declaration
	struct-declaration = specifier-qualifier-list struct-declarator-list ';'
	specifier-qualifier-list = [type-specifier type-qualifier] specifier-qualifier-list(opt)
	struct-declarator-list = struct-declarator | struct-declarator-list ',' struct-declarator
	struct-declarator = declarator | declarator(opt) : constant expression

	enum-specifier = enum identifier | enum identifier(opt) '{' enumerator-list '}'

	enumerator-list = enumerator | enumerator-list ',' enumerator

	enumerator = enumeration-constant | enumeration-constant '=' constant-expression

	TYPE_QUALIFIER = [const volatile]

	DECL = pointer(opt) direct-declarator
	direct-declarator = identifer | '(' declarator ')' |
			direct-declarator '[' constant-expression ']' |
			direct-declarator '(' parameter-type-list ')' |
			direct-declarator '(' identifier-list ')' |
	pointer = '*' type-qualifier(opt) | '*' type-qualifier(opt) pointer
	parameter-type-list = parameter-list | parameter-list ', ...';
	parameter-list = parameter-declaration | parameter-list ',' parameter-declaration
	parameter-declaration = declaration-specifiers declaration |
				declaration-specifiers abstract-declarator(opt)
	identifier-list = identifier | identifier-list ',' identifier

	typename = specifier-qualifier-list abstract-declarator(opt)
	abstract-declarator = pointer | pointer(opt) direct-abstract-declarator
	direct-abstract-declarator = '(' abstract-declarator ')' |
					direct-abstract-declarator '[' constant-expression(opt) ']'
					direct-abstract-declarator '(' parameter-type-list(opt) ')'

	typedef-name = identifier

	initializer = assignment-expression | '{' inintializer-list '}' | '{' initializer-list ',' '}'
	initializer-list = initializer | initializer-list ',' initializer
*/

/* DECLARATION (NO RR)
	DECLARATION = DECL_SPECI INIT_DECL_LIST(opt) ';'
	DECL_SPECI = [STORAGE_CLASS_SPECI TYPE_SPECI TYPE_QUALIFIER] DECL_SPECI(opt)

	INIT_DECL_LIST_REST = ',' INIT_DECL INIT_DECL_LIST_REST | NIL
	INIT_DECL_LIST = INIT_DECL INIT_DECL_LIST_REST

	INIT_DECL = DECL | DECL = INITALIZER

	STORAGE_CLASS_SPECI = [TYPEDEF EXTERN STATIC AUTO REGISTER]

	TYPE_SPECI = [void char short int long float double signed
			unsigned STRUCT_UNION_SPECI ENUM_SPECI TYPEDEF_NAME]

	STRUCT_UNION_SPECI = [struct union] ID(opt) { struct-declaration-list } |
				[struct union] ID

	struct-declaration-list-rest = struct-declaration struct-declaration-list-rest | NIL
	struct-declaration-list = struct-declaration struct-declaration-list-rest

	struct-declaration = specifier-qualifier-list struct-declarator-list ';'

	specifier-qualifier-list = [type-specifier type-qualifier] specifier-qualifier-list(opt)

	struct-declarator-list-rest = ',' struct-declarator struct-declarator-list-rest | NIL
	struct-declarator-list = struct-declarator struct-declarator-list-rest

	struct-declarator = declarator | declarator(opt) : constant expression

	enum-specifier = enum identifier | enum identifier(opt) '{' enumerator-list '}'

	enumerator-list-rest = ',' enumerator enumerator-list-rest | NIL
	enumerator-list = enumerator enumerator-list

	enumerator = enumeration-constant | enumeration-constant '=' constant-expression

	TYPE_QUALIFIER = [const volatile]

	DECL = pointer(opt) direct-declarator

	direct-declarator-rest = '[' constant-expression(opt) ']' direct-declarator-rest |
				'(' parameter-type-list ')' direct-declarator-rest |
				'(' identifier-list(opt) ')' direct-declarator-rest |
				NIL
	direct-declarator = identifier direct-declarator-rest | '(' declarator ')' direct-declarator-rest

	pointer = '*' type-qualifier(opt) | '*' type-qualifier(opt) pointer

	parameter-type-list = parameter-list | parameter-list ', ...';

	parameter-list-rest = ',' parameter-declaration parameter-list-rest | NIL
	parameter-list = parameter-declaration parameter-list-rest

	parameter-declaration = declaration-specifiers declaration |
				declaration-specifiers abstract-declarator(opt)

	identifier-list-rest = ',' identifier identifier-list-rest | NIL
	identifier-list = identifier identifier-list-rest

	typename = specifier-qualifier-list abstract-declarator(opt)
	abstract-declarator = pointer | pointer(opt) direct-abstract-declarator


	direct-abstract-declarator-rest = '[' constant-expression(opt) ']' direct-abstract-declarator-rest |
					'(' parameter-type-list(opt) ']' direct-abstract-declarator-rest |
					NIL
	direct-abstract-declarator = '(' abstract-declarator ')' direct-abstract-declarator-rest

	typedef-name = identifier

	initializer = assignment-expression | '{' inintializer-list '}' | '{' initializer-list ',' '}'
	initializer-list-rest = ',' initializer initalizer-list-rest | NIL
	initializer-list = initializer initializer-list-rest
*/

/* STATEMENT
	STATEMENT = LABELED_STATEMENT | COMPOUND_STATEMENT |
		EXPRESSION_STATEMENT | SELECTION_STATEMENT |
		ITERATION_STATEMENT | JUMP_STATEMENT

	LABELED_S = ID ':' STATEMENT | case CONST_EXPR ':' STATEMENT | default ':' STATEMENT

	COMPOUND_STATEMENT = '{' DECLARATION_LIST(opt) STATEMENT_LIST(opt) '}'


	// DECLARATION_LIST = DECLARATION | DECLARATION_LIST DECLARATION
	//
	DECLARATION_LIST_REST = DECLARATION DECLARATION_LIST_REST | NIL
	DECLARATION_LIST = DECLARATION DECLARATION_LIST_REST

	// STATEMENT_LIST = STATEMENT | STATEMENT_LIST STATEMENT
	//
	STATEMENT_LIST_REST = STATEMENT STATEMENT_LIST_REST | NIL
	STATEMENT_LIST = STATEMENT STATEMENT_LIST_REST

	EXPRESSION_STATEMENT = EXPRESSION(opt) ';'

	SELECTION_STATEMENT = if '(' EXPRESSION ')' STATEMENT |
				if '(' EXPRESSION ')' STATEMENT else STATEMENT |
				switch '(' EXPRESSION ')' STATEMENT

	ITERATION_STATEMENT = while '(' EXPRESSION ')' STATEMENT |
				do STATEMENT while '(' EXPRESSION ')' ';' |
				for '(' EXPRESSION(opt) ';' EXPRESSION(opt) ';' EXPRESSION(opt) ')' STATEMENT

	JUMP_STATEMENT = goto identifier ';' | continue ';' | break ';' | return EXPRESSION(opt) ';'
*/

/* EXTERNAL_DEFINITION
	TRANSLATION_UNIT =
		EXTERNAL_DECLARATION TRANSLATION_UNIT_REST
	TRANSLATION_UNIT_REST = EXTERNAL_DECLARATION TRANSLATION_UNIT_REST | NIL
	EXTERNAL_DECLARATION = FUNCTION_DEFINITION | DECLARATION
	FUNCTION_DEFINITION = DECLARATION_SPECIFIER(opt) DECLARATOR
				DECLARATION_LIST(opt) COMPOUND_STATEMENT
*/


#define GREEN "\x1b[;32;1m"
#define RED "\x1b[;31;1m"
#define CYAN "\x1b[;36;1m"
#define RESET "\x1b[0m"

static void panic(const char *msg)
{
	fprintf(stdout, "parser:%zu:%zu: " RED "error" RESET ": %s\n ",
		gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column, msg);

	print_line(gl_queue->back(gl_queue).line);
	fprintf(stdout, "\n ");

	shift_line(gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	fprintf(stdout, RED "\b^" GREEN);
	print_token_underline(gl_queue->back(gl_queue).type);
	fprintf(stdout, "\n ");

	shift_line(gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);

	fprintf(stdout, "\b%s\n" RESET, msg);

	exit(1);
}

static int match_and_pop(enum token c)
{
	if (gl_queue->back(gl_queue).type != c) {
		fprintf(stdout, "parser:%zu:%zu: " RED "error" RESET ": expected \"",
			gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		print_token(c);
		fprintf(stdout, "\" before \"");
		print_token(gl_queue->back(gl_queue).type);
		fprintf(stdout, "\"\n ");

		print_line(gl_queue->back(gl_queue).line);
		fprintf(stdout, "\n ");

		shift_line(gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		fprintf(stdout, RED "\b^" GREEN);
		print_token_underline(gl_queue->back(gl_queue).type);
		fprintf(stdout, "\n ");

		shift_line(gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);

		fprintf(stdout, "\b");
		print_token(c);
		fprintf(stdout, "\n" RESET);
		exit(1);
	}
	gl_queue->pop(gl_queue);
	next();
	return 0;
}

struct identifier {
	size_t dline; /* declare line */
	size_t dcolumn; /* declare column */
	char *name;
};

// BKDR Hash Function
unsigned int parser_hash(char const *str)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

static void print_underline(size_t time)
{
	for (size_t i = 0; i < time; ++i) {
		fputc('~', stdout);
	}
}

static void panic_redefine(struct ast_node *nd, struct identifier *v)
{
	fprintf(stdout, "parser:%zu:%zu: " RED "error" RESET ": %s was redeclared.\n ",
		nd->dline, nd->dcolumn, v->name);

	print_line(nd->dline);
	fprintf(stdout, "\n " RED);

	shift_line(nd->dline, nd->dcolumn);
	print_underline(strlen(v->name));

	fprintf(stdout, RESET "\nparser:%zu:%zu: " CYAN "note" RESET ": %s was declared here.\n ",
		v->dline, v->dcolumn, v->name);
	print_line(v->dline);
	fprintf(stdout, "\n " RED);

	shift_line(v->dline, v->dcolumn);
	print_underline(strlen(v->name));
	fprintf(stdout, "\n" RESET);
	exit(1);
}

static void panic_not_typedef_name(const char *name)
{
	fprintf(stdout, "parser:%zu:%zu: " RED "error" RESET ": %s is not a typename.\n ",
		gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column, name);

	print_line(gl_queue->back(gl_queue).line);
	fprintf(stdout, "\n " RED);

	shift_line(gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	print_underline(strlen(name));
	fprintf(stdout, "\n" RESET);
	exit(1);
}

static mapstack_t typedef_mapstack;
static void insert_id(struct ast_node *nd)
{
	unsigned int hash = parser_hash(nd->ptr);
	struct identifier *value = (struct identifier *)typedef_mapstack->search_cur(typedef_mapstack, hash);
	if (value != NULL) {
		panic_redefine(nd, value);
		exit(1); /* never be here */
	}
	struct identifier *id = (struct identifier *)calloc(1, sizeof(struct identifier));
	id->dline = nd->dline;
	id->dcolumn = nd->dcolumn;
	id->name = nd->ptr;

	typedef_mapstack->insert(typedef_mapstack, hash, id);
}

static int typedef_name_check(const char *str)
{
	unsigned int hash = parser_hash(str);
	return typedef_mapstack->exist(typedef_mapstack, hash);
}

#undef GREEN
#undef RED
#undef CYAN
#undef RESET

static ast_t *identifier(void)
{
	ast_t *node = new_node(AS_ID, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	node->ptr = gl_queue->back(gl_queue).str;

	match_and_pop(ID);
	return node;
}

static ast_t *constant(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case CHAR_CONST:
		node = new_node(AS_INT_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		node->i = gl_queue->back(gl_queue).i_data;
		match_and_pop(CHAR_CONST);
		return node;
	case INT_CONST:
		node = new_node(AS_INT_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		node->i = gl_queue->back(gl_queue).i_data;
		match_and_pop(INT_CONST);
		return node;
	case UINT_CONST:
		node = new_node(AS_UINT_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		node->ui = gl_queue->back(gl_queue).ui_data;
		match_and_pop(UINT_CONST);
		return node;
	case ULONG_CONST:
		node = new_node(AS_ULONG_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		node->uli = gl_queue->back(gl_queue).uli_data;
		match_and_pop(ULONG_CONST);
		return node;
	case LONG_CONST:
		node = new_node(AS_LONG_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		node->li = gl_queue->back(gl_queue).li_data;
		match_and_pop(LONG_CONST);
		return node;
	default:
		match_and_pop(INT_CONST);
		return NULL; /* SHOULD NOT BE HERE */
	}
}

static ast_t *string(void)
{
	ast_t *node = new_node(AS_STR_LIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	node->ptr = gl_queue->back(gl_queue).str;
	match_and_pop(STRING);
	return node;
}

static int is_specifier_qualifier(token_t tk)
{
	switch (tk.type) {
	case K_VOID:
	case K_CHAR:
	case K_SHORT:
	case K_INT:
	case K_LONG:
	case K_FLOAT:
	case K_DOUBLE:
	case K_SIGNED:
	case K_UNSIGNED:
	case K_CONST:
	case K_VOLATILE:
	case K_STRUCT:
	case K_UNION:
	case K_ENUM:
		return 1;
	case ID:
		return typedef_name_check(tk.str);
	default:
		return 0;
	}
}

static int is_declaration_specifier(token_t tk)
{
	if (is_specifier_qualifier(tk)) {
		return 1;
	}
	switch (tk.type) {
	case K_TYPEDEF:
	case K_EXTERN:
	case K_STATIC:
	case K_AUTO:
	case K_REGISTER:
		return 1;
	default:
		return 0;
	}
}

/******************************************************
 *
 *
 * EXPRESSION START
 *
 *
 ******************************************************/
static ast_t *expr(void);

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
	case STRING:
		return string();
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
		node = new_node(AS_ARGU_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_INDEX, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pp);
		add_son(node, expr());
		match_and_pop(R_BRACK);
		return post_rest(node);
	case L_PARA:
		match_and_pop(L_PARA);
		node = new_node(AS_FUNC_CALL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pp);
		if (!(gl_queue->back(gl_queue).type == R_PARA)) {
			add_son(node, argu_list());
		}
		match_and_pop(R_PARA);
		return post_rest(node);
	case MEMBER:
		match_and_pop(MEMBER);
		node = new_node(AS_MEMBER, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pp);
		add_son(node, identifier());
		return post_rest(node);
	case PTR_MEMBER:
		match_and_pop(PTR_MEMBER);
		node = new_node(AS_PTR_MEMBER, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pp);
		add_son(node, identifier());
		return post_rest(node);
	case INC:
		match_and_pop(INC);
		node = new_node(AS_POST_INC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pp);
		return post_rest(node);
	case DEC:
		match_and_pop(DEC);
		node = new_node(AS_POST_DEC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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

static ast_t *typename(void);
static ast_t *cast(void);
static ast_t *unary(void)
{
	ast_t *node;
	ast_t *ret;
	ret = new_node(AS_UNARY, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column); /* FOR ASSIGNMENT */
	switch(gl_queue->back(gl_queue).type) {
	case INC:
		match_and_pop(INC);
		node = new_node(AS_PRE_INC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, unary());
		break;
	case DEC:
		match_and_pop(DEC);
		node = new_node(AS_PRE_DEC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, unary());
		break;
	case PLUS:
		match_and_pop(PLUS);
		node = new_node(AS_POS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case MINUS:
		match_and_pop(MINUS);
		node = new_node(AS_NEG, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case AND:
		match_and_pop(AND);
		node = new_node(AS_ADDRESS_OF, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case MUL:
		match_and_pop(MUL);
		node = new_node(AS_VALUE_OF, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case COMPLEMENT:
		match_and_pop(COMPLEMENT);
		node = new_node(AS_COMPLEMENT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case NOT:
		match_and_pop(NOT);
		node = new_node(AS_NOT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cast());
		break;
	case SIZEOF:
		match_and_pop(SIZEOF);
		node = new_node(AS_SIZEOF, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		switch (gl_queue->back(gl_queue).type) {
		case L_PARA:
			next();
			if (is_specifier_qualifier(gl_queue->back_count(gl_queue, 1))) {
				match_and_pop(L_PARA);
				add_son(node, typename());
				match_and_pop(R_PARA);
			} else {
				add_son(node, unary());
			}
			break;
		default:
			add_son(node, unary());
			break;
		}
		break;
	default:
		node = post();
		break;
	}
	return add_son(ret, node);
}

static ast_t *cast(void)
{
	ast_t *node;
	switch(gl_queue->back(gl_queue).type) {
	case L_PARA:
		next();
		if (is_specifier_qualifier(gl_queue->back_count(gl_queue, 1))) {
			node = new_node(AS_CAST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
			match_and_pop(L_PARA);
			add_son(node, typename());
			match_and_pop(R_PARA);
			add_son(node, cast());
			return node;
		}
		/* fall through */
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
		node = new_node(AS_MUL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cc);
		add_son(node, cast());
		return mul_rest(node);
	case DIVIDE:
		match_and_pop(DIVIDE);
		node = new_node(AS_DIVIDE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cc);
		add_son(node, cast());
		return mul_rest(node);
	case MOD:
		match_and_pop(MOD);
		node = new_node(AS_MOD, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_ADD, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, mm);
		add_son(node, mul());
		return add_rest(node);
	case MINUS:
		match_and_pop(MINUS);
		node = new_node(AS_MINUS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_SHL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, aa);
		add_son(node, add());
		return shift_rest(node);
	case SHR:
		match_and_pop(SHR);
		node = new_node(AS_SHR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_LT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case GT:
		match_and_pop(GT);
		node = new_node(AS_GT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case LE:
		match_and_pop(LE);
		node = new_node(AS_LE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ss);
		add_son(node, shift());
		return rela_rest(node);
	case GE:
		match_and_pop(GE);
		node = new_node(AS_GE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_EQ, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, rr);
		add_son(node, rela());
		return equ_rest(node);
	case NEQ:
		match_and_pop(NEQ);
		node = new_node(AS_NEQ, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_AND, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_XOR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_OR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_LOG_AND, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_LOG_OR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_CONDI, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, lor);
		add_son(node, expr());
		match_and_pop(COLON);
		return add_son(node, condi());
	default:
		return lor;
	}
}

static ast_t *const_expr(void)
{
	/* TODO: CHECK IF THIS IS REAL CONSTEXPR */
	ast_t *node = new_node(AS_CONSTEXPR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, condi());
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
			panic("L-Value should be a unary expression");
			exit(1);
		}
		break;
	default:
		break;
	}

	switch (gl_queue->back(gl_queue).type) {
	case ASS:
		match_and_pop(ASS);
		node = new_node(AS_ASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case MULASS:
		match_and_pop(MULASS);
		node = new_node(AS_MULASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case DIVASS:
		match_and_pop(DIVASS);
		node = new_node(AS_DIVASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case MODASS:
		match_and_pop(MODASS);
		node = new_node(AS_MODASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case ADDASS:
		match_and_pop(ADDASS);
		node = new_node(AS_ADDASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case SUBASS:
		match_and_pop(SUBASS);
		node = new_node(AS_SUBASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case SHLASS:
		match_and_pop(SHLASS);
		node = new_node(AS_SHLASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case SHRASS:
		match_and_pop(SHRASS);
		node = new_node(AS_SHRASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case ANDASS:
		match_and_pop(ANDASS);
		node = new_node(AS_ANDASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case XORASS:
		match_and_pop(XORASS);
		node = new_node(AS_XORASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, cnd);
		return add_son(node, assign());
	case ORASS:
		match_and_pop(ORASS);
		node = new_node(AS_ORASS, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
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
		node = new_node(AS_COMMA, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, aa);
		add_son(node, assign());
		return expr_rest(node);
	default:
		return aa;
	}
}

static ast_t *expr(void)
{
	ast_t *node = new_node(AS_EXPR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, expr_rest(assign()));
}

/******************************************************
 *
 * EXPRESSION END
 *
 * DECLARATION START 
 *
 ******************************************************/
static ast_t *initializer(void);
static ast_t *initializer_list_rest(ast_t *ii)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_INITIALIZER_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ii);
		add_son(node, initializer());
		return initializer_list_rest(node);
	default:
		return ii;
	}
}

static ast_t *initializer_list(void)
{
	ast_t *node = new_node(AS_INITIALIZER_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, initializer_list_rest(initializer()));
}

static ast_t *initializer(void)
{
	ast_t *node = new_node(AS_INITIALIZER, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	switch (gl_queue->back(gl_queue).type) {
	case L_CURLY:
		match_and_pop(L_CURLY);
		add_son(node, initializer_list());
		if (gl_queue->back(gl_queue).type == COMMA) {
			match_and_pop(COMMA);
		}
		match_and_pop(R_CURLY);
		return node;
	default:
		add_son(node, assign());
		return node;
	}
}

static ast_t *typedef_name(void)
{
	ast_t *node = new_node(AS_TYPEDEF_NAME, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, identifier());
}

static ast_t *parameter_type_list(void);
static ast_t *direct_abstract_declarator_rest(ast_t *aa)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case L_BRACK:
		node = new_node(AS_DIRECT_ABSTRACT_DECL_ARRAY, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		match_and_pop(L_BRACK);
		add_son(node, aa);
		if (gl_queue->back(gl_queue).type != R_BRACK) {
			add_son(node, const_expr());
		}
		match_and_pop(R_BRACK);
		return direct_abstract_declarator_rest(node);
	case L_PARA:
		node = new_node(AS_DIRECT_ABSTRACT_DECL_FUNC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		match_and_pop(L_PARA);
		add_son(node, aa);
		if (gl_queue->back(gl_queue).type != R_PARA) {
			add_son(node, parameter_type_list());
		}
		match_and_pop(R_PARA);
		return direct_abstract_declarator_rest(node);
	default:
		return aa;
	}
}

static ast_t *abstract_declarator(void);
static ast_t *direct_abstract_declarator(void)
{
	ast_t *node = new_node(AS_DIRECT_ABSTRACT_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(L_PARA);
	add_son(node, abstract_declarator());
	match_and_pop(R_PARA);
	return direct_abstract_declarator_rest(node);
}

static ast_t *pointer(void);
static ast_t *abstract_declarator(void)
{
	ast_t *node = new_node(AS_ABSTRACT_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	if (gl_queue->back(gl_queue).type == MUL) {
		add_son(node, pointer());
		if (gl_queue->back(gl_queue).type == L_PARA) {
			return add_son(node, direct_abstract_declarator());
		} else {
			return node;
		}
	} else {
		return add_son(node, direct_abstract_declarator());
	}
}

static ast_t *struct_union_specifier(void);
static ast_t *enum_specifier(void);
static ast_t *specifier_qualifier_list(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case K_VOID:
		match_and_pop(K_VOID);
		node = new_node(AS_DECL_VOID, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_CHAR:
		match_and_pop(K_CHAR);
		node = new_node(AS_DECL_CHAR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_SHORT:
		match_and_pop(K_SHORT);
		node = new_node(AS_DECL_SHORT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_INT:
		match_and_pop(K_INT);
		node = new_node(AS_DECL_INT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_LONG:
		match_and_pop(K_LONG);
		node = new_node(AS_DECL_LONG, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_FLOAT:
		match_and_pop(K_FLOAT);
		node = new_node(AS_DECL_FLOAT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_DOUBLE:
		match_and_pop(K_DOUBLE);
		node = new_node(AS_DECL_DOUBLE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_SIGNED:
		match_and_pop(K_SIGNED);
		node = new_node(AS_DECL_SIGNED, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_UNSIGNED:
		match_and_pop(K_UNSIGNED);
		node = new_node(AS_DECL_UNSIGNED, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_CONST:
		match_and_pop(K_CONST);
		node = new_node(AS_DECL_CONST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_VOLATILE:
		match_and_pop(K_VOLATILE);
		node = new_node(AS_DECL_VOLATILE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;

	case K_STRUCT:
	case K_UNION:
		node = struct_union_specifier();
		break;

	case K_ENUM:
		node = enum_specifier();
		break;
	case ID:
		if (typedef_name_check(gl_queue->back(gl_queue).str)) {
			match_and_pop(ID);
			node = new_node(AS_DECL_TYPENAME, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
			break;
		}
		panic_not_typedef_name(gl_queue->back(gl_queue).str);
		exit(1); /* never be here */
	default:
		panic("cannot find declaration specifier");
		exit(1); /* SHOULD NOT BE HERE */
	}
	if (is_specifier_qualifier(gl_queue->back(gl_queue))) {
		return add_son(node, specifier_qualifier_list());
	} else {
		return node;
	}
}

static ast_t *typename(void)
{
	ast_t *node = new_node(AS_TYPENAME, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, specifier_qualifier_list());
	switch (gl_queue->back(gl_queue).type) {
	case MUL:
	case L_PARA:
	/* ABSTRACT DECLARATOR */
		return add_son(node, abstract_declarator());
	default:
	/* WITHOUT ABSTRACT DECLARATOR */
		return node;
	}
}

static ast_t *identifier_list_rest(ast_t *ii)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_ID_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ii);
		add_son(node, identifier());
		return identifier_list_rest(node);
	default:
		return ii;
	}
}

static ast_t *identifier_list(void)
{
	ast_t *node = new_node(AS_ID_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, identifier_list_rest(identifier()));
}

static ast_t *decl(void);
static ast_t *declaration_specifier(void);
static ast_t *parameter_declaration(void)
{
	ast_t *node = new_node(AS_PARA_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, declaration_specifier());
	unsigned long i;
	for (i = 0; gl_queue->back_count(gl_queue, i).type == L_PARA ||
			gl_queue->back_count(gl_queue, i).type == MUL; ++i) {
		next();
	}
	if (gl_queue->back_count(gl_queue, i).type == ID) {
		return add_son(node, decl());
	}
	switch (gl_queue->back(gl_queue).type) {
	/* DECLARATOR */
	case MUL:
	case L_PARA:
	/* ABSTRACT DECLARATOR */
		return add_son(node, abstract_declarator());
	default:
	/* WITHOUT ABSTRACT DECLARATOR */
		return node;
	}
}

static ast_t *parameter_list_rest(ast_t *pdd)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		next();
		if (gl_queue->back_count(gl_queue, 1).type == MEMBER) {
			return pdd;
		}
		match_and_pop(COMMA);
		node = new_node(AS_PARA_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, pdd);
		add_son(node, parameter_declaration());
		return parameter_list_rest(node);
	default:
		return pdd;
	}
}

static ast_t *parameter_list(void)
{
	ast_t *node = new_node(AS_PARA_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	return add_son(node, parameter_list_rest(parameter_declaration()));
}

static ast_t *parameter_type_list(void)
{
	ast_t *node;
	ast_t *pp;
	pp = parameter_list();
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		match_and_pop(MEMBER);
		match_and_pop(MEMBER);
		match_and_pop(MEMBER);
		node = new_node(AS_DECL_PARA_TYPE_LIST_VLEN, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		return add_son(node, pp);
	default:
		node = new_node(AS_DECL_PARA_TYPE_LIST_NORM, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		return add_son(node, pp);
	}
}

static ast_t *pointer(void)
{
	ast_t *node;
	match_and_pop(MUL);
	switch (gl_queue->back(gl_queue).type) {
	case K_CONST:
		match_and_pop(K_CONST);
		node = new_node(AS_DECL_CONST_PTR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_VOLATILE:
		match_and_pop(K_VOLATILE);
		node = new_node(AS_DECL_VOLATILE_PTR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	default:
		node = new_node(AS_DECL_PTR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	}
	if (gl_queue->back(gl_queue).type == MUL) {
		return add_son(node, pointer());
	} else {
		return node;
	}
}

static ast_t *direct_declarator_rest(ast_t *iidd)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case L_BRACK:
		match_and_pop(L_BRACK);
		node = new_node(AS_DECL_ARRAY, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, iidd);
		if (gl_queue->back(gl_queue).type != R_BRACK) {
			add_son(node, const_expr());
		}
		match_and_pop(R_BRACK);
		return direct_declarator_rest(node);
	case L_PARA:
		match_and_pop(L_PARA);
		node = new_node(AS_DECL_FUNC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, iidd);
		switch (gl_queue->back(gl_queue).type) {
		case ID:
			add_son(node, identifier_list());
			/* FALL THROUGH */
		case R_PARA:
			match_and_pop(R_PARA);
			return direct_declarator_rest(node);
		default:
			add_son(node, parameter_type_list());
			match_and_pop(R_PARA);
			return direct_declarator_rest(node);
		}
	default:
		return iidd;
	}
}

static ast_t *direct_declarator(void)
{
	ast_t *node = new_node(AS_DIRECT_DECLARATOR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	switch (gl_queue->back(gl_queue).type) {
	case ID:
		return direct_declarator_rest(add_son(node, identifier()));
	case L_PARA:
		match_and_pop(L_PARA);
		add_son(node, decl());
		match_and_pop(R_PARA);
		return direct_declarator_rest(node);
	default:
		panic("declaration specifier not found");
		exit(1);
	}
}

static ast_t *decl(void)
{
	ast_t *node = new_node(AS_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	switch (gl_queue->back(gl_queue).type) {
	case MUL: /* POINTER */
		add_son(node, pointer());
		break;
	default:
		break;
	}
	return add_son(node, direct_declarator());
}

static ast_t *struct_declarator(void)
{
	ast_t *node = new_node(AS_STRUCT_DECLARATOR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	switch (gl_queue->back(gl_queue).type) {
	case COLON:
		match_and_pop(COLON);
		add_son(node, const_expr());
		return node;
	default:
		add_son(node, decl());
		if (gl_queue->back(gl_queue).type == COLON) {
			match_and_pop(COLON);
			add_son(node, const_expr());
		}
		return node;
	}
}

// ENUMERATION_CONST := ID
static ast_t *enumerator(void)
{
	ast_t *node = new_node(AS_ENUMERATOR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, identifier()); // enumeration-constant
	if (gl_queue->back(gl_queue).type == ASS) {
		match_and_pop(ASS);
		add_son(node, const_expr());
	}
	return node;
	
}

static ast_t *enumerator_list_rest(ast_t *ee)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_ENUM_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, enumerator());
		return enumerator_list_rest(node);
	default:
		return ee;
	}
}

static ast_t *enumerator_list(void)
{
	ast_t *node = new_node(AS_ENUM_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, enumerator());
	return enumerator_list_rest(node);
}

static ast_t *enum_specifier(void)
{
	ast_t *node = new_node(AS_DECL_ENUM, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_ENUM);
	switch (gl_queue->back(gl_queue).type) {
	case ID:
		add_son(node, identifier());
		break;
	default:
		match_and_pop(L_CURLY);
		add_son(node, enumerator_list());
		match_and_pop(R_CURLY);
		return node;
	}
	if (gl_queue->back(gl_queue).type == L_CURLY) {
		match_and_pop(L_CURLY);
		add_son(node, enumerator_list());
		match_and_pop(R_CURLY);
	}
	return node;
}

static ast_t *struct_declarator_list_rest(ast_t *ss)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_STRUCT_DECLARATOR_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, struct_declarator());
		return struct_declarator_list_rest(node);
	default:
		return ss;
	}
}

static ast_t *struct_declarator_list(void)
{
	ast_t *node = new_node(AS_STRUCT_DECLARATOR_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, struct_declarator());
	return struct_declarator_list_rest(node);
}

static ast_t *struct_declaration(void)
{
	ast_t *node = new_node(AS_STRUCT_DECLARATION, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, specifier_qualifier_list());
	add_son(node, struct_declarator_list());
	match_and_pop(SEMI);
	return node;
}

static ast_t *struct_declaration_list_rest(ast_t *ss)
{
	ast_t *node;
	if (is_specifier_qualifier(gl_queue->back(gl_queue))) {
		node = new_node(AS_STRUCT_DECLARATION_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, struct_declaration());
		return struct_declaration_list_rest(node);
	} else {
		return ss;
	}
}

static ast_t *struct_declaration_list(void)
{
	ast_t *node = new_node(AS_STRUCT_DECLARATION_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, struct_declaration());
	return struct_declaration_list_rest(node);
}

static ast_t *init_decl(void)
{
	ast_t *node = new_node(AS_INIT_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, decl());
	switch (gl_queue->back(gl_queue).type) {
	case ASS:
		match_and_pop(ASS);
		return add_son(node, initializer());
	default:
		return node;
	}
}

static ast_t *init_decl_list_rest(ast_t *ii)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case COMMA:
		match_and_pop(COMMA);
		node = new_node(AS_INIT_DECL_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ii);
		add_son(node, init_decl());
		return init_decl_list_rest(node);
	default:
		node = new_node(AS_INIT_DECL_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		return add_son(node, ii);
	}
}

static ast_t *init_decl_list(void)
{
	return init_decl_list_rest(init_decl());
}

static ast_t *struct_union_specifier(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case K_STRUCT:
		node = new_node(AS_DECL_STRUCT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		match_and_pop(K_STRUCT);
		break;
	case K_UNION:
		node = new_node(AS_DECL_UNION, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		match_and_pop(K_UNION);
		break;
	default:
		match_and_pop(K_STRUCT);
		break; /* SHOULD NOT BE HERE */
	}
	switch (gl_queue->back(gl_queue).type) {
	case ID:
		add_son(node, identifier());
		break;
	default:
		match_and_pop(L_CURLY);
		add_son(node, struct_declaration_list());
		match_and_pop(R_CURLY);
		return node;
	}
	if (gl_queue->back(gl_queue).type == L_CURLY) {
		match_and_pop(L_CURLY);
		add_son(node, struct_declaration_list());
		match_and_pop(R_CURLY);
	}
	return node;
}

static ast_t *declaration_specifier(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case K_TYPEDEF:
		match_and_pop(K_TYPEDEF);
		node = new_node(AS_DECL_TYPEDEF, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_EXTERN:
		match_and_pop(K_EXTERN);
		node = new_node(AS_DECL_EXTERN, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_STATIC:
		match_and_pop(K_STATIC);
		node = new_node(AS_DECL_STATIC, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_AUTO:
		match_and_pop(K_AUTO);
		node = new_node(AS_DECL_AUTO, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_REGISTER:
		match_and_pop(K_REGISTER);
		node = new_node(AS_DECL_REGISTER, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_VOID:
		match_and_pop(K_VOID);
		node = new_node(AS_DECL_VOID, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_CHAR:
		match_and_pop(K_CHAR);
		node = new_node(AS_DECL_CHAR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_SHORT:
		match_and_pop(K_SHORT);
		node = new_node(AS_DECL_SHORT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_INT:
		match_and_pop(K_INT);
		node = new_node(AS_DECL_INT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_LONG:
		match_and_pop(K_LONG);
		node = new_node(AS_DECL_LONG, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_FLOAT:
		match_and_pop(K_FLOAT);
		node = new_node(AS_DECL_FLOAT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_DOUBLE:
		match_and_pop(K_DOUBLE);
		node = new_node(AS_DECL_DOUBLE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_SIGNED:
		match_and_pop(K_SIGNED);
		node = new_node(AS_DECL_SIGNED, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_UNSIGNED:
		match_and_pop(K_UNSIGNED);
		node = new_node(AS_DECL_UNSIGNED, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_CONST:
		match_and_pop(K_CONST);
		node = new_node(AS_DECL_CONST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;
	case K_VOLATILE:
		match_and_pop(K_VOLATILE);
		node = new_node(AS_DECL_VOLATILE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		break;

	case K_STRUCT:
	case K_UNION:
		node = struct_union_specifier();
		break;

	case K_ENUM:
		node = enum_specifier();
		break;

	case ID:
		if (typedef_name_check(gl_queue->back(gl_queue).str)) {
			match_and_pop(ID);
			node = new_node(AS_DECL_TYPENAME, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
			break;
		}
		panic_not_typedef_name(gl_queue->back(gl_queue).str);
		exit(1); /* SHOULD NOT BE HERE */
	default:
		panic("cannot find declaration specifier");
		exit(1); /* SHOULD NOT BE HERE */
	}
	if (is_declaration_specifier(gl_queue->back(gl_queue))) {
		return add_son(node, declaration_specifier());
	} else {
		return node;
	}
}

static ast_t *declaration(void)
{
	ast_t *node = new_node(AS_DECLARATION, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, declaration_specifier());
	switch (gl_queue->back(gl_queue).type) {
	case ID:
	case MUL:
	case L_PARA:
		add_son(node, init_decl_list());
		if (exist_ast(node->son_array[0], AS_DECL_TYPEDEF)) {
			iterate_type_ast(node->son_array[1], AS_ID, insert_id);
		}
		break;
	default:
		break;
	}
	match_and_pop(SEMI);
	return node;
}

/******************************************************
 *
 * DECLARATION END
 *
 * STATEMENT START
 *
 ******************************************************/
static ast_t *goto_jump(void)
{
	ast_t *node = new_node(AS_JUMP_GOTO, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_GOTO);
	add_son(node, identifier());
	match_and_pop(SEMI);
	return node;
}

static ast_t *continue_jump(void)
{
	ast_t *node = new_node(AS_JUMP_CONTINUE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_CONTINUE);
	match_and_pop(SEMI);
	return node;
}

static ast_t *break_jump(void)
{
	ast_t *node = new_node(AS_JUMP_BREAK, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_BREAK);
	match_and_pop(SEMI);
	return node;
}

static ast_t *return_jump(void)
{
	ast_t *node = new_node(AS_JUMP_RETURN, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_RETURN);
	if (gl_queue->back(gl_queue).type != SEMI) {
		add_son(node, expr());
	}
	match_and_pop(SEMI);
	return node;
}

static ast_t *statement(void);
static ast_t *while_iter(void)
{
	ast_t *node = new_node(AS_WHILE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_WHILE);
	match_and_pop(L_PARA);
	add_son(node, expr());
	match_and_pop(R_PARA);
	add_son(node, statement());
	return node;
}

static ast_t *do_while_iter(void)
{
	ast_t *node = new_node(AS_DO_WHILE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_DO);
	add_son(node, statement());
	match_and_pop(K_WHILE);
	match_and_pop(L_PARA);
	add_son(node, expr());
	match_and_pop(R_PARA);
	match_and_pop(SEMI);
	return node;
}

static ast_t *for_iter(void)
{
	ast_t *node = new_node(AS_FOR, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_FOR);
	match_and_pop(L_PARA);
	if (gl_queue->back(gl_queue).type != SEMI) {
		add_son(node, expr());
	}
	match_and_pop(SEMI);
	if (gl_queue->back(gl_queue).type != SEMI) {
		add_son(node, expr());
	}
	match_and_pop(SEMI);
	if (gl_queue->back(gl_queue).type != R_PARA) {
		add_son(node, expr());
	}
	match_and_pop(R_PARA);
	add_son(node, statement());
	return node;
}

static ast_t *if_select(void)
{
	ast_t *node = new_node(AS_IF, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_IF);
	match_and_pop(L_PARA);
	add_son(node, expr());
	match_and_pop(R_PARA);
	add_son(node, statement());
	if (gl_queue->back(gl_queue).type == K_ELSE) {
		match_and_pop(K_ELSE);
		add_son(node, statement());
	}
	return node;
}

static ast_t *switch_select(void)
{
	ast_t *node = new_node(AS_SWITCH, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_SWITCH);
	match_and_pop(L_PARA);
	add_son(node, expr());
	match_and_pop(R_PARA);
	add_son(node, statement());
	return node;
}

static ast_t *declaration_list_rest(ast_t *dd)
{
	ast_t *node = new_node(AS_DECLARATION_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, dd);
	if (is_declaration_specifier(gl_queue->back(gl_queue))) {
	/* next is declaration */
		add_son(node, declaration());
		return declaration_list_rest(node);
	} else {
		return node;
	}
}

static ast_t *declaration_list(void)
{
	return declaration_list_rest(declaration());
}

static ast_t *statement_list_rest(ast_t *ss)
{
	ast_t *node = new_node(AS_STATEMENT_LIST, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, ss);
	switch (gl_queue->back(gl_queue).type) {
	case R_CURLY:
	case END:
		return node;
	default:
		return statement_list_rest(add_son(node, statement()));
	}
}

static ast_t *statement_list(void)
{
	return statement_list_rest(statement());
}

static ast_t *compound(void)
{
	ast_t *node = new_node(AS_COMPOUND, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);

	typedef_mapstack->push(typedef_mapstack);

	match_and_pop(L_CURLY);
	/* DECLARATION : DECLARATION SPECIFIER */
	if (is_declaration_specifier(gl_queue->back(gl_queue))) {
		add_son(node, declaration_list());
	}

	if (gl_queue->back(gl_queue).type != R_CURLY) {
		add_son(node, statement_list());
	}
	match_and_pop(R_CURLY);

	typedef_mapstack->pop(typedef_mapstack);

	return node;
}

static ast_t *case_label(void)
{
	ast_t *node = new_node(AS_LABEL_CASE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_CASE);
	add_son(node, const_expr());
	match_and_pop(COLON);
	add_son(node, statement());
	return node;
}

static ast_t *default_label(void)
{
	ast_t *node = new_node(AS_LABEL_DEFAULT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	match_and_pop(K_DEFAULT);
	match_and_pop(COLON);
	add_son(node, statement());
	return node;
}

static ast_t *id_label(void)
{
	ast_t *node = new_node(AS_LABEL_ID, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, identifier());
	match_and_pop(COLON);
	add_son(node, statement());
	return node;
}

static ast_t *expr_state(void)
{
	ast_t *node;
	switch (gl_queue->back(gl_queue).type) {
	case SEMI:
		match_and_pop(SEMI);
		node = new_node(AS_EXPR_NULL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		return node;
	default:
		node = expr();
		match_and_pop(SEMI);
		return node;
	}
}

static ast_t *statement(void)
{
	ast_t *node = new_node(AS_STATE, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	switch (gl_queue->back(gl_queue).type) {
	case K_CASE:
		return add_son(node, case_label());
	case K_DEFAULT:
		return add_son(node, default_label());

	/* COMPOUND_STATEMENT */
	case L_CURLY:
		return add_son(node, compound());

	/* SELECTION_STATEMENT */
	case K_IF:
		return add_son(node, if_select());
	case K_SWITCH:
		return add_son(node, switch_select());

	/* ITERATION_STATEMENT */
	case K_WHILE:
		return add_son(node, while_iter());
	case K_DO:
		return add_son(node, do_while_iter());
	case K_FOR:
		return add_son(node, for_iter());

	/* JUMP STATEMENT */
	case K_GOTO:
		return add_son(node, goto_jump());
	case K_CONTINUE:
		return add_son(node, continue_jump());
	case K_BREAK:
		return add_son(node, break_jump());
	case K_RETURN:
		return add_son(node, return_jump());

	case ID:
		next();
		if (gl_queue->back_count(gl_queue, 1).type == COLON) {
			return add_son(node, id_label());
		}
		/* ELSE FALL THROUGH */
	default:
		return add_son(node, expr_state());
	}
}

/******************************************************
 *
 * STATEMENT END
 *
 * EXTERNAL START
 *
 ******************************************************/

static ast_t *function_definition(void)
{
	ast_t *node = new_node(AS_FUNC_DEFINITION, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	if (is_declaration_specifier(gl_queue->back(gl_queue))) {
		add_son(node, declaration_specifier());
	}

	switch (gl_queue->back(gl_queue).type) {
	case MUL:
	case ID:
	case L_PARA:
		add_son(node, decl());
		break;
	default:
		lexer_token_seek(node->dline, node->dcolumn);
		remove_ast(node);
		return NULL;
	}

	if (is_declaration_specifier(gl_queue->back(gl_queue))) {
		add_son(node, declaration_list());
	}

	if (gl_queue->back(gl_queue).type != L_CURLY) {
		lexer_token_seek(node->dline, node->dcolumn);
		remove_ast(node);
		return NULL;
	}
	add_son(node, compound());
	return node;
}

static ast_t *external_declaration(void)
{
	ast_t *node = new_node(AS_EXTERN_DECL, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	ast_t *try_func = function_definition();
	if (try_func == NULL) {
		return add_son(node, declaration());
	} else {
		return add_son(node, try_func);
	}
}

static ast_t *translation_unit_rest(ast_t *ee)
{
	ast_t *node;
	switch(gl_queue->back(gl_queue).type) {
	case END:
		return ee;
	default:
		node = new_node(AS_TRANSLATION_UNIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
		add_son(node, ee);
		add_son(node, external_declaration());
		return translation_unit_rest(node);
	}
}

ast_t *translation_unit(void)
{
	init_map_stack(&typedef_mapstack);

	ast_t *node = new_node(AS_TRANSLATION_UNIT, NULL, gl_queue->back(gl_queue).line, gl_queue->back(gl_queue).column);
	add_son(node, external_declaration());
	ast_t *ret = translation_unit_rest(node);

	free_map_stack(typedef_mapstack);
	return ret;
}

/******************************************************
 *
 *
 * EXTERNAL END
 *
 *
 ******************************************************/
