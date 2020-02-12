#ifndef AALLOC_H_
#define AALLOC_H_
#include <stddef.h>
#include "ast.h"
void init_aalloc(void);
ast_t *acalloc(size_t nitem, size_t size);
void aalloc_delete(void);
#endif /* AALLOC_H_ */
