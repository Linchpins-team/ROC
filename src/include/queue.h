#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

enum token {
/* 0 */		PLUS=0,
		MINUS,
		MUL,
		DIVIDE,
		UNUSED_ASSIGN,
/* 5 */		INT_CONST,
		NAME,
		L_PARA,
		R_PARA,
		SEMI,
/* 10 */	END,
		ID,
		STRING,
		COMMA,
		ASS,
/* 15 */	MULASS,
		DIVASS,
		MODASS,
		ADDASS,
		SUBASS,
/* 20 */	SHLASS,
		SHRASS,
		ANDASS,
		XORASS,
		ORASS,
/* 25 */	QUESTION,
		COLON,
		LOG_OR,
		LOG_AND,
		OR,
/* 30 */	XOR,
		AND,
		EQ,
		NEQ,
		LT,
/* 35 */	GT,
		LE,
		GE,
		SHL,
		SHR,
/* 40 */	MOD,
		INC,
		DEC,
		COMPLEMENT,
		NOT,
/* 45 */	SIZEOF,
		MEMBER,
		PTR_MEMBER,
		L_BRACK,
		R_BRACK,
/* 50 */	DQUOTE,
		K_AUTO,
		K_BREAK,
		K_CASE,
		K_CHAR,
/* 55 */	K_CONST,
		K_CONTINUE,
		K_DEFAULT,
		K_DO,
		K_DOUBLE,
/* 60 */	K_ELSE,
		K_ENUM,
		K_EXTERN,
		K_FLOAT,
		K_FOR,
/* 65 */	K_GOTO,
		K_IF,
		K_INT,
		K_LONG,
		K_REGISTER,
/* 70 */	K_RETURN,
		K_SHORT,
		K_SIGNED,
		K_UNUSED_SIZEOF, /* K_SIZEOF WAS DEFINED AS SIZEOF */
		K_STATIC,
/* 75 */	K_STRUCT,
		K_SWITCH,
		K_TYPEDEF,
		K_UNION,
		K_UNSIGNED,
/* 80 */	K_VOID,
		K_VOLATILE,
		K_WHILE,
		L_CURLY,
		R_CURLY,
/* 85 */	CHAR_CONST,
		LONG_CONST,
		UINT_CONST,
		ULONG_CONST,
};

typedef struct {
	enum token type;
	size_t line;
	size_t column;
	union {
		int i_data;
		unsigned int ui_data;
		long int li_data;
		unsigned long int uli_data;
		char *str;
	};
} token_t;

typedef struct queue {
	struct queue_node *first;
	struct queue_node *last;
	void (*push)(struct queue *q, token_t value);
	void (*pop)(struct queue *q);
	token_t (*front)(struct queue *q);
	token_t (*back)(struct queue *q);
	token_t (*back_count)(struct queue *q, unsigned long count);
	int (*empty)(struct queue *q);
} *queue_t;

struct queue_node {
	struct queue_node *prev;
	token_t value;
};

void init_queue(struct queue **q);
void clear_queue(queue_t q);
#endif /* QUEUE_H_ */
