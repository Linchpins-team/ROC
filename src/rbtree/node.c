#include<stdio.h>
#include<stdlib.h>
#include "node.h"

void insert(node *n, node *nil, value v)
{
	node **child = select_child(n, nil, v);
	if (child == NULL) {
		// itself
		return;
	}
	if (*child == NULL) {
		// child hasn't created
		node* c = (node *) malloc(sizeof(node));
		c->parent = n;
		c->color = RED;
		c->value = &v;
		*child = c;
		//case1(c, nil);
	} else {
		insert(*child, nil, v);
	}
}

node** select_child(node *n, node *nil, value v) 
{
	if (n == nil || v < *n->value) {
		return &n->left;
	}
	if (v > *n->value) {
		return &n->right;
	}
	return NULL;
}

void delete(node *n) 
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
