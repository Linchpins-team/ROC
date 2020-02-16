#ifndef IDHANDLER_H_
#define IDHANDLER_H_

/*
	type :=
	char *str

	"tesarg"
		t : typedef
		e : extern
		s : static
		a : auto
		r : register
		g : global

	ss : short | short int | signed short | signed short int
	us : unsigned short | unsigned short int
	si : int | signed int | signed
	ui : unsigned | unsigned int
	sl : long
	ul : unsigned long

	st : structure
	un : union
	en : enum

	p[cn][vn] : pointer to
		c : const
		v : volatile
		n : none
*/

enum id_type {
	ID_LABEL = 0,
	ID_TAG,
	ID_MEMBER,
	ID_TYPE,
	ID_NORM,
};

struct id_node {
	unsigned int hash;
	unsigned int number;
	char *type;
	const char *name;
	struct id_node *next;
};

struct id_namespace {
	int scope; // 0 - file, 1 - function , 2 , 3 ... block
	struct id_node *label_name; // function-scope
	struct id_node **tags; // union struct enum tags
	struct id_node **member; // struct union members
	struct id_node **ordinary; // others
	struct id_node **type;
};

struct id_node *push_id(char const *name, enum id_type t);
int check_id(char const *name, enum id_type); // 1 - exists 0 - not exists

void in_function_scope(void);
void out_function_scope(void);

void in_block_scope(void);
void out_block_scope(void);

void init_id_namespace(void);

#endif /* IDHANDLER_H_ */
