#include "queue.h"
#include "strlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *source;

size_t line, column;
queue_t gl_queue;

static strlist_t *strlist;

enum token current_token;

unsigned int id_count = 0;
struct identifier {
	unsigned int hash;
	unsigned int type;
	void *memory;
	char name[1024];
} *id_array;

static void panic(const char *msg)
{
	fprintf(stderr, "ERROR: LEXER: %s\n", msg);
	exit(1);
}

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

static unsigned long get_dec_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isdigit(c = fgetc(source))) {
		ulli *= 10;
		ulli += c - '0';
	}
	fseek(source, -1, SEEK_CUR);
	return ulli;
}

static unsigned long get_oct_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isdigit(c = fgetc(source)) && c != '8' && c != '9') {
		ulli <<= 3;
		ulli |= c - '0';
	}
	fseek(source, -1, SEEK_CUR);
	return ulli;
}

static unsigned long get_hex_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isxdigit(c = fgetc(source))) {
		ulli <<= 4;
		if (isdigit(c)) {
			ulli |= c - '0';
		} else {
			ulli |= c - 'A' + 10;
		}
	}
	fseek(source, -1, SEEK_CUR);
	return ulli;
}

static void next_number(int c)
{
	unsigned long int uli;
	if (c == 0) {
		if (toupper(c = fgetc(source)) == 'X') {
			/* HEX */
			uli = get_hex_ul(c);
		} else {
			/* OCT */
			uli = get_oct_ul(c);
		}
	} else {
		uli = get_dec_ul(c);
	}

	token_t t;
	c = fgetc(source);
	if (c == 'u' || c == 'U') {
		c = fgetc(source);
		if (c == 'l' || c == 'L') {
			t.type = ULONG_CONST;
			t.uli_data = uli;
		} else {
			t.type = UINT_CONST;
			t.ui_data = (unsigned int)uli;
			fseek(source, -1, SEEK_CUR);
		}
	} else if (c == 'l' || c == 'L') {
		c = fgetc(source);
		if (c == 'u' || c == 'U') {
			t.type = ULONG_CONST;
			t.uli_data = uli;
		} else {
			t.type = LONG_CONST;
			t.li_data = (long int)uli;
			fseek(source, -1, SEEK_CUR);
		}
	} else {
		t.type = INT_CONST;
		t.i_data = (int)uli;
		fseek(source, -1, SEEK_CUR);
	}

	gl_queue->push(gl_queue, t);
}

