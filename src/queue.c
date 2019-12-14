#include "queue.h"
#include <assert.h>
#include <stdlib.h>

static void push_queue(struct queue *q, int value)
{
	struct queue_node *new_node = (struct queue_node *)calloc(1, sizeof(struct queue_node));
	new_node->prev = (void *)0;
	new_node->value = value;
	if (q->first != (void *)0) {
		q->first->prev = new_node;
	} else {
		q->last = new_node;
	}
	q->first = new_node;
}

static void pop_queue(struct queue *q)
{
	if (q->last == (void *)0) {
		return;
	}

	if (q->last == q->first) {
		free(q->last);
		q->first = q->last = (void *)0;
	} else {
		struct queue_node *tmp = q->last;
		q->last = q->last->prev;
		free(tmp);
	}
}

static int front_queue(struct queue *q)
{
	if (q->first != (void *)0) {
		return q->first->value;
	}
	assert(0);
}

static int back_queue(struct queue *q)
{
	if (q->last != (void *)0) {
		return q->last->value;
	}
	assert(0);
}

void init_queue(struct queue **q)
{
	if (q == (void *)0) {
		return;
	}

	*q = calloc(1, sizeof(struct queue));
	if (*q == (void *)0) {
		return;
	}

	(*q)->first = (void *)0;
	(*q)->last = (void *)0;
	(*q)->push = push_queue;
	(*q)->pop = pop_queue;
	(*q)->front = front_queue;
	(*q)->back = back_queue;
}

void clear_queue(queue_t q)
{
	while (q->first != (void *)0) {
		q->pop(q);
	}
	free(q);
}