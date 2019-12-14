#include "queue.h"

#include <stdio.h>


int main(void)
{
	queue_t q;
	int arr[] = {7, 9, 8, 13, 17, 22, 129, 211};
	init_queue(&q);
	for (int i = 0; i < 5; ++i) {
		printf("%d\n", arr[i]);
		q->push(q, arr[i]);
	}
	for (int i = 0; i < 5; ++i) {
		printf("%d\n", q->back(q));
		q->pop(q);
	}
	clear_queue(q);
	return 0;
}
