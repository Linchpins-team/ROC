#include "ast.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

ast_t *new_node(enum asttype type, ast_t **parent)
{
	ast_t *ptr = (ast_t *)calloc(1, sizeof(ast_t));
	if (ptr == NULL) {
		return NULL;
	}

	/* Setup Parent Node */
	if (parent != NULL && *parent != NULL) {
		add_son(parent, ptr);
		ptr->parent = *parent;
	} else {
		ptr->parent = NULL;
	}

	/* Setup Son Node */
	ptr->type = type;
	ptr->son_array_size = 0;
	ptr->son_count = 0;
	return ptr;
}

void free_node(ast_t *node)
{
	/* FIXME: free the void pointer if needed */
	free(node);
}

ast_t *add_son(ast_t **restrict parent, ast_t *restrict son)
{
	assert(parent != NULL);
	assert(son != NULL);
	assert(*parent != NULL);

	/* Check if the son_array_size is enough */
	if ((*parent)->son_array_size <= (*parent)->son_count) {
		/* son_array_size should not be less than son_count, though. */

		/* Extend the size of son_array */
		ast_t *new_ptr = (ast_t *)realloc(*parent, sizeof(ast_t) + sizeof(ast_t *) * ((*parent)->son_count + 2));
		if (new_ptr == NULL) {
			fprintf(stderr, "REALLOC FAILED: At Line %d , Function: %s , File %s\n", __LINE__, __func__, __FILE__);
			exit(EXIT_FAILURE);
		}
		*parent = new_ptr;
		(*parent)->son_array_size += 2;
	}

	/* Add the son node to parent */
	(*parent)->son_array[(*parent)->son_count++] = son;

	/* Add the parent to son */
	son->parent = *parent;

	return *parent;
}

void remove_all(ast_t *parent)
{
	for (size_t i = 0; i < parent->son_count; ++i) {
		remove_all(parent->son_array[i]);
	}
	free(parent);
}

static const char *getname(enum asttype t)
{
	switch (t) {
	case AS_INT_LIT:
		return "INT";
	case AS_EXPR:
		return "EXPR";
	case AS_ADD:
		return "ADD";
	case AS_MINUS:
		return "MINUS";
	case AS_MUL:
		return "MUL";
	case AS_DIVIDE:
		return "DIVIDE";
	case AS_POS:
		return "POS";
	case AS_NEG:
		return "NEG";
	case AS_PRIM:
		return "PRIMARY";
	}
}

static void __print_all(ast_t *parent, size_t s)
{
	for (size_t i = 0; i < s; ++i) {
		fputs("    ", stdout);
	}
	fprintf(stdout, "%s\n", getname(parent->type));
	for (size_t i = 0; i < parent->son_count; ++i) {
		__print_all(parent->son_array[i], s+1);
	}
}

void print_all(ast_t *parent)
{
	__print_all(parent, 0);
}
