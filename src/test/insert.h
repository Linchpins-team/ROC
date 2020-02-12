#ifndef INSERT_H_
#define INSERT_H_
#include <stdio.h>
#include <stdarg.h>

void start_insert(FILE *file);
void end_insert(FILE *file);
void insert_putc(int c, FILE *file);
void insert_puts(const char *str, FILE *file);
void insert_printf(FILE *file, const char *format, ...);

#endif // INSERT_H_
