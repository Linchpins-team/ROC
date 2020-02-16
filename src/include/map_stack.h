#ifndef MAP_STACK_H_
#define MAP_STACK_H_
#include "map.h"
#include <stddef.h>

typedef struct map_stack *mapstack_t;

struct map_stack {
	size_t map_arr_pushtime;
	size_t map_arr_size;
	map_t *map_arr;

	void (*insert)(mapstack_t ms, unsigned int key, void *value);
	int (*exist)(mapstack_t ms, unsigned int key);
	void *(*search)(mapstack_t ms, unsigned int key);
	void *(*search_cur)(mapstack_t ms, unsigned int key);
	void (*delete)(mapstack_t ms, unsigned int key);

	void (*push)(mapstack_t ms);
	void (*pop)(mapstack_t ms);
};

void init_map_stack(mapstack_t *stack);
void free_map_stack(mapstack_t stack);

#endif /* MAP_STACK_H_ */
