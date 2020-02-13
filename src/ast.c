#include "ast.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

ast_t *new_node(enum asttype type, ast_t *parent)
{
	ast_t *ptr = (ast_t *)calloc(1, sizeof(ast_t));
	if (ptr == NULL) {
		return NULL;
	}

	/* Setup Parent Node */
	if (parent != NULL) {
		add_son(parent, ptr);
		ptr->parent = parent;
	} else {
		ptr->parent = NULL;
	}

	/* Setup Son Node */
	ptr->type = type;
	ptr->son_array_size = 0;
	ptr->son_array = NULL;
	ptr->son_count = 0;
	return ptr;
}

ast_t *add_son(ast_t *restrict parent, ast_t *restrict son)
{
	assert(parent != NULL);
	assert(son != NULL);

	/* Check if the son_array_size is enough */
	if (parent->son_array_size <= parent->son_count) {
		/* son_array_size should not be less than son_count, though. */

		/* Extend the size of son_array */
		ast_t **new_ptr = (ast_t **)realloc(parent->son_array, sizeof(ast_t *) * (parent->son_array_size += 2));
		if (new_ptr == NULL) {
			fprintf(stderr, "REALLOC FAILED: At Line %d , Function: %s , File %s\n", __LINE__, __func__, __FILE__);
			exit(EXIT_FAILURE);
		}
		parent->son_array = new_ptr;
	}

	/* Add the son node to parent */
	parent->son_array[parent->son_count++] = son;

	/* Add the parent to son */
	son->parent = parent;

	return parent;
}

static void __print_all(ast_t *parent, size_t s)
{
	for (size_t i = 0; i < s; ++i) {
		fputs("  ", stdout);
	}
	fprintf(stdout, "%3d\n", parent->type);
	for (size_t i = 0; i < parent->son_count; ++i) {
		__print_all(parent->son_array[i], s+1);
	}
}

void print_ast(ast_t *parent)
{
	__print_all(parent, 0);
}

void remove_ast(ast_t *parent)
{
	for (size_t i = 0; i < parent->son_count; ++i) {
		remove_ast(parent->son_array[i]);
	}
	free(parent);
}
