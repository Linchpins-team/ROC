#include "idhandler.h"
#include <stdlib.h>
#include <assert.h>

static struct id_namespace glspace;

static unsigned int hash(const char *s)
{
	unsigned int seed, hash;
	seed = 131;
	hash = 0;

	while (*s) {
		hash = hash * seed + *s++;
	}
	return hash & 0x7FFFFFFFU;
}

static void delete_list(struct id_node *head)
{
	while (head != NULL) {
		struct id_node *ptr = head;
		head = head->next;
		free(ptr);
	}
}

void init_id_namespace(void)
{
	glspace.scope = 0;
	glspace.label_name = NULL;
	glspace.tags = calloc(1, sizeof(struct id_node *));
	glspace.member = calloc(1, sizeof(struct id_node *));
	glspace.type = calloc(1, sizeof(struct id_node *));
}

static struct id_node *push(struct id_node **head, char const *name, enum id_type t)
{
	struct id_node *ptr = calloc(1, sizeof(struct id_node));
	if (ptr == NULL) {
		exit(1);
	}

	ptr->next = *head;
	*head = ptr;

	ptr->name = name;
	ptr->hash = hash(name);
	return ptr;
}

static int check(struct id_node *head, char const *name)
{
	unsigned int h = hash(name);
	while (head != NULL) {
		if (head->hash == h) {
			return 1;
		}
		head = head->next;
	}
	return 0;
}

static struct id_node *push_label(char const *name)
{
	return push(&glspace.label_name, name, ID_LABEL);
}

static int check_label(char const *name)
{
	return check(&glspace.label_name, name);
}

static struct id_node *push_type(char const *name)
{
	return push(&glspace.type[glspace.scope], name, ID_TYPE);
}

static int check_type(char const *name)
{
	return check(glspace.type[glspace.scope], name);
}

static struct id_node *push_tags(char const *name)
{
	return push(&glspace.tags[glspace.scope], name, ID_TAG);
}

static int check_tags(char const *name)
{
	return check(glspace.tags[glspace.scope], name);
}

static struct id_node *push_ordinary(char const *name)
{
	return push(&glspace.ordinary[glspace.scope], name, ID_NORM);
}

static int check_ordinary(char const *name)
{
	return check(glspace.ordinary[glspace.scope], name);
}

struct id_node *push_id(char const *name, enum id_type t)
{
	switch (t) {
	case ID_LABEL:
		return push_label(name);
	case ID_TAG:
		return push_tags(name);
	case ID_TYPE:
		return push_type(name);
	case ID_NORM:
		return push_ordinary(name);
	default:
		return NULL;
	}
}

int check_id(char const *name, enum id_type)
{
	switch (t) {
	case ID_LABEL:
		return check_label(name);
	case ID_TAG:
		return check_tags(name);
	case ID_TYPE:
		return check_type(name);
	case ID_NORM:
		return check_ordinary(name);
	default:
		return 0;
	}
}

void in_function_scope(void)
{
	assert(glspace.scope == 0);
	scope = 1;
}
