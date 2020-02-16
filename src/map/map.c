#include "map.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

static void left_rotate(map_t m, struct map_node *nd)
{
	struct map_node *ndrchild = nd->right;
	nd->right = ndrchild->left;
	if (nd->right != m->nil) {
		nd->right->parent = nd;
	}
	ndrchild->parent = nd->parent;
	if (nd == nd->parent->left) {
		nd->parent->left = ndrchild;
	} else {
		nd->parent->right = ndrchild;
	}
	ndrchild->left = nd;
	nd->parent = ndrchild;
}

static void right_rotate(map_t m, struct map_node *nd)
{
	struct map_node *ndrchild = nd->left;
	nd->left = ndrchild->right;
	if (nd->left != m->nil) {
		nd->left->parent = nd;
	}
	ndrchild->parent = nd->parent;
	if (nd == nd->parent->left) {
		nd->parent->left = ndrchild;
	} else {
		nd->parent->right = ndrchild;
	}
	ndrchild->right = nd;
	nd->parent = ndrchild;
}

static struct map_node *
rb_insert_fix_left(map_t m, struct map_node *nd)
{
	struct map_node *uncle = nd->parent->parent->right;
	if (uncle->color) { /* RED */
		nd->parent->color = 0;
		uncle->color = 0;
		nd->parent->parent->color = 1;
		nd = nd->parent->parent;
	} else { /* UNCLE is Black */
		if (nd == nd->parent->right) {
			nd = nd->parent;
			left_rotate(m, nd);
		}
		nd->parent->color = 0;
		nd->parent->parent->color = 1;
		right_rotate(m, nd->parent->parent);
	}
	return nd;
}

static struct map_node *
rb_insert_fix_right(map_t m, struct map_node *nd)
{
	struct map_node *uncle = nd->parent->parent->left;
	if (uncle->color) { /* RED */
		nd->parent->color = 0;
		uncle->color = 0;
		nd->parent->parent->color = 1;
		nd = nd->parent->parent;
	} else { /* UNCLE is black */
		if (nd == nd->parent->left) {
			nd = nd->parent;
			right_rotate(m, nd);
		}
		nd->parent->color = 0;
		nd->parent->parent->color = 1;
		left_rotate(m, nd->parent->parent);
	}
	return nd;
}

static void rb_insert_fix(map_t m, struct map_node *nd)
{
	/* while parent is red */
	while (nd->parent->color) {
		if (nd->parent == nd->parent->parent->left) {
			nd = rb_insert_fix_left(m, nd);
		} else {
			nd = rb_insert_fix_right(m, nd);
		}
	}
	if (m->nil->left != m->nil) {
		m->root = m->nil->left;
		m->nil->left = m->nil;
	} else if (m->nil->right != m->nil) {
		m->root = m->nil->right;
		m->nil->right = m->nil;
	}
	m->root->color = 0;
}

static void rb_insert(map_t m, unsigned int key, void *value)
{
	struct map_node *nd = (struct map_node *)calloc(1, sizeof(struct map_node));
	if (nd == NULL) {
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}

	nd->key = key;
	nd->value = value;
	nd->left = m->nil;
	nd->right = m->nil;
	nd->color = 1; /* RED */

	struct map_node *new_parent = m->nil;
	struct map_node *front = m->root;
	while (front != m->nil) {
		new_parent = front;
		if (nd->key < front->key) {
			front = front->left;
		} else {
			front = front->right;
		}
	}

	nd->parent = new_parent;
	if (new_parent == m->nil) {
		m->root = nd;
	} else if (nd->key < new_parent->key) {
		new_parent->left = nd;
	} else {
		new_parent->right = nd;
	}

	rb_insert_fix(m, nd);
}

static struct map_node *
rb_search_node(map_t m, unsigned int key)
{
	struct map_node *nd = m->root;
	while (nd != m->nil && nd->key != key) {
		if (key < nd->key) {
			nd = nd->left;
		} else { /* key > nd->key */
			nd = nd->right;
		}
	}
	return nd == m->nil ? NULL : nd;
}

static struct map_node *
subtree_leftmost(map_t m, struct map_node *nd)
{
	while (nd->left != m->nil) {
		nd = nd->left;
	}
	return nd;
}

static struct map_node *
subtree_rightmost(map_t m, struct map_node *nd)
{
	while (nd->right != m->nil) {
		nd = nd->right;
	}
	return nd;
}

static struct map_node *
inorder_successor(map_t m, struct map_node *nd)
{
	if (nd->right != m->nil) {
		return subtree_leftmost(m, nd->right);
	}

	struct map_node *successor = nd->parent;
	while (successor != m->nil && nd == successor->right) {
		nd = successor;
		successor = successor->parent;
	}
	return successor;
}

