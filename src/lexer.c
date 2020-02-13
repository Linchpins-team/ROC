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

static void get_keyword(token_t *const t)
{
	switch (t->str[0]) {
	case 'a':
		if (!strcmp("auto", t->str)) {
			t->type = K_AUTO;
			return;
		}
		t->type = ID;
		return;
	case 'b':
		if (!strcmp("break", t->str)) {
			t->type = K_BREAK;
			return;
		}
		t->type = ID;
		return;
	case 'c':
		switch (t->str[1]) {
		case 'a':
			if (!strcmp("case", t->str)) {
				t->type = K_CASE;
				return;
			}
			t->type = ID;
			return;
		case 'h':
			if (!strcmp("char", t->str)) {
				t->type = K_CHAR;
				return;
			}
			t->type = ID;
			return;
		case 'o':
			if (!strcmp("continue", t->str)) {
				t->type = K_CONTINUE;
				return;
			} else if (!strcmp("const", t->str)) {
				t->type = K_CONST;
				return;
			}
			t->type = ID;
			return;
		default:
			t->type = ID;
			return;
		}
	case 'd':
		if (!strcmp("double", t->str)) {
			t->type = K_DOUBLE;
			return;
		} else if (!strcmp("do", t->str)) {
			t->type = K_DO;
			return;
		} else if (!strcmp("default", t->str)) {
			t->type = K_DEFAULT;
			return;
		}
		t->type = ID;
		return;
	case 'e':
		if (!strcmp("else", t->str)) {
			t->type = K_ELSE;
			return;
		} else if (!strcmp("enum", t->str)) {
			t->type = K_ENUM;
			return;
		} else if (!strcmp("extern", t->str)) {
			t->type = K_EXTERN;
			return;
		}
		t->type = ID;
		return;
	case 'f':
		if (!strcmp("float", t->str)) {
			t->type = K_FLOAT;
			return;
		}
		t->type = ID;
		return;
	case 'g':
		if (!strcmp("goto", t->str)) {
			t->type = K_GOTO;
			return;
		}
		t->type = ID;
		return;
	case 'i':
		if (!strcmp("if", t->str)) {
			t->type = K_IF;
			return;
		} else if (!strcmp("int", t->str)) {
			t->type = K_INT;
			return;
		}
		t->type = ID;
		return;
	case 'l':
		if (!strcmp("long", t->str)) {
			t->type = K_LONG;
			return;
		}
		t->type = ID;
		return;
	case 'r':
		if (!strcmp("register", t->str)) {
			t->type = K_REGISTER;
			return;
		} else if (!strcmp("return", t->str)) {
			t->type = K_RETURN;
			return;
		}
		t->type = ID;
		return;
	case 's':
		switch (t->str[1]) {
		case 'h':
			if (!strcmp("short", t->str)) {
				t->type = K_SHORT;
				return;
			}
			t->type = ID;
			return;
		case 'i':
			if (!strcmp("signed", t->str)) {
				t->type = K_SIGNED;
				return;
			} else if (!strcmp("sizeof", t->str)) {
				t->type = SIZEOF;
				return;
			}
			t->type = ID;
			return;
		case 't':
			if (!strcmp("static", t->str)) {
				t->type = K_STATIC;
				return;
			} else if (!strcmp("struct", t->str)) {
				t->type = K_STRUCT;
				return;
			}
			t->type = ID;
			return;
		case 'w':
			if (!strcmp("switch", t->str)) {
				t->type = K_SWITCH;
				return;
			}
			t->type = ID;
			return;
		default:
			t->type = ID;
			return;
		}
	case 't':
		if (!strcmp("typedef", t->str)) {
			t->type = K_TYPEDEF;
			return;
		}
		t->type = ID;
		return;
	case 'u':
		if (!strcmp("union", t->str)) {
			t->type = K_UNION;
			return;
		} else if (!strcmp("unsigned", t->str)) {
			t->type = K_UNSIGNED;
			return;
		}
		t->type = ID;
		return;
	case 'v':
		if (!strcmp("void", t->str)) {
			t->type = K_VOID;
			return;
		} else if (!strcmp("volatile", t->str)) {
			t->type = K_VOLATILE;
			return;
		}
		t->type = ID;
		return;
	case 'w':
		if (!strcmp("while", t->str)) {
			t->type = K_WHILE;
			return;
		}
		t->type = ID;
		return;
	default:
		t->type = ID;
		return;
	}
}

static void next_name(int c)
{
	token_t t;
	t.str[0] = c;
	size_t i;
	for (i = 1; (isalnum(c = fgetc(source)) || c == '_') && i < sizeof(t.str) - 1; ++i) {
		t.str[i] = c;
	}
	t.str[i] = '\0';
	fseek(source, -1, SEEK_CUR); // from stdio.h

	/* check whether this is identifier */
	get_keyword(&t);

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
	case '{':
		t.type = L_CURLY;
		gl_queue->push(gl_queue, t);
		return 0;
	case '}':
		t.type = R_CURLY;
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
	case '\"':
		t.type = DQUOTE;
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
