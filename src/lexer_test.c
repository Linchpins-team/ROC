#include "queue.h"
#include "lexer.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	init_queue(&gl_queue);
	source = fopen(argv[1], "r");
	next();
	while (gl_queue->back(gl_queue).type != END) {
		printf("%d\n", gl_queue->back(gl_queue).type);
		gl_queue->pop(gl_queue);
		next();
	}
	clear_queue(gl_queue);
	fclose(source);
}
