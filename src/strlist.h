#ifndef STRLIST_H_
#define STRLIST_H_
struct strnode {
	char *str;
	struct strnode *next;
};
typedef struct {
	struct strnode *first;
	char const *(*push_front)(strlist_t *list, char const *str);
	void (*delete)(strlist_t *list);
} strlist_t;

strlist_t *init_strlist(void);

#endif /* STRLIST_H_ */
