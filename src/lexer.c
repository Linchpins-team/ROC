#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

FILE *source;

size_t line, column;
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

static void next_name(int c)
{
	token_t t;
	t.str[0] = c;
	int i;
	for (int i = 1; (isalnum(c = fgetc(source)) || c == '_') && i < sizeof(t.str) - 1; ++i) {
		t.str[i] = c;
	}
	t.str[i] = '\0';
	fseek(source, -1, SEEK_CUR); // from stdio.h

	/* check whether this is identifier */
	if (!strcmp("sizeof", t.str)) {
		t.type = SIZEOF;
	} else {
		t.type = ID;
	}

	gl_queue->push(gl_queue, t);
}

/* 0 = success; 1 = failed */
static int next_op(int c)
{
	token_t t;
	switch (c) {
	case '+':
		c = fgetc(source);
		switch (c) {
		case '+':
			t.type = INC;
			break;
		case '=':
			t.type = ADDASS;
			break;
		default:
			t.type = PLUS;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '-':
		c = fgetc(source);
		switch (c) {
		case '-':
			t.type = DEC;
			break;
		case '=':
			t.type = SUBASS;
			break;
		case '>':
			t.type = PTR_MEMBER;
			break;
		default:
			t.type = MINUS;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '*':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = MULASS;
			break;
		default:
			t.type = MUL;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '/':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = DIVASS;
			break;
		default:
			t.type = DIVIDE;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '(':
		t.type = L_PARA;
		gl_queue->push(gl_queue, t);
		return 0;
	case ')':
		t.type = R_PARA;
		gl_queue->push(gl_queue, t);
		return 0;
	case ';':
		t.type = SEMI;
		gl_queue->push(gl_queue, t);
		return 0;
	case '<':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = LE;
			break;
		case '<':
			c = fgetc(source);
			switch (c) {
			case '=':
				t.type = SHLASS;
				break;
			default:
				t.type = SHL;
				fseek(source, -1, SEEK_CUR);
			}
			break;
		default:
			t.type = LT;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '>':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = GE;
			break;
		case '>':
			c = fgetc(source);
			switch (c) {
			case '=':
				t.type = SHRASS;
				break;
			default:
				t.type = SHR;
				fseek(source, -1, SEEK_CUR);
			}
			break;
		default:
			t.type = GT;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '=':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = EQ;
			break;
		default:
			t.type = ASS;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '!':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = NEQ;
			break;
		default:
			t.type = NOT;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '%':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = MODASS;
			break;
		default:
			t.type = MOD;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '&':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = ANDASS;
			break;
		case '&':
			t.type = LOG_AND;
			break;
		default:
			t.type = AND;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '^':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = XORASS;
			break;
		default:
			t.type = XOR;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '|':
		c = fgetc(source);
		switch (c) {
		case '=':
			t.type = ORASS;
			break;
		case '|':
			t.type = LOG_OR;
			break;
		default:
			t.type = OR;
			fseek(source, -1, SEEK_CUR);
		}
		gl_queue->push(gl_queue, t);
		return 0;
	case '?':
		t.type = QUESTION;
		gl_queue->push(gl_queue, t);
		return 0;
	case ':':
		t.type = COLON;
		gl_queue->push(gl_queue, t);
		return 0;
	case '~':
		t.type = COMPLEMENT;
		gl_queue->push(gl_queue, t);
		return 0;
	case '[':
		t.type = L_BRACK;
		gl_queue->push(gl_queue, t);
		return 0;
	case ']':
		t.type = R_BRACK;
		gl_queue->push(gl_queue, t);
		return 0;
	case '.':
		t.type = MEMBER;
		gl_queue->push(gl_queue, t);
		return 0;
	case ',':
		t.type = COMMA;
		gl_queue->push(gl_queue, t);
		return 0;
	case -1:
		t.type = END;
		gl_queue->push(gl_queue, t);
		return 0;
	default:
		return 1;
	}
}

void next(void)
{
	while (1) {
		int c = fgetc(source);
		if (isalpha(c) || c == '_') {
			next_name(c);
			return;
		}
		if (isdigit(c)) {
			next_number(c);
			return;
		}
		switch (c) {
		case '\n':
			++line;
			break;
		default:
			if (!next_op(c)) {
				return;
			}
			break;
		}
	}
}
