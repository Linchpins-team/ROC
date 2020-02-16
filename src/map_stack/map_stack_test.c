#include "map_stack.h"
#include <stdio.h>
#include <stddef.h>

int main(void)
{
	mapstack_t m;
	init_map_stack(&m);
	m->insert(m, 100, NULL);
	m->insert(m, 101, NULL);
	m->insert(m, 102, NULL);
	m->insert(m, 103, NULL);
	m->push(m);
	printf("%d\n", m->exist(m, 100));
	printf("%d\n", m->exist(m, 101));
	printf("%d\n", m->exist(m, 102));
	printf("%d\n", m->exist(m, 103));
	m->insert(m, 103, NULL);
	m->insert(m, 104, NULL);
	printf("%d\n", m->exist(m, 103));
	printf("%d\n", m->exist(m, 104));
	m->pop(m);
	printf("%d\n", m->exist(m, 103));
	printf("%d\n", m->exist(m, 104));
	free_map_stack(m);
}
