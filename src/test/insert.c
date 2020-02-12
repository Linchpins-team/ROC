#include "insert.h"
#include <stdio.h>
#include <stdarg.h>

static size_t buffer_size;
static size_t current_size;
static char *buffer;
static void copy_to_buffer(FILE *file)
{
	buffer_size = 1024;
	current_size = 0;
	buffer = malloc(buffer_size * sizeof(char));
	assert(buffer);
	long int cur = ftell(file);
	static char *ptr = buffer;
	while (1) {
		ptr = fgets(ptr, buffer_size - current_size, file);
		if (ptr == NULL) {
			break;
		}
	}
}
void start_insert(FILE *file)
{
}
