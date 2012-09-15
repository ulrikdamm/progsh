#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

typedef struct {
    char **tokens;
    int token_count;
} input_line;

/* Reads a line of whitespace-separated tokens from an input stream */
int input_read_line_from_stream(FILE *input_stream, input_line **line);

/* Reads a line of whitespace-separated tokens from the standard input */
int input_read_line(input_line **line);

/* Frees an input_line */
void input_line_free(input_line *line);

/* Gets the error message associated with an error number */
const char *input_get_error(int err);

#endif