static void get_keyword(token_t *const t, char const *restrict const buffer)
{
	switch (buffer[0]) {
	case 'a':
		if (!strcmp("auto", buffer)) {
			t->type = K_AUTO;
			return;
		}
		t->type = ID;
		return;
	case 'b':
		if (!strcmp("break", buffer)) {
			t->type = K_BREAK;
			return;
		}
		t->type = ID;
		return;
	case 'c':
		switch (buffer[1]) {
		case 'a':
			if (!strcmp("case", buffer)) {
				t->type = K_CASE;
				return;
			}
			t->type = ID;
			return;
		case 'h':
			if (!strcmp("char", buffer)) {
				t->type = K_CHAR;
				return;
			}
			t->type = ID;
			return;
		case 'o':
			if (!strcmp("continue", buffer)) {
				t->type = K_CONTINUE;
				return;
			} else if (!strcmp("const", buffer)) {
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
		if (!strcmp("double", buffer)) {
			t->type = K_DOUBLE;
			return;
		} else if (!strcmp("do", buffer)) {
			t->type = K_DO;
			return;
		} else if (!strcmp("default", buffer)) {
			t->type = K_DEFAULT;
			return;
		}
		t->type = ID;
		return;
	case 'e':
		if (!strcmp("else", buffer)) {
			t->type = K_ELSE;
			return;
		} else if (!strcmp("enum", buffer)) {
			t->type = K_ENUM;
			return;
		} else if (!strcmp("extern", buffer)) {
			t->type = K_EXTERN;
			return;
		}
		t->type = ID;
		return;
	case 'f':
		if (!strcmp("float", buffer)) {
			t->type = K_FLOAT;
			return;
		}
		t->type = ID;
		return;
	case 'g':
		if (!strcmp("goto", buffer)) {
			t->type = K_GOTO;
			return;
		}
		t->type = ID;
		return;
	case 'i':
		if (!strcmp("if", buffer)) {
			t->type = K_IF;
			return;
		} else if (!strcmp("int", buffer)) {
			t->type = K_INT;
			return;
		}
		t->type = ID;
		return;
	case 'l':
		if (!strcmp("long", buffer)) {
			t->type = K_LONG;
			return;
		}
		t->type = ID;
		return;
	case 'r':
		if (!strcmp("register", buffer)) {
			t->type = K_REGISTER;
			return;
		} else if (!strcmp("return", buffer)) {
			t->type = K_RETURN;
			return;
		}
		t->type = ID;
		return;
	case 's':
		switch (buffer[1]) {
		case 'h':
			if (!strcmp("short", buffer)) {
				t->type = K_SHORT;
				return;
			}
			t->type = ID;
			return;
		case 'i':
			if (!strcmp("signed", buffer)) {
				t->type = K_SIGNED;
				return;
			} else if (!strcmp("sizeof", buffer)) {
				t->type = SIZEOF;
				return;
			}
			t->type = ID;
			return;
		case 't':
			if (!strcmp("static", buffer)) {
				t->type = K_STATIC;
				return;
			} else if (!strcmp("struct", buffer)) {
				t->type = K_STRUCT;
				return;
			}
			t->type = ID;
			return;
		case 'w':
			if (!strcmp("switch", buffer)) {
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
		if (!strcmp("typedef", buffer)) {
			t->type = K_TYPEDEF;
			return;
		}
		t->type = ID;
		return;
	case 'u':
		if (!strcmp("union", buffer)) {
			t->type = K_UNION;
			return;
		} else if (!strcmp("unsigned", buffer)) {
			t->type = K_UNSIGNED;
			return;
		}
		t->type = ID;
		return;
	case 'v':
		if (!strcmp("void", buffer)) {
			t->type = K_VOID;
			return;
		} else if (!strcmp("volatile", buffer)) {
			t->type = K_VOLATILE;
			return;
		}
		t->type = ID;
		return;
	case 'w':
		if (!strcmp("while", buffer)) {
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
	char buffer[1024];
	buffer[0] = c;
	size_t i;
	for (i = 1; (isalnum(c = fgetc(source)) || c == '_') && i < sizeof(buffer) - 1; ++i) {
		buffer[i] = c;
	}
	buffer[i] = '\0';
	fseek(source, -1, SEEK_CUR); // from stdio.h

	/* check whether this is identifier */
	get_keyword(&t, buffer);

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
	case -1:
		t.type = END;
		gl_queue->push(gl_queue, t);
		return 0;
	default:
		return 1;
	}
}

static int get_hex(void)
{
	int c = fgetc(source);
	int ret;
	if (isxdigit(c)) {
		if (isdigit(c)) {
			ret = c - '0';
		} else {
			ret = c - 'A' + 10;
		}
	}

	if (isxdigit(c)) {
		ret <<= 4;
		if (isdigit(c)) {
			ret = c - '0';
		} else {
			ret = c - 'A' + 10;
		}
	}
	return ret;
}

static int get_oct(void)
{
	int c = fgetc(source);
	int ret;
	if (isdigit(c) && c != '8' && c != '9') {
		ret = c - '0';
	}

	if (isdigit(c) && c != '8' && c != '9') {
		ret <<= 3;
		ret |= c - '0';
	} else {
		return ret;
	}

	if (isdigit(c) && c != '8' && c != '9') {
		ret <<= 3;
		ret |= c - '0';
	}
	return ret;
}

static int get_escape(void)
{
	int c = fgetc(source);
	if (isdigit(c)) {
		return get_oct();
	}
	switch (c) {
	case '\\':
		return '\\';
	case '\"':
		return '\"';
	case '\'':
		return '\'';
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '?':
		return '?';
	case 'x':
		return get_hex();
	default:
		panic("unknown escape sequence");
		return -1;
	}
}

static void next_char(void)
{
	token_t t;
	int c = fgetc(source);
	t.type = CHAR_CONST;
	switch (c) {
	case '\\':
		t.i_data = get_escape();
		fgetc(source);
		gl_queue->push(gl_queue, t);
		return;
	case '\'':
		panic("THE CHARACTER CONSTANT WITHOUT A CHARACTER");
		return; /* SHOULD NOT BE HERE */
	default:
		t.i_data = c;
		fgetc(source);
		gl_queue->push(gl_queue, t);
		return;
	}
}

static void next_string(void)
{
	token_t t;
	t.type = STRING;

	size_t it = 0;
	size_t sz = 32;
	t.str = malloc(sizeof(char) * sz);
	if (t.str == NULL) {
		panic("cannot allocate memory");
		return;
	}

	int c;
	while ((c = fgetc(source)) != '\"') {
		if (c == '\\') {
			t.str[it++] = get_escape();
		} else {
			t.str[it++] = c;
		}
		if (it >= sz) {
			t.str = realloc(t.str, sizeof(char) * (sz *= 2));
			if (t.str == NULL) {
				panic("cannot allocate memory");
				return;
			}
		}
		if (c == -1) {
			panic("string without end");
		}
	}
	t.str[it] = 0;
	strlist->push_front(strlist, t.str);
	gl_queue->push(gl_queue, t);
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
		if (c == '\'') {
			next_char();
			return;
		}
		if (c == '\"') {
			next_string();
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

void exit_lexer(void)
{
	clear_queue(gl_queue);
	strlist->delete(strlist);
}

void init_lexer(void)
{
	init_queue(&gl_queue);
	strlist = init_strlist();
}
