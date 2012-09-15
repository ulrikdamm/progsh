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
static input_token *input_token_append_token(input_token *parent_token, const char *string, int *error);

#pragma mark - Public functions

input_token *input_read_line_from_stream(FILE *in, int *error) {
    char line_buffer[2048];
    if (fgets(line_buffer, sizeof(line_buffer), in) != line_buffer) {
		*error = ERROR_READ;
		return NULL;
    }
	line_buffer[strlen(line_buffer) - 1] = 0; // remove newline character
	
	input_token *first_token = NULL;
	
	const char *separators = " \t";
	char *token_string = strtok(line_buffer, separators);
	
	while (token_string != NULL) {
		input_token *token = input_token_append_token(first_token, token_string, error);
		if (*error != ERROR_NO_ERROR) {
			input_token_free(token);
			return NULL;
		}
		
		if (!first_token) {
			first_token = token;
		}
		
		token_string = strtok(NULL, separators);
	}
	
	*error = ERROR_NO_ERROR;
	return first_token;
}

input_token *input_read_line(int *error) {
    return input_read_line_from_stream(stdin, error);
}

void input_token_free(input_token *token) {
	if (!token) {
		return;
	}
	
	input_token_free(token->next);
	free(token->string);
	free(token);
}

const char *input_get_error(int err) {
	if (err < 0 || err >= ERROR_COUNT) {
		return NULL;
	}
	
    return error_msg[err];
}

#pragma mark - Interval functions

static input_token *input_token_append_token(input_token *parent_token, const char *string, int *error) {
	size_t nbytes = strlen(string) + 1;
	
	char *token_str = malloc(nbytes);
	input_token *token = malloc(sizeof(input_token));
	
	if (!token_str || !token) {
		free(token_str);
		free(token);
		*error = ERROR_MEM;
		return NULL;
	}
	
	memcpy(token_str, string, nbytes);
	
	token->string = token_str;
	token->next = NULL;
	
	if (parent_token) {
		while (parent_token->next != NULL) {
			parent_token = parent_token->next;
		}
	
		parent_token->next = token;
	}
	
	*error = ERROR_NO_ERROR;
	return token;
}
