#include "array.h"
#include "util.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void array_init_with_element_size(array *a, size_t size) {
	memset(a, 0, sizeof(array));
	a->element_size = size;
}

array *array_alloc_with_element_size(size_t size) {
	array *a = salloc(sizeof(array));
	array_init_with_element_size(a, size);
	return a;
}

void array_push(array *a, void *data) {
	if (!a || !data) return;
	
	size_t space_req = a->element_size * (a->element_count + 1);
	int use_dyn_buf = (a->dynamic_buffer || space_req > STATIC_BUFFER_SIZE);
	void *dest;
	
	if (!use_dyn_buf) {
		dest = a->static_buffer + a->element_count * a->element_size;
	} else {
		if (space_req > a->buffer_size) {
			a->buffer_size = space_req * 4;
			a->dynamic_buffer = srealloc(a->dynamic_buffer, a->buffer_size);
		}
		
		dest = a->dynamic_buffer + a->element_count * a->element_size;
	}
	
	memcpy(dest, data, a->element_size);
	a->element_count++;
}

void *array_pop(array *a) {
	void *obj = array_get(a, a->element_count - 1);
	array_remove_object_at_index(a, a->element_count - 1);
	return obj;
}

unsigned int array_length(array *a) {
	return a->element_count;
}

void *array_get(array *a, unsigned int index) {
	if (a->dynamic_buffer) {
		return a->dynamic_buffer + index * a->element_size;
	} else {
		return a->static_buffer + index * a->element_size;
	}
}

void array_remove_object_at_index(array *a, unsigned int index) {
	void *dest;
	void *from;
	size_t bytes;
	
	if (a->dynamic_buffer) {
		dest = a->dynamic_buffer + index * a->element_size;
		from = a->dynamic_buffer + (index + 1) * a->element_size;
		bytes = a->buffer_size - (from - a->dynamic_buffer);
	} else {
		dest = a->static_buffer + index * a->element_size;
		from = a->static_buffer + (index + 1) * a->element_size;
		bytes = STATIC_BUFFER_SIZE - (from - (void *)a->static_buffer);
	}
	
	memcpy(dest, from, bytes);
	a->element_count--;
}

void array_deinit(array *a) {
	free(a->dynamic_buffer);
	a->dynamic_buffer = NULL;
}

void array_free(array *a) {
	array_deinit(a);
	free(a);
}
