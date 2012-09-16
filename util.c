#include "util.h"
#include <stdio.h>

void *salloc(size_t size) {
	void *memory = calloc(1, size);
	
	if (!memory) {
		printf("Out of memory\n");
		exit(1);
	}
	
	return memory;
}