#ifndef __DUMB_STRING__
#define __DUMB_STRING__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
	char *buf;
	size_t len;
	size_t cap;
} dumb_string;

void init_dumb_string(dumb_string *ds, char *buf, size_t cap) {
	size_t len = strlen(buf);
	#ifdef DEBUG
	if (cap) {
		assert(len <= cap);
	}
	#endif
	ds->cap = cap ? cap : len * 2;
	ds->buf = malloc((ds->cap + 1) * sizeof(char));
        assert(ds->buf);
	memcpy(ds->buf, buf, len);
	ds->buf[len] = '\0';
	ds->len = len;
}

#define INIT_DUMB_STRING_DEF(ds, buf) init_dumb_string((ds), (buf), 0);

void append_dumb_string(dumb_string *ds, char *a) {
	size_t a_len = strlen(a);
	if (a_len + ds->len >= ds->cap) {
		ds->cap += a_len * 2;
		void *m = realloc(ds->buf, (ds->cap + 1) * sizeof(char));
	        assert(m);
		ds->buf = m;
	}
	memcpy(ds->buf + ds->len * sizeof(char), a, a_len);
	ds->len += a_len;
	ds->buf[ds->len] = '\0';
}

void append_dumb_string_char(dumb_string *ds, char c) {
	if (1 + ds->len >= ds->cap) {
		ds->cap += 8;
		void *m = realloc(ds->buf, (ds->cap + 1) * sizeof(char));
	        assert(m);
		ds->buf = m;
	}
	ds->buf[ds->len] = c;
	++ds->len;
	ds->buf[ds->len] = '\0';
}

extern inline void free_dumb_string(dumb_string *ds) {
	free(ds->buf);
	ds->buf = NULL;
	ds->len = 0;
	ds->cap = 0;
}

extern inline void clear_dumb_string(dumb_string *ds) {
	memset(ds->buf, 0, ds->len);
	ds->len = 0;
}

#endif
