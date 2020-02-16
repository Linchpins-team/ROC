#ifndef LEXER_H_
#define LEXER_H_
#include "queue.h"
#include <stdio.h>
void next(void);
void init_lexer(void);
void exit_lexer(void);
void print_line(size_t line);
void shift_line(size_t line, size_t column);
void print_token(enum token t);
void print_token_underline(enum token t);
extern queue_t gl_queue;
extern FILE *source;
#endif // LEXER_H_
