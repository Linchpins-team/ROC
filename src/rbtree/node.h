#define RED 0
#define BLACK 1

typedef unsigned int * value;

typedef struct node {
	struct node *parent;
	struct node *left;
	struct node *right;
	int color;
	value value;
} *node_t;

node_t new_tree();

void insert(node_t, node_t, value);

node_t* select_child(node_t, node_t, value);

void delete(node_t);

void case1(node_t, node_t);
void case2(node_t);
void case3(node_t);
void case4(node_t);
void case5(node_t);

void print_tree(node_t);
