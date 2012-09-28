#include "util.h"
#include <stdio.h>

void *salloc(size_t bytes) {
	void *mem = calloc(1, bytes);
	
	if (!mem) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	
	return mem;
}

void *srealloc(void *mem, size_t bytes) {
	void *new_mem = realloc(mem, bytes);
	
	if (!new_mem) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	
	return new_mem;
}
