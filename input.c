#include "input.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ERROR_NO_ERROR,
	ERROR_READ,
	ERROR_MEM,
	ERROR_COUNT,
} errors;

static const char *error_msg[] = {
	[ERROR_NO_ERROR] = "No error",
	[ERROR_READ] = "Error reading from input stream",
	[ERROR_MEM] = "Out of memory",
};

/* Adds a token to a line. Automatically expands available space */
static input_line *input_line_append_token(input_line *line, const char *token, int *error);

#pragma mark - Public functions

input_line *input_read_line_from_stream(FILE *in, int *error) {
    char line_buffer[2048];
    if (fgets(line_buffer, sizeof(line_buffer), in) != line_buffer) {
		*error = ERROR_READ;
		return NULL;
    }
	line_buffer[strlen(line_buffer) - 1] = 0; // remove newline character
	
	input_line *first_line = NULL;
	
	const char *separators = " \t";
	char *token = strtok(line_buffer, separators);
	
	while (token != NULL) {
		input_line *line = input_line_append_token(first_line, token, error);
		if (*error != ERROR_NO_ERROR) {
			input_line_free(line);
			return NULL;
		}
		
		if (!first_line) {
			first_line = line;
		}
		
		token = strtok(NULL, separators);
	}
	
	*error = ERROR_NO_ERROR;
	return first_line;
}

input_line *input_read_line(int *error) {
    return input_read_line_from_stream(stdin, error);
}

void input_line_free(input_line *line) {
	if (!line) {
		return;
	}
	
	input_line_free(line->next);
	free(line->token);
	free(line);
}

const char *input_get_error(int err) {
	if (err < 0 || err >= ERROR_COUNT) {
		return NULL;
	}
	
    return error_msg[err];
}

#pragma mark - Interval functions

static input_line *input_line_append_token(input_line *parent_line, const char *token, int *error) {
	size_t nbytes = strlen(token) + 1;
	
	char *line_str = malloc(nbytes);
	input_line *line = malloc(sizeof(input_line));
	
	if (!line_str || !line) {
		free(line_str);
		free(line);
		*error = ERROR_MEM;
		return NULL;
	}
	
	memcpy(line_str, token, nbytes);
	
	line->token = line_str;
	line->next = NULL;
	
	if (parent_line) {
		while (parent_line->next != NULL) {
			parent_line = parent_line->next;
		}
	
		parent_line->next = line;
	}
	
	*error = ERROR_NO_ERROR;
	return line;
}
