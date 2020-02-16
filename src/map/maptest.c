#include "map.h"
#include <stdio.h>
#include <stddef.h>
int main(void)
{
	map_t map;
	init_map(&map);
	for (int i = 20; i < 100; ++i) {
		map->insert(map, i, NULL);
	}
	for (int i = 20; i < 100; i+=2) {
		map->delete(map, i);
	}
	for (int i = 20; i < 100; ++i) {
		printf("%d\n", map->exist(map, i));
	}
	free_map(map);
}
