#ifndef LEXER_H_
#define LEXER_H_
#include <stdio.h>
void next(void);
void init_lexer(void);
void exit_lexer(void);
extern queue_t gl_queue;
extern FILE *source;
#endif // LEXER_H_
