#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

typedef struct input_line_struct {
    char *token;
    struct input_line_struct *next;
} input_line;

/* Reads a line of whitespace-separated tokens from an input stream */
input_line *input_read_line_from_stream(FILE *input_stream, int *error);

/* Reads a line of whitespace-separated tokens from the standard input */
input_line *input_read_line(int *error);

/* Frees an input_line */
void input_line_free(input_line *line);

/* Gets the error message associated with an error number */
const char *input_get_error(int err);

#endif
