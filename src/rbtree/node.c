#include<stdio.h>
#include<stdlib.h>
#include "node.h"

node_t new_tree()
{
	node_t nil = (node_t) malloc(sizeof(struct node));
	return nil;
}

void insert(node_t n, node_t nil, value v)
{
	node_t *child = select_child(n, nil, v);
	if (child == NULL) {
		// itself
		return;
	}
	if (*child == NULL) {
		// child hasn't created
		node_t c = (node_t) malloc(sizeof(struct node));
		c->parent = n;
		c->color = RED;
		c->value = v;
		*child = c;
		//case1(c, nil);
	} else {
		insert(*child, nil, v);
	}
}

node_t* select_child(node_t n, node_t nil, value v) 
{
	if (n == nil || *v < *n->value) {
		return &n->left;
	}
	if (*v > *n->value) {
		return &n->right;
	}
	return NULL;
}

void delete(node_t n) 
{
	if (n == NULL) {
		return;
	}
	delete(n->left);
	free(n->left);
	n->left = NULL;

	delete(n->right);
	free(n->right);
	n->right = NULL;

	n->value = NULL;
}

value search(node_t n, unsigned int key) 
{
	if (n == NULL) {
		return NULL;
	}
	// if n is NIL
	if (n->parent == NULL || key < *n->value) {
		return search(n->left, key);
	}
	if (key > *n->value) {
		return search(n->right, key);
	}
	return n->value;
}

void print_tree(node_t n) 
{
	if (n == NULL) {
		return;
	}
	printf("( ");
	print_tree(n->left);
	printf(" %d ", *n->value);
	print_tree(n->right);
	printf(" )");
}
