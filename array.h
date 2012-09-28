#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

typedef struct array_struct array;

#define STATIC_BUFFER_SIZE 64

struct array_base_struct {
	char static_buffer[STATIC_BUFFER_SIZE];
	void *dynamic_buffer;
	unsigned int buffer_size;
	unsigned int element_count;
	size_t element_size;
};

/* Initializes an already-allocated array instance */
void array_init_with_element_size(array *a, size_t size);

/* Allocates and initializes an array instance */
array *array_alloc_with_element_size(size_t size);

/* Appends an element to an array instance */
void array_push(array *a, void *data);

/* Returns and removes the last object in the array */
void *array_pop(array *a);

/* Gets the number of elements in the array */
unsigned int array_length(array *a);

/* Gets the object at the specified index */
void *array_get(array *a, unsigned int index);

/* Removes the object at the specified index */
void array_remove_object_at_index(array *a, unsigned int index);

/* Cleans up an array instance initialized with array_init_with_element_size */
void array_deinit(array *a);
 
/* Cleans up an array instance initialized with array_alloc_with_element_size */
void array_free(array *a);

#endif
