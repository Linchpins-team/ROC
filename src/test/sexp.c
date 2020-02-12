#include "sexp.h"
#include <stdio.h>
#include <stdlib.h>

sexp_t *sexp_open(const char *file, const char *attr)
{
	sexp_t *ret = (sexp_t *)calloc(1, sizeof(sexp_t));
	if (ret == NULL) {
		return NULL;
	}
	ret->file = fopen(file, attr);
	if (ret->file == NULL) {
		free(ret);
		return NULL;
	}
	ret->floor = 0;
	return ret;
}

sexp_t *sexp_tmp(void)
{
	sexp_t *ret = (sexp_t *)calloc(1, sizeof(sexp_t));
	if (ret == NULL) {
		return NULL;
	}

	ret->file = tmpfile();

	if (ret->file == NULL) {
		free(ret);
		return NULL;
	}
	ret->floor = 0;
	return ret;
}

void sexp_close(sexp_t *desc)
{
	if (desc != NULL) {
		if (desc->file != NULL) {
			fclose(desc->file);
		}
		free(desc);
	}
}

static void new_line_tab(sexp_t *desc)
{
	fputc('\n', desc->file);
	for (int i = 1; i < desc->floor; ++i) {
		fputc('\t', desc->file);
	}
}

int sexp_push(sexp_t *desc, const char *type_name)
{
	if (desc->floor++) {
		new_line_tab(desc);
	}
	fprintf(desc->file, "(%s", type_name);
	return 0;
}

int sexp_pop(sexp_t *desc)
{
	fputc(')', desc->file);
	--(desc->floor);
	return 0;
}

int sexp_new_data(sexp_t *desc, const char *data)
{
	fprintf(desc->file, " %s", data);
	return 0;
}

// Check the current word is a escape character
static int is_escape(FILE *file)
{
	fseek(file, -1, SEEK_CUR);
	if (fgetc(file) == '\\') {
		fseek(file, -1, SEEK_CUR);
		if (is_escape(file)) {
			fseek(file, 1, SEEK_CUR);
			return 0;
		}
		fseek(file, 1, SEEK_CUR);
		return 1;
	}
	return 0;
}

// Jump to the " sign
static void jump_left_str(FILE *file)
{
	do {
		fseek(file, -2, SEEK_CUR);
		if (fgetc(file) == '\"') {
			fseek(file, -1, SEEK_CUR);
			if (!is_escape(file)) {
				break;
			}
			fgetc(file);
		}
	} while (1);
}

// Jump to the right of the " sign
static void jump_right_str(FILE *file)
{
	do {
		if (fgetc(file) == '\"') {
			fseek(file, -1, SEEK_CUR);
			if (!is_escape(file)) {
				fgetc(file);
				break;
			}
			fgetc(file);
		}
	} while (1);
}

// Start from the next of the right parenthese
static void go_to_left_parentheses(FILE *file)
{
	int right_count = 0;
	fseek(file, -1, SEEK_CUR);
	while (1) {
		int c = fgetc(file);

		if (c == '(') {
			if (--right_count <= 0) {
				break;
			}
		}

		if (c == ')') {
			++right_count;
		}

		if (c == '\"') {
			jump_left_str(file);
		}

		fseek(file, -2, SEEK_CUR);
	}
	fseek(file, -1, SEEK_CUR);
}

// Go to the next of the right parenthese
// from the left parenthese
static void go_to_right(FILE *file)
{
	int left_count = 0;
	while (1) {
		int c = fgetc(file);
		if (c == ')') {
			if (--left_count <= 0) {
				break;
			}
		}
		if (c == '(') {
			++left_count;
		}
		if (c == '\"') {
			jump_right_str(file);
		}
	}
}

static FILE * copy_previous_file(FILE *old)
{
	FILE *ret = tmpfile();
	if (ret == NULL) {
		return NULL;
	}
	long int pos = ftell(old);
	// TODO
	int c = fgetc(old);
	while (c != -1) {
		putc(c, ret);
	}
	fseek(old, pos, SEEK_SET);
	return ret;
}

int sexp_add_layer(sexp_t *desc, const char *type_name)
{
	go_to_left_parentheses(desc->file);
	fprintf(desc->file, "(%s", type_name);
	++(desc->floor);
	new_line_tab(desc);
	--(desc->floor);
	go_to_right(desc->file);
	return 0;
}

int sexp_print(sexp_t const *desc)
{
	long int cur_pos = ftell(desc->file);
	fseek(desc->file, 0, SEEK_SET);
	int c;
	while ((c = fgetc(desc->file)) != -1) {
		putc(c, stdout);
	}
	putc('\n', stdout);
	fseek(desc->file, cur_pos, SEEK_SET);
	return 0;
}
