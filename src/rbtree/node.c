#include<stdio.h>
#include<stdlib.h>
#include "node.h"

void insert(w_node n, value v)
{
	node **child = select_child(n, v);
	if (child == NULL) {
		// itself
		return;
	}
	if (*child == NULL) {
		// child hasn't created
		node* c = (node *) malloc(sizeof(node));
		c->parent = n.node;
		c->color = RED;
		c->value = v;
		*child = c;
		case1(work(c, n.nil));
	} else {
		insert(work(*child, n.nil), v);
	}
}

node** select_child(w_node n, value v) 
{
	if (n.node == n.nil || v < *n.node->value) {
		return &n.node->left;
	}
	if (v > *n.node->value) {
		return &n.node->right;
	}
	return NULL;
}

w_node work(node *n, node *nil)
{
	w_node w;
	w.nil = nil;
	w.node = n;
	return w;
}
