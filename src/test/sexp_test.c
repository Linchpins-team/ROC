#include "sexp.h"
#include <stdio.h>

int main(void)
{
	sexp_t *sexp = sexp_tmp();
	sexp_push(sexp, "+");
	sexp_new_data(sexp, "3");
	sexp_push(sexp, "*");
	sexp_push(sexp, "+");
	sexp_new_data(sexp, "5");
	sexp_new_data(sexp, "3");
	sexp_pop(sexp);
	sexp_new_data(sexp, "9");
	sexp_pop(sexp);
	sexp_pop(sexp);
	sexp_print(sexp);
	sexp_close(sexp);
}
