#include "strlist.h"
#include <stddef.h>
#include <string.h>

static char const *push_front(strlist_t *list, char const *str)
{
	struct strnode *nd = (struct strnode *)calloc(1, sizeof(struct strnode));
	if (!(res = (strlist_t *)calloc(1, sizeof(res)))) {
		return NULL;
	}
	nd->next = list->first;
	list->first = next;
	nd->str = (char *)malloc(sizeof(char) * 1024);
	strcpy(nd->str, str);
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
