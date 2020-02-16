#ifndef STRLIST_H_
#define STRLIST_H_
struct strnode {
	char *str;
	struct strnode *next;
};
typedef struct string_list {
	struct strnode *first;
	char *(*push_front)(struct string_list *list, char *str);
	void (*delete)(struct string_list *list);
} strlist_t;

strlist_t *init_strlist(void);

#endif /* STRLIST_H_ */
