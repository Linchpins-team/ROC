#ifndef QUEUE_H_
#define QUEUE_H_
typedef struct queue {
	struct queue_node *first;
	struct queue_node *last;
	void (*push)(struct queue *q, int value);
	void (*pop)(struct queue *q);
	int (*front)(struct queue *q);
	int (*back)(struct queue *q);
} *queue_t;
struct queue_node {
	struct queue_node *prev;
	int value;
};
void init_queue(struct queue **q);
void clear_queue(queue_t q);
#endif /* QUEUE_H_ */
