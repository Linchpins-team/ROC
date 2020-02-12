#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

enum token {
	PLUS=0, MINUS, MUL, DIVIDE, ASSIGN, NUMBER,
	NAME, L_PARA, R_PARA, SEMI, END, ID, STRING,
	COMMA, ASS, MULASS, DIVASS, MODASS, ADDASS, SUBASS,
	SHLASS, SHRASS, ANDASS, XORASS, ORASS,
	QUESTION, COLON, LOG_OR, LOG_AND, OR, XOR, AND,
	EQ, NEQ, LT, GT, LE, GE, SHL, SHR,
	MOD, INC, DEC, ADRESS, VALUE, COMPLEMENT, NOT, SIZEOF,
	MEMBER, PTR_MEMBER, L_BRACK, R_BRACK,
};

typedef struct {
	enum token type;
	union {
		long long int lli_data;
		char str[1024];
	};
} token_t;

typedef struct queue {
	struct queue_node *first;
	struct queue_node *last;
	void (*push)(struct queue *q, token_t value);
	void (*pop)(struct queue *q);
	token_t (*front)(struct queue *q);
	token_t (*back)(struct queue *q);
} *queue_t;

struct queue_node {
	struct queue_node *prev;
	token_t value;
};

extern size_t line, column; // lexer.c

void init_queue(struct queue **q);
void clear_queue(queue_t q);
#endif /* QUEUE_H_ */
