#ifndef SEXP_H_
#define SEXP_H_
#include <stdio.h>
typedef struct {
	FILE *file;
	int floor;
} sexp_t;

struct sexp_node {
	struct sexp_node *parent;
	struct sexp_node *next;
	struct sexp_node *prev;
	struct sexp_node *first_child;
	struct sexp_node *last_child;
	const char *type;
};

sexp_t *sexp_tmp(void) __attribute__((warn_unused_result));
sexp_t *sexp_open(const char *file, const char *attr) __attribute__((warn_unused_result));

void sexp_close(sexp_t *);

// Chapter 1: write an S-Exp

// add a child S-Exp node
int sexp_push(sexp_t *, const char *type_name);

// leave the child S-Exp for the parent S-Exp
int sexp_pop(sexp_t *);

// add a data
int sexp_new_data(sexp_t *, const char *data);

// add layer
int sexp_add_layer(sexp_t *, const char *type_name);

// print all the S-Exp
int sexp_print(sexp_t const *);
#endif // SEXP_H_
