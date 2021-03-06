#ifndef LEXER_H_
#define LEXER_H_
#include "queue.h"
#include <stdio.h>
#include <stddef.h>
void next(void);
void init_lexer(void);
void exit_lexer(void);
size_t check_line_prev(size_t line, size_t column);
void print_line(size_t line);
void shift_end(size_t line);
void shift_line(size_t line, size_t column);
void print_token(enum token t);
void print_token_underline(enum token t);
void print_token_indicater(enum token t);
void lexer_token_seek(size_t line, size_t column);
extern queue_t gl_queue;
extern FILE *source;
extern char const *source_filename;
#endif // LEXER_H_
