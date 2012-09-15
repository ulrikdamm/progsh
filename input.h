#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

typedef struct input_token_struct {
    char *string;
    struct input_token_struct *next;
} input_token;

/* Reads a line of whitespace-separated tokens from an input stream */
input_token *input_read_line_from_stream(FILE *input_stream, int *error);

/* Reads a line of whitespace-separated tokens from the standard input */
input_token *input_read_line(int *error);

/* Frees an input_line */
void input_token_free(input_token *token);

/* Gets the error message associated with an error number */
const char *input_get_error(int err);

#endif
