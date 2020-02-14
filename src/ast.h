#ifndef AST_H_
#define AST_H_
#include <stddef.h>

enum asttype {
/* 0 */		AS_INT_LIT = 0,
		AS_FLOAT_LIT,
		AS_STR_LIT,
		AS_ADD,
		AS_MINUS,
/* 5 */		AS_MUL,
		AS_DIVIDE,
		AS_POS,
		AS_NEG,
		AS_PRIM,
/* 10 */	AS_INDEX,
		AS_FUNC_CALL,
		AS_MEMBER,
		AS_ID,
		AS_PTR_MEMBER,
/* 15 */	AS_POST_INC,
		AS_POST_DEC,
		AS_ARGU_LIST,
		AS_PRE_INC,
		AS_PRE_DEC,
/* 20 */	AS_ADDRESS_OF,
		AS_VALUE_OF,
		AS_COMPLEMENT,
		AS_NOT,
		AS_SIZEOF,
/* 25 */	AS_MOD,
		AS_SHL,
		AS_SHR,
		AS_LT,
		AS_GT,
/* 30 */	AS_LE,
		AS_GE,
		AS_EQ,
		AS_NEQ,
		AS_AND,
/* 35 */	AS_XOR,
		AS_OR,
		AS_LOG_OR,
		AS_LOG_AND,
		AS_CONDI,
/* 40 */	AS_UNARY,
		AS_ASS,
		AS_MULASS,
		AS_DIVASS,
		AS_MODASS,
/* 45 */	AS_ADDASS,
		AS_SUBASS,
		AS_SHLASS,
		AS_SHRASS,
		AS_ANDASS,
/* 50 */	AS_XORASS,
		AS_ORASS,
		AS_EXPR_STAT,
		AS_EXPR_NULL,
		AS_COMMA,
/* 55 */	AS_DECLARATION,
		AS_DECL_TYPEDEF,
		AS_DECL_EXTERN,
		AS_DECL_STATIC,
		AS_DECL_AUTO,
/* 60 */	AS_DECL_REGISTER,
		AS_DECL_VOID,
		AS_DECL_CHAR,
		AS_DECL_SHORT,
		AS_DECL_INT,
/* 65 */	AS_DECL_LONG,
		AS_DECL_FLOAT,
		AS_DECL_DOUBLE,
		AS_DECL_SIGNED,
		AS_DECL_UNSIGNED,
/* 70 */	AS_DECL_STRUCT,
		AS_DECL_UNION,
		AS_DECL_ENUM,
		AS_DECL_TYPENAME,
		AS_DECL_CONST,
/* 75 */	AS_DECL_VOLATILE,
		AS_DECL,
		AS_DIRECT_DECLARATOR,
		AS_DECL_ARRAY,
		AS_DECL_FUNC,
/* 80 */	AS_CONSTEXPR,
		AS_INIT_DECL_LIST,
		AS_INIT_DECL,
		AS_DECL_PTR,
		AS_DECL_CONST_PTR,
/* 85 */	AS_DECL_VOLATILE_PTR,
		AS_DECL_PARA_TYPE_LIST_VLEN,
		AS_DECL_PARA_TYPE_LIST_NORM,
		AS_PARA_LIST,
		AS_PARA_DECL,
/* 90 */	AS_ID_LIST,
		AS_TYPENAME,
		AS_ABSTRACT_DECL,
		AS_DIRECT_ABSTRACT_DECL,
		AS_DIRECT_ABSTRACT_DECL_ARRAY,
/* 95 */	AS_DIRECT_ABSTRACT_DECL_FUNC,
		AS_TYPEDEF_NAME,
		AS_INITIALIZER,
		AS_INITIALIZER_LIST,
		AS_EXPR,
/* 100 */	AS_STATE,
		AS_JUMP_GOTO,
		AS_JUMP_CONTINUE,
		AS_JUMP_BREAK,
		AS_JUMP_RETURN,
/* 105 */	AS_WHILE,
		AS_DO_WHILE,
		AS_FOR,
		AS_IF,
		AS_SWITCH,
/* 110 */	AS_COMPOUND,
		AS_LABEL_CASE,
		AS_LABEL_DEFAULT,
		AS_LABEL_ID,
		AS_DECLARATION_LIST,
/* 115 */	AS_STATEMENT_LIST,
		AS_FUNC_DEFINITION,
		AS_EXTERN_DECL,
		AS_TRANSLATION_UNIT,
};

typedef struct ast_node {
	struct ast_node *parent;
	enum asttype type;
	size_t son_array_size;
	size_t son_count;
	union {
		int i;
		unsigned int ui;
		long int li;
		unsigned long int uli;
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
