#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

typedef struct input_token_struct {
    char *string;
    struct input_token_struct *next;
} input_token;

/* Reads a line of whitespace-separated tokens from an input stream */
input_token *input_read_line_from_stream(FILE *input_stream);

/* Reads a line of whitespace-separated tokens from the standard input */
input_token *input_read_line(void);

/* Frees an input_line */
void input_token_free(input_token *token);

/* Adds a token to a line. Automatically expands available space. Parent token may be NULL */
input_token *input_token_append_token(input_token *parent_token, const char *string);

#endif