static struct map_node *
rb_delete_fix_left(map_t m, struct map_node *nd)
{
	struct map_node *sister = nd->parent->right;
	if (sister->color == 1) { /* sister is RED */
		sister->color = 0;
		nd->parent->color = 1;
		left_rotate(m, nd->parent);
		sister = nd->parent->right;
	}

	if (sister->left->color == 0 && sister->right->color == 0) {
		sister->color = 1;
		nd = nd->parent;
	} else {
		if (sister->right->color == 0) {
			sister->left->color = 0;
			sister->color = 1;
			right_rotate(m, sister);
			sister = nd->parent->right;
		}
		sister->color = nd->parent->color;
		nd->parent->color = 0;
		sister->right->color = 0;
		left_rotate(m, nd->parent);
		nd = m->root;
	}
	return nd;
}

static struct map_node *
rb_delete_fix_right(map_t m, struct map_node *nd)
{
	struct map_node *sister = nd->parent->left;
	if (sister->color == 1) { /* sister is RED */
		sister->color = 0;
		nd->parent->color = 1;
		right_rotate(m, nd->parent);
		sister = nd->parent->left;
	}

	if (sister->right->color == 0 && sister->left->color == 0) {
		sister->color = 1;
		nd = nd->parent;
	} else {
		if (sister->left->color == 0) {
			sister->right->color = 0;
			sister->color = 1;
			left_rotate(m, sister);
			sister = nd->parent->left;
		}
		sister->color = nd->parent->color;
		nd->parent->color = 0;
		sister->left->color = 0;
		right_rotate(m, nd->parent);
		nd = m->root;
	}
	return nd;
}

static void
rb_delete_fix(map_t m, struct map_node *nd)
{
	while (nd != m->root && nd->color == 0) {
		if (nd == nd->parent->left) {
			nd = rb_delete_fix_left(m, nd);
		} else {
			nd = rb_delete_fix_right(m, nd);
		}
	}
	nd->color = 0;
}

static void rb_delete(map_t m, unsigned int key)
{
	struct map_node *nd = rb_search_node(m, key);
	if (nd == m->nil) {
		return;
	}
	struct map_node *real_delete;
	struct map_node *real_delete_child;
	if (nd->left == m->nil || nd->right == m->nil) {
		real_delete = nd;
	} else {
		real_delete = inorder_successor(m, nd);
	}

	if (real_delete->left != m->nil) {
		real_delete_child = real_delete->left;
	} else {
		real_delete_child = real_delete->right;
	}

	real_delete_child->parent = real_delete->parent;
	if (real_delete->parent == m->nil) {
		m->root = real_delete_child;
	} else if (real_delete == real_delete->parent->left) {
		real_delete->parent->left = real_delete_child;
	} else {
		real_delete->parent->right = real_delete_child;
	}

	if (real_delete != nd) {
		nd->key = real_delete->key;
		nd->value = real_delete->value;
	}

	free(real_delete);

	if (real_delete->color == 0) {
		rb_delete_fix(m, real_delete_child);
	}
}

static void *
rb_search(map_t m, unsigned int key)
{
	struct map_node * nd = rb_search_node(m, key);
	if (nd == NULL) {
		return NULL;
	}
	return nd->value;
}

static int
rb_exist(map_t m, unsigned int key)
{
	return rb_search_node(m, key) != NULL;
}

void init_map(map_t *m)
{
	if (m != NULL) {
		map_t new_m = (map_t)calloc(1, sizeof(struct map));
		if (new_m == NULL) {
			fprintf(stderr, "cannot allocate memory\n");
			exit(1);
		}

		new_m->nil = (struct map_node *)calloc(1, sizeof(struct map_node));
		if (new_m->nil == NULL) {
			fprintf(stderr, "cannot allocate memory\n");
			exit(1);
		}
		new_m->nil->color = 0;
		new_m->nil->parent = new_m->nil;
		new_m->nil->left = new_m->nil;
		new_m->nil->right = new_m->nil;
		new_m->root = new_m->nil;

		new_m->search = rb_search;
		new_m->insert = rb_insert;
		new_m->exist = rb_exist;
		new_m->delete = rb_delete;

		*m = new_m;
	}
}

static void
free_all(map_t m, struct map_node *nd)
{
	if (nd->left != m->nil) {
		free_all(m, nd->left);
	}
	if (nd->right != m->nil) {
		free_all(m, nd->right);
	}
	free(nd->value);
	free(nd);
}

void free_map(map_t m)
{
	if (m != NULL) {
		if (m->root != m->nil) {
			free_all(m, m->root);
		}
		free(m->nil);
		free(m);
	}
}
