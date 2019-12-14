#include "queue.h"

#include <stdio.h>
#include <ctype.h>

FILE *source;
queue_t gl_queue;

enum token current_token;

unsigned int id_count = 0;
struct identifier {
	unsigned int hash;
	unsigned int type;
	void *memory;
	char name[1024];
} *id_array;

static unsigned int hash(const char *s)
{
	unsigned int seed, hash;
	seed = 131;
	hash = 0;

	while (*s) {
		hash = hash * seed + *s++;
	}
	return hash & 0x7FFFFFFFU;
}

static void next_number(int c)
{
	// DECIMAL INTEGER ONLY
	long long int lli = c - '0';
	while (isdigit(c = fgetc(source))) {
		lli *= 10;
		lli += c - '0';
	}
	fseek(source, -1, SEEK_CUR);
	token_t t;
	t.type = NUMBER;
	t.lli_data = lli;
	gl_queue->push(gl_queue, t);
}

static void next_id(int c) // NOT YET IMPLEMENTED
{
	char name[1024];
	name[0] = c;
	for (int i = 1; isalnum(c = fgetc(source)) || c == '_'; ++i) {
		name[i] = c;
	}
	fseek(source, -1, SEEK_CUR); // from stdio.h

	hash(name);

	return;
}

void next(void)
{
	while (1) {
		int c = fgetc(source);
		if (isalpha(c) || c == '_') {
			next_id(c);
			return;
		}
		if (isdigit(c)) {
			next_number(c);
			return;
		}
		token_t t;
		switch (c) {
		case '+':
			t.type = PLUS;
			gl_queue->push(gl_queue, t);
			return;
		case '-':
			t.type = MINUS;
			gl_queue->push(gl_queue, t);
			return;
		case '*':
			t.type = MUL;
			gl_queue->push(gl_queue, t);
			return;
		case '/':
			t.type = DIVIDE;
			gl_queue->push(gl_queue, t);
			return;
		case '(':
			t.type = L_PARA;
			gl_queue->push(gl_queue, t);
			return;
		case ')':
			t.type = R_PARA;
			gl_queue->push(gl_queue, t);
			return;
		case ';':
			t.type = SEMI;
			gl_queue->push(gl_queue, t);
			return;
		case -1:
			t.type = END;
			gl_queue->push(gl_queue, t);
			return;
		default:
			break;
		}
	}
}
