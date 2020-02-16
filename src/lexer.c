#include "lexer.h"
#include "queue.h"
#include "strlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *source;

static size_t line;
static size_t column;

static size_t token_line;
static size_t token_column;

static long int line_start;

queue_t gl_queue;

static strlist_t *strlist;

static char const *const token_str[] = {
	"+",
	"-",
	"*",
	"/",
	"UNUSED =",
	"integer constant",
	"NAME",
	"(",
	")",
	";",
	"end-of-file",
	"identifier",
	"string-literal",
	",",
	"=",
	"*=",
	"/=",
	"%=",
	"+=",
	"-=",
	"<<=",
	">>=",
	"&=",
	"^=",
	"|=",
	"?",
	":",
	"||",
	"&&",
	"|",
	"^",
	"&",
	"==",
	"!=",
	"<",
	">",
	"<=",
	">=",
	"<<",
	">>",
	"%",
	"++",
	"--",
	"~",
	"!",
	"sizeof",
	".",
	"->",
	"[",
	"]",
	"\"",
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"unused_sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",
	"{",
	"}",
	"character constant",
	"long int constant",
	"unsinged int constant",
	"unsinged long int constant",
};

void print_token(enum token t)
{
	fprintf(stdout, "%s", token_str[t]);
}

void print_token_underline(enum token t)
{
	size_t sz = strlen(token_str[t]);
	for (size_t i = 0; i < sz; ++i) {
		fputc('~', stdout);
	}
}

#define GREEN "\x1b[;32;1m"
#define RED "\x1b[;31;1m"
#define RESET "\x1b[0m"

static void panic(const char *msg)
{
	fprintf(stdout, "lexer:%zu:%zu: " RED "error" RESET ": %s\n ",
		token_line, token_column, msg);

	print_line(token_line);
	fprintf(stdout, "\n ");

	shift_line(token_line, token_column);
	fprintf(stdout, RED "^~~~~~~~~~~~~~~\n " GREEN);
	shift_line(token_line, token_column);

	fprintf(stdout, "%s\n" RESET, msg);

	exit(1);
}

#undef GREEN
#undef RED
#undef RESET

static void lexer_seekback(void)
{
	--column;
	fseek(source, -1, SEEK_CUR);
}

static void set_newline(void)
{
	++line;
	column = 1;
	line_start = ftell(source);
}

static int lexer_getc(void)
{
	++column;
	return fgetc(source);
}

static void lexer_push(token_t t)
{
	t.line = token_line;
	t.column = token_column;
	gl_queue->push(gl_queue, t);
}

static void jump_block_comment(void)
{
	int c = lexer_getc();
	while (c != -1) {
		if (c == '*') {
			c = lexer_getc();
			if (c == '/') {
				return;
			} else {
				continue;
			}
		} else if (c == '\n') {
			set_newline();
		}
		c = lexer_getc();
	}
	panic("block comment without end");
	return;
}

static void jump_line_comment(void)
{
	int c = lexer_getc();
	while (c != -1 && c != '\n') {
		c = lexer_getc();
	}

	if (c == '\n') {
		set_newline();
	}

	return;
}

static unsigned long get_dec_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isdigit(c = lexer_getc())) {
		ulli *= 10;
		ulli += c - '0';
	}
	lexer_seekback();
	return ulli;
}

static unsigned long get_oct_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isdigit(c = lexer_getc()) && c != '8' && c != '9') {
		ulli <<= 3;
		ulli |= c - '0';
	}
	lexer_seekback();
	return ulli;
}

static unsigned long get_hex_ul(int c)
{
	unsigned long int ulli = c - '0';
	while (isxdigit(c = lexer_getc())) {
		ulli <<= 4;
		if (isdigit(c)) {
			ulli |= c - '0';
		} else {
			ulli |= toupper(c) - 'A' + 10;
		}
	}
	lexer_seekback();
	return ulli;
}

