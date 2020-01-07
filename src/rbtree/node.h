#define RED 0
#define BLACK 1

typedef unsigned int value;

typedef struct {
	node *parent;
	node *left;
	node *right;
	int color;
	value *value;
} node;

typedef struct {
	node *nil;
	node *node;
} w_node;

void insert(w_node, value);

node** select_child(w_node, value);

w_node work(node*, node*);

