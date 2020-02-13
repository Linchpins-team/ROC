#ifndef AST_H_
#define AST_H_
#include <stddef.h>

enum asttype {
	AS_INT_LIT, AS_FLOAT_LIT, AS_STR_LIT, AS_EXPR,
	AS_ADD, AS_MINUS, AS_MUL, AS_DIVIDE, AS_POS,
	AS_NEG, AS_PRIM, AS_INDEX, AS_FUNC_CALL, AS_MEMBER,
	AS_ID, AS_PTR_MEMBER, AS_POST_INC, AS_POST_DEC,
	AS_ARGU_LIST, AS_PRE_INC, AS_PRE_DEC, AS_ADDRESS_OF,
	AS_VALUE_OF, AS_COMPLEMENT, AS_NOT, AS_SIZEOF,
	AS_MOD, AS_SHL, AS_SHR, AS_LT, AS_GT, AS_LE, AS_GE,
	AS_EQ, AS_NEQ, AS_AND, AS_XOR, AS_OR,
	AS_LOG_OR, AS_LOG_AND, AS_CONDI, AS_UNARY,
	AS_ASS, AS_MULASS, AS_DIVASS, AS_MODASS,
	AS_ADDASS, AS_SUBASS, AS_SHLASS, AS_SHRASS,
	AS_ANDASS, AS_XORASS, AS_ORASS, AS_EXPR_STAT,
	AS_EXPR_NULL, AS_COMMA,
};

typedef struct ast_node {
	struct ast_node *parent;
	enum asttype type;
	size_t son_array_size;
	size_t son_count;
	union {
		long long int lli;
		char c;
		void *ptr;
	};
	struct ast_node **son_array;
} ast_t;

/* Return: the pointer to new_node */
ast_t *new_node(enum asttype type, ast_t *parent);

/* Return: the pointer to parent */
ast_t *add_son(ast_t *restrict parent, ast_t *restrict son);

/* Print all nodes */
void print_ast(ast_t *parent);

/* Remove all nodes */
void remove_ast(ast_t *parent);
#endif // AST_H_
