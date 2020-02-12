#ifndef AST_H_
#define AST_H_
#include <stddef.h>

enum asttype {
	AS_INT_LIT, AS_FLOAT_LIT, AS_STR_LIT, AS_EXPR, AS_ADD, AS_MINUS, AS_MUL, AS_DIVIDE, AS_POS,
	AS_NEG, AS_PRIM,
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
	struct ast_node *son_array[];
} ast_t;

/* Return: the pointer to new_node */
ast_t *new_node(enum asttype type, ast_t **parent);

void free_node(ast_t *node);

/* Return: the pointer to parent */
ast_t *add_son(ast_t **restrict parent, ast_t *restrict son);

/* Remove all node */
void remove_all(ast_t *parent);

/* Print all node */
void print_all(ast_t *parent);
#endif // AST_H_
