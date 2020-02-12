#include "ast.h"
#include "aalloc.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

ast_t *new_node(enum asttype type, ast_t *parent)
{
	ast_t *ptr = (ast_t *)acalloc(1, sizeof(ast_t));
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


static const char *getname(ast_t const *ast)
{
	static char buffer[256];
	enum asttype t = ast->type;
	switch (t) {
	case AS_INT_LIT:
		sprintf(buffer, "INT: %lld", ast->lli);
		return buffer;
	case AS_FLOAT_LIT:
		return "FLOAT";
	case AS_STR_LIT:
		return "STRING";
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
	case AS_INDEX:
		return "INDEX";
	case AS_FUNC_CALL:
		return "FUNC_CALL";
	case AS_MEMBER:
		return "MEMBER";
	case AS_ID:
		return "ID";
	case AS_PTR_MEMBER:
		return "PTR_MEMBER";
	case AS_POST_INC:
		return "POST_INC";
	case AS_POST_DEC:
		return "POST_DEC";
	case AS_ARGU_LIST:
		return "ARGU_LIST";
	case AS_PRE_INC:
		return "PRE_INC";
	case AS_PRE_DEC:
		return "PRE_DEC";
	case AS_ADDRESS_OF:
		return "ADDRESS_OF";
	case AS_VALUE_OF:
		return "VALUE_OF";
	case AS_COMPLEMENT:
		return "COMPLEMENT";
	case AS_NOT:
		return "NOT";
	case AS_SIZEOF:
		return "SIZEOF";
	case AS_MOD:
		return "MOD";
	case AS_SHL:
		return "SHL";
	case AS_SHR:
		return "SHR";
	case AS_LT:
		return "LT";
	case AS_GT:
		return "GT";
	case AS_LE:
		return "LE";
	case AS_GE:
		return "GE";
	case AS_EQ:
		return "EQ";
	case AS_NEQ:
		return "NEQ";
	case AS_AND:
		return "AND";
	case AS_XOR:
		return "XOR";
	case AS_OR:
		return "OR";
	case AS_LOG_OR:
		return "LOG_OR";
	case AS_LOG_AND:
		return "LOG_AND";
	case AS_CONDI:
		return "CONDI";
	case AS_UNARY:
		return "UNARY";
	case AS_ASS:
		return "ASS";
	case AS_MULASS:
		return "MULASS";
	case AS_DIVASS:
		return "DIVASS";
	case AS_MODASS:
		return "MODASS";
	case AS_ADDASS:
		return "ADDASS";
	case AS_SUBASS:
		return "SUBASS";
	case AS_SHLASS:
		return "SHLASS";
	case AS_SHRASS:
		return "SHRASS";
	case AS_ANDASS:
		return "ANDASS";
	case AS_XORASS:
		return "XORASS";
	case AS_ORASS:
		return "ORASS";
	case AS_EXPR_STAT:
		return "EXPR_STAT";
	case AS_EXPR_NULL:
		return "EXPR_NULL";
	case AS_COMMA:
		return "COMMA";
	default:
		return "UNKNOWN";
	}
}

static void __print_all(ast_t *parent, size_t s)
{
	for (size_t i = 0; i < s; ++i) {
		fputs("  ", stdout);
	}
	fprintf(stdout, "%s\n", getname(parent));
	for (size_t i = 0; i < parent->son_count; ++i) {
		__print_all(parent->son_array[i], s+1);
	}
}

void print_all(ast_t *parent)
{
	__print_all(parent, 0);
}
