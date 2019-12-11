#include <stdio.h>
#include <ctype.h>

FILE *source;

enum token {PLUS, MINUS, TIMES, DIVIDE, ASSIGN, NUMBER, NAME};

enum token current_token;

unsigned int id_count = 0;
struct identifier {
	unsigned int hash;
	unsigned int type;
	void *memory;
	char name[1024];
} *id_array;

unsigned int hash(const char *s)
{
	unsigned int seed, hash;
	seed = 131;
	hash = 0;

	while (*s) {
		hash = hash * seed + *s++;
	}
	return hash & 0x7FFFFFFFU;
}

void next(void)
{
	int c = fgetc(source);
	if (isalpha(c) || c == '_') {
		char name[1024];
		name[0] = c;
		for (int i = 1; isalnum(c = fgetc(source)) || c == '_'; ++i) {
			name[i] = c;
		}
		fseek(source, -1, SEEK_CUR); // from stdio.h

		hash(name);
	}
	if (isdigit(c)
}
