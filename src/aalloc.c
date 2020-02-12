#include "aalloc.h"
#include <stddef.h>
#include <stdlib.h>

static size_t ptr_count = 0;
static size_t arr_size = 0;
static ast_t **autoarr;



static void extend(void)
{
	void *newptr = (void **)realloc(autoarr, (arr_size + 8)*sizeof(void *));
	if (newptr == NULL) {
		exit(1);
	}
	arr_size = arr_size + 8;
	autoarr = newptr;
}

void init_aalloc(void)
{
	ptr_count = 0;
	arr_size = 0;
}

ast_t *acalloc(size_t nitem, size_t size)
{
	if (arr_size <= ptr_count) {
		extend();
	}
	ast_t *nptr = calloc(nitem, size);
	if (nptr == NULL) {
		exit(1);
	}
	autoarr[ptr_count++] = nptr;
	return nptr;
}

void aalloc_delete(void)
{
	for (size_t i = 0; i < ptr_count; ++i) {
		free(autoarr[i]->son_array);
		free(autoarr[i]);
		autoarr[i] = NULL;
	}
	ptr_count = 0;
	arr_size = 0;
	free(autoarr);
	autoarr = NULL;
}