static void next_number(int c)
{
	unsigned long int uli;
	if (c == 0) {
		if (toupper(c = lexer_getc()) == 'X') {
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
	c = lexer_getc();
	if (c == 'u' || c == 'U') {
		c = lexer_getc();
		if (c == 'l' || c == 'L') {
			t.type = ULONG_CONST;
			t.uli_data = uli;
		} else {
			t.type = UINT_CONST;
			t.ui_data = (unsigned int)uli;
			lexer_seekback();
		}
	} else if (c == 'l' || c == 'L') {
		c = lexer_getc();
		if (c == 'u' || c == 'U') {
			t.type = ULONG_CONST;
			t.uli_data = uli;
		} else {
			t.type = LONG_CONST;
			t.li_data = (long int)uli;
			lexer_seekback();
		}
	} else {
		t.type = INT_CONST;
		t.i_data = (int)uli;
		lexer_seekback();
	}

	lexer_push(t);
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
		} else if (!strcmp("for", buffer)) {
			t->type = K_FOR;
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
	for (i = 1; (isalnum(c = lexer_getc()) || c == '_') && i < sizeof(buffer) - 1; ++i) {
		buffer[i] = c;
	}
	buffer[i] = '\0';
	lexer_seekback(); // from stdio.h

	/* check whether this is identifier */
	get_keyword(&t, buffer);

	lexer_push(t);
}

/* 0 = success; 1 = failed */
static int next_op(int c)
{
	token_t t;
	switch (c) {
	case '+':
		c = lexer_getc();
		switch (c) {
		case '+':
			t.type = INC;
			break;
		case '=':
			t.type = ADDASS;
			break;
		default:
			t.type = PLUS;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '-':
		c = lexer_getc();
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
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '*':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = MULASS;
			break;
		default:
			t.type = MUL;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '/':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = DIVASS;
			break;
		default:
			t.type = DIVIDE;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '(':
		t.type = L_PARA;
		lexer_push(t);
		return 0;
	case ')':
		t.type = R_PARA;
		lexer_push(t);
		return 0;
	case ';':
		t.type = SEMI;
		lexer_push(t);
		return 0;
	case '<':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = LE;
			break;
		case '<':
			c = lexer_getc();
			switch (c) {
			case '=':
				t.type = SHLASS;
				break;
			default:
				t.type = SHL;
				lexer_seekback();
			}
			break;
		default:
			t.type = LT;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '>':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = GE;
			break;
		case '>':
			c = lexer_getc();
			switch (c) {
			case '=':
				t.type = SHRASS;
				break;
			default:
				t.type = SHR;
				lexer_seekback();
			}
			break;
		default:
			t.type = GT;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '=':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = EQ;
			break;
		default:
			t.type = ASS;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '!':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = NEQ;
			break;
		default:
			t.type = NOT;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '%':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = MODASS;
			break;
		default:
			t.type = MOD;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '&':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = ANDASS;
			break;
		case '&':
			t.type = LOG_AND;
			break;
		default:
			t.type = AND;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '^':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = XORASS;
			break;
		default:
			t.type = XOR;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '|':
		c = lexer_getc();
		switch (c) {
		case '=':
			t.type = ORASS;
			break;
		case '|':
			t.type = LOG_OR;
			break;
		default:
			t.type = OR;
			lexer_seekback();
		}
		lexer_push(t);
		return 0;
	case '?':
		t.type = QUESTION;
		lexer_push(t);
		return 0;
	case ':':
		t.type = COLON;
		lexer_push(t);
		return 0;
	case '~':
		t.type = COMPLEMENT;
		lexer_push(t);
		return 0;
	case '[':
		t.type = L_BRACK;
		lexer_push(t);
		return 0;
	case ']':
		t.type = R_BRACK;
		lexer_push(t);
		return 0;
	case '{':
		t.type = L_CURLY;
		lexer_push(t);
		return 0;
	case '}':
		t.type = R_CURLY;
		lexer_push(t);
		return 0;
	case '.':
		t.type = MEMBER;
		lexer_push(t);
		return 0;
	case ',':
		t.type = COMMA;
		lexer_push(t);
		return 0;
	case -1:
		t.type = END;
		lexer_push(t);
		return 0;
	default:
		return 1;
	}
}

static int get_hex(void)
{
	int c = lexer_getc();
	int ret;
	if (isxdigit(c)) {
		if (isdigit(c)) {
			ret = c - '0';
		} else {
			ret = toupper(c) - 'A' + 10;
		}
	}

	if (isxdigit(c)) {
		ret <<= 4;
		if (isdigit(c)) {
			ret = c - '0';
		} else {
			ret = toupper(c) - 'A' + 10;
		}
	}
	return ret;
}

static int get_oct(void)
{
	int c = lexer_getc();
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
	int c = lexer_getc();
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
	int c = lexer_getc();
	t.type = CHAR_CONST;
	switch (c) {
	case '\\':
		t.i_data = get_escape();
		lexer_getc();
		lexer_push(t);
		return;
	case '\'':
		panic("character constant not found");
		return; /* SHOULD NOT BE HERE */
	default:
		t.i_data = c;
		lexer_getc();
		lexer_push(t);
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
	do {
		while ((c = lexer_getc()) != '\"') {
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

		while ((c = lexer_getc()) == '\n' || c == '\t' || c == ' ') {
			continue;
		}
	} while (c == '\"');
	lexer_seekback();

	t.str[it] = 0;
	strlist->push_front(strlist, t.str);
	lexer_push(t);
}

void next(void)
{
	while (1) {
		token_line = line;
		token_column = column;

		int c = lexer_getc();
		if (isalpha(c) || c == '_') {
			next_name(c);
			return;
		}
		if (isdigit(c)) {
			next_number(c);
			return;
		}
		if (c == '#') {
			jump_line_comment();
			continue;
		}
		if (c == '/') {
			c = lexer_getc();
			if (c == '/') {
				jump_line_comment();
				continue;
			} else if (c == '*') {
				jump_block_comment();
				continue;
			} else {
				lexer_seekback();
				lexer_seekback();
				c = lexer_getc();
			}
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
			set_newline();
			break;
		default:
			if (!next_op(c)) {
				return;
			}
			break;
		}
	}
}

void print_line(size_t line)
{
	long int cur = ftell(source);
	fseek(source, 0, SEEK_SET);
	size_t cur_line = 1;

	int c;
	if (line > 1) {
		while ((c = fgetc(source)) != -1) {
			if (c == '\n') {
				++cur_line;
			}
			if (cur_line == line) {
				break;
			}
		}
	}
	while ((c = fgetc(source)) != -1 && c != '\n') {
		fputc(c, stdout);
	}
	fseek(source, cur, SEEK_SET);
}

void shift_line(size_t line, size_t column)
{
	long int cur = ftell(source);
	fseek(source, 0, SEEK_SET);
	size_t cur_line = 1;

	int c;
	if (line > 1) {
		while ((c = fgetc(source)) != -1) {
			if (c == '\n') {
				++cur_line;
			}
			if (cur_line == line) {
				break;
			}
		}
	}

	size_t cur_column = 1;
	if (column > 1) {
		while ((c = fgetc(source)) != -1 && c != '\n') {
			if (c == '\t') {
				fputc(c, stdout);
			} else {
				fputc(' ', stdout);
			}
			++cur_column;
			if (cur_column == column) {
				break;
			}
		}
	}
	fseek(source, cur, SEEK_SET);
}

void exit_lexer(void)
{
	clear_queue(gl_queue);
	strlist->delete(strlist);
}

void init_lexer(void)
{
	line = 1;
	column = 1;
	init_queue(&gl_queue);
	strlist = init_strlist();
	line_start = ftell(source);
}
