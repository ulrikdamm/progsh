#include "input.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	input_line line;
	int allocated_token_count;
} input_line_int;

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

/* Ensures that there's enough free space in the token array for at least one more element */
static int input_line_reserve_space(input_line_int *line);

/* Adds a token to a line. Automatically expands available space */
static int input_line_append_token(input_line_int *line, const char *token);

#pragma mark - Public functions

int input_read_line_from_stream(FILE *in, input_line **line_out) {
    char line_buffer[2048];
    if (fgets(line_buffer, sizeof(line_buffer), in) != line_buffer) {
		return ERROR_READ;
    }
	line_buffer[strlen(line_buffer) - 1] = 0; // remove newline character
	
	input_line_int *line = calloc(1, sizeof(input_line_int));
	
	if (!line) {
		return ERROR_MEM;
	}
	
	const char *separators = " \t";
	char *token = strtok(line_buffer, separators);
	
	while (token != NULL) {
		int error;
		if ((error = input_line_append_token(line, token)) != ERROR_NO_ERROR) {
			input_line_free((input_line *)line);
			return error;
		}
		token = strtok(NULL, separators);
	}
	
	*line_out = (input_line *)line;
	return ERROR_NO_ERROR;
}

int input_read_line(input_line **line) {
    return input_read_line_from_stream(stdin, line);
}

void input_line_free(input_line *line) {
	int i;
	for (i = 0; i < line->token_count; i++) {
		free(line->tokens[i]);
	}
	
	free(line->tokens);
	free(line);
}

const char *input_get_error(int err) {
	if (err < 0 || err >= ERROR_COUNT) {
		return NULL;
	}
	
    return error_msg[err];
}

#pragma mark - Interval functions

static int input_line_append_token(input_line_int *line, const char *token) {
	size_t nbytes = strlen(token) + 1;
	char *line_str = malloc(nbytes);
	
	if (!line_str) {
		return ERROR_MEM;
	}
	
	memcpy(line_str, token, nbytes);
	
	int error;
	if ((error = input_line_reserve_space(line)) != ERROR_NO_ERROR) {
		free(line_str);
		return error;
	}
	
	((input_line *)line)->tokens[((input_line *)line)->token_count++] = line_str;
	
	return ERROR_NO_ERROR;
}

static int input_line_reserve_space(input_line_int *line) {
	if (line->allocated_token_count >= ((input_line *)line)->token_count + 1) {
		return ERROR_NO_ERROR;
	}
	
	if (line->allocated_token_count == 0) {
		line->allocated_token_count = 10;
	} else {
		line->allocated_token_count *= 2;
	}
	
	((input_line *)line)->tokens = realloc(((input_line *)line)->tokens, line->allocated_token_count * sizeof(const char *));
	
	if (((input_line *)line)->tokens == NULL) {
		return ERROR_MEM;
	}
	
	return ERROR_NO_ERROR;
}
