#include "input.h"
#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#pragma mark - Public functions

input_token *input_read_line_from_stream(FILE *in) {
    char line_buffer[2048];
    if (fgets(line_buffer, sizeof(line_buffer), in) != line_buffer) {
		return NULL;
    }
	line_buffer[strlen(line_buffer) - 1] = 0; // remove newline character
	
	input_token *first_token = NULL;
	
	const char *separators = " \t";
	char *token_string = strtok(line_buffer, separators);
	
	while (token_string != NULL) {
		input_token *token = input_token_append_token(first_token, token_string);
		
		if (!first_token) {
			first_token = token;
		}
		
		token_string = strtok(NULL, separators);
	}
	
	return first_token;
}

input_token *input_read_line(void) {
    return input_read_line_from_stream(stdin);
}

void input_token_free(input_token *token) {
	if (!token) {
		return;
	}
	
	input_token_free(token->next);
	free(token->string);
	free(token);
}

input_token *input_token_append_token(input_token *parent_token, const char *string) {
	size_t nbytes = strlen(string) + 1;
	
	char *token_str = salloc(nbytes);
	input_token *token = salloc(sizeof(input_token));
	
	memcpy(token_str, string, nbytes);
	
	token->string = token_str;
	token->next = NULL;
	
	if (parent_token) {
		while (parent_token->next != NULL) {
			parent_token = parent_token->next;
		}
	
		parent_token->next = token;
	}
	
	return token;
}
