#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdlib.h>
#include <assert.h>

typedef struct {
	void **elems;
	size_t size;
	size_t elem_size;
	size_t cap;
	void (*free_elem)(void *);
} array;

#define INIT_ARRAY(a, cap, elem_size) init_array_f((a), (cap), (elem_size), NULL)

void free_array(array *a) {
	void (*ff)(void *) = a->free_elem ? a->free_elem : free;
	for (size_t i = 0; i < (a)->size; ++i) {
		ff(a->elems[i]);
	}
	free(a->elems);
	a->elems = NULL;
	a->size = 0;
	a->cap = 0;
	a->elem_size = 0;
}

void init_array_f(array *a, size_t cap, size_t elem_size, void (*free_elem)(void *)) {
	a->elems = malloc(elem_size * cap * sizeof(char));
	assert(a->elems);
	a->size = 0;
	a->cap = cap;
	a->elem_size = elem_size;
	a->free_elem = free_elem;
}

void add_array(array *a, void *e) {
	if (a->size + 1 > a->cap) { // >
		a->cap *= 2;
		void *m = realloc(a->elems, a->cap * a->elem_size);
		assert(m);
		a->elems = m;
	}
	a->elems[a->size] = e;
	++a->size;
}

#endif
