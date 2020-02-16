#include "map_stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

static void mapstack_push(mapstack_t ms)
{
	++ms->map_arr_pushtime;
	if (ms->map_arr_pushtime >= ms->map_arr_size) {
		ms->map_arr_size *= 2;
		ms->map_arr = (map_t *)realloc(ms->map_arr, ms->map_arr_size * sizeof(map_t));
		if (ms->map_arr == NULL) {
			fprintf(stderr, "cannot allocate memory\n");
			exit(1);
		}
	}
	init_map(&ms->map_arr[ms->map_arr_pushtime]);
}

static void mapstack_pop(mapstack_t ms)
{
	free_map(ms->map_arr[ms->map_arr_pushtime]);
	--ms->map_arr_pushtime;
}

static void mapstack_insert(mapstack_t ms, unsigned int key, void *value)
{
	ms->map_arr[ms->map_arr_pushtime]->insert(ms->map_arr[ms->map_arr_pushtime], key, value);
}

static int mapstack_exist(mapstack_t ms, unsigned int key)
{
	for (size_t i = ms->map_arr_pushtime; i > 0; --i) {
		if (ms->map_arr[i]->exist(ms->map_arr[i], key)) {
			return 1;
		}
	}
	if (ms->map_arr[0]->exist(ms->map_arr[0], key)) {
		return 1;
	}
	return 0;
}

static void *mapstack_search(mapstack_t ms, unsigned int key)
{
	for (size_t i = ms->map_arr_pushtime; i > 0; --i) {
		if (ms->map_arr[i]->exist(ms->map_arr[i], key)) {
			return ms->map_arr[i]->search(ms->map_arr[i], key);
		}
	}
	if (ms->map_arr[0]->exist(ms->map_arr[0], key)) {
		return ms->map_arr[0]->search(ms->map_arr[0], key);
	}
	return NULL;
}

static void *mapstack_search_cur(mapstack_t ms, unsigned int key)
{
	return ms->map_arr[ms->map_arr_pushtime]->search(ms->map_arr[ms->map_arr_pushtime], key);
}

static void mapstack_delete(mapstack_t ms, unsigned int key)
{
	ms->map_arr[ms->map_arr_pushtime]->delete(ms->map_arr[ms->map_arr_pushtime], key);
}

void init_map_stack(mapstack_t *stack)
{
	if (stack != NULL) {
		mapstack_t s = (mapstack_t)calloc(1, sizeof(struct map_stack));
		if (s == NULL) {
			fprintf(stderr, "cannot allocate memory\n");
			exit(1);
		}

		s->map_arr_size = 1;
		s->map_arr_pushtime = 0;
		s->map_arr = (map_t *)calloc(s->map_arr_size, sizeof(map_t));
		if (s->map_arr == NULL) {
			fprintf(stderr, "cannot allocate memory\n");
			exit(1);
		}

		init_map(&s->map_arr[0]);

		s->insert = mapstack_insert;
		s->exist = mapstack_exist;
		s->search = mapstack_search;
		s->search_cur = mapstack_search_cur;
		s->delete = mapstack_delete;
		s->push = mapstack_push;
		s->pop = mapstack_pop;
		*stack = s;
	}
}

void free_map_stack(mapstack_t stack)
{
	for (size_t i = 0; i <= stack->map_arr_pushtime; ++i) {
		free_map(stack->map_arr[i]);
	}
	free(stack->map_arr);
	free(stack);
}
