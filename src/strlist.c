#include "strlist.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

static char *push_front(strlist_t *list, char *str)
{
	struct strnode *nd = (struct strnode *)calloc(1, sizeof(struct strnode));
	if (!nd) {
		fprintf(stderr, "ERROR: STRLIST: cannot allocate memory\n");
		exit(1);
	}
	nd->next = list->first;
	list->first = nd;
	nd->str = str;
	return nd->str;
}

void delete(strlist_t *list)
{
	struct strnode *ptr;
	ptr = list->first;
	while (ptr) {
		list->first = ptr->next;
		free(ptr->str);
		free(ptr);
		ptr = list->first;
	}
	free(list);
}

strlist_t *init_strlist(void)
{
	strlist_t *res;
	if (!(res = (strlist_t *)calloc(1, sizeof(res)))) {
		return NULL;
	}
	res->first = NULL;
	res->push_front = push_front;
	res->delete = delete;
	return res;
}
