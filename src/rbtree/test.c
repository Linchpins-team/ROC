#include "node.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h> 

unsigned int * random_array(int);

int main() 
{
	// begin test
	int length = 20;
	unsigned int * array = random_array(length);
	for (int i = 0; i < length; i++) {
		printf("%d ", array[i]);
	}
	printf("\n");
	node_t n = new_tree();

	for (int i = 0; i < length; i++) {
		insert(n, n, &array[i]);
	}

	print_tree(n);
}

unsigned int * random_array(int length)
{
	time_t t;
	srand((unsigned) time(&t));
	unsigned int * array = (unsigned int *) malloc(length * sizeof(unsigned int));
	for (int i = 0; i < length; i++) {
		array[i] = i;
	}
	for (int i = 0; i < length; i++) {
		int n = rand() % (length - i);
		unsigned int swap = array[i];
		array[i] = array[n+i];
		array[n+i] = swap;
	}	
	return array;
}
