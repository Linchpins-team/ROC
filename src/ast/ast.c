#include "ast.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
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


ast_t *new_node(enum asttype type, ast_t *parent, size_t dline, size_t dcolumn)
{
	ast_t *ptr = (ast_t *)calloc(1, sizeof(ast_t));
	if (ptr == NULL) {
		panic(strerror(errno));
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
	ptr->dline = dline;
	ptr->dcolumn = dcolumn;
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
			panic(strerror(errno));
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
	free(parent->son_array);
	free(parent);
}

int exist_ast(ast_t *parent, enum asttype type)
{
	if (parent == NULL) {
		return 0;
	}
	if (parent->type == type) {
		return 1;
	}

	for (size_t i = 0; i < parent->son_count; ++i) {
		if (exist_ast(parent->son_array[i], type)) {
			return 1;
		}
	}
	return 0;
}

void iterate_type_ast(ast_t *parent, enum asttype type, void (*func)(struct ast_node *))
{
	if (parent == NULL) {
		return;
	}

	if (parent->type == type) {
		func(parent);
	}

	for (size_t i = 0; i < parent->son_count; ++i) {
		iterate_type_ast(parent->son_array[i], type, func);
	}
}
