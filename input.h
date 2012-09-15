#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

typedef struct input_token_struct {
    char *string;
    struct input_token_struct *next;
} input_token;

typedef int input_error;

/* Reads a line of whitespace-separated tokens from an input stream */
input_token *input_read_line_from_stream(FILE *input_stream, input_error *error);

/* Reads a line of whitespace-separated tokens from the standard input */
input_token *input_read_line(input_error *error);

/* Frees an input_line */
void input_token_free(input_token *token);

/* Adds a token to a line. Automatically expands available space. Parent token may be NULL */
input_token *input_token_append_token(input_token *parent_token, const char *string, input_error *error);

/* Gets the error message associated with an error number */
const char *input_get_error(input_error err);

#endif
