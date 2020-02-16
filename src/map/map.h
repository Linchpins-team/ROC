#ifndef MAP_H_
#define MAP_H_
typedef struct map *map_t;
struct map_node {
	unsigned int key;
	void *value;
	int color; // 0 - black 1 - red
	struct map_node *parent;
	struct map_node *left;
	struct map_node *right;
};

struct map {
	struct map_node *root;
	struct map_node *nil;
	void (*insert)(map_t m, unsigned int key, void *value);
	int (*exist)(map_t m, unsigned int key);
	void *(*search)(map_t m, unsigned int key);
	void (*delete)(map_t m, unsigned int key);
};

void init_map(map_t *m);
void free_map(map_t m);
#endif /* MAP_H_ */
