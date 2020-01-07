#define RED 0
#define BLACK 1

typedef unsigned int value;

typedef struct node_t {
	struct node_t *parent;
	struct node_t *left;
	struct node_t *right;
	int color;
	value *value;
} node;

void insert(node*, node*, value);

node** select_child(node*, node*, value);

void delete(node*);

void case1(node*, node*);
void case2(node*);
void case3(node*);
void case4(node*);
void case5(node*);
