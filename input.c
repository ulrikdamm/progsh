#include "input.h"
#include "util.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define MAX_STRING_LENGTH 1024

typedef struct parser_struct parser;
typedef struct string_ref_struct string_ref;

struct parser_struct {
	const char *source;
	int source_length;
	int source_counter;
};

struct string_ref_struct {
	int start;
	int length;
};

/* Checks if next character in input is whitespace */
static int is_whitespace(char c);

/* Skips characters until next non-whitespace */
static void ignore_whitespace(parser *p);

/* Peeks at the next character in input */
static char peek(parser *p);

/* Peeks n characters forward in input */
static char peekn(parser *p, int n);

/* Gets the next character in input and moves input pointer */
static char get_char(parser *p);

/* Read a string or keyword from input.
 * If the next character is ", it will read until the next " character.
 * Otherwise, it will read until next whitespace character */
static string_ref get_string(parser *p);
 
/* Reads any redirection (>, <, 2>, >>) and changes cur_cmd accordingly.
 * Returns 1 if next token was a redirection, 0 otherwise */
static int parse_redirection(cmd *cur_cmd, parser *p);

/* Reads a keyword and changes cur_cmd accordingly */
static void parse_keyword(cmd *cur_cmd, parser *p);

cmd *parse_input(const char *input, input_parse_error *error) {
	parser *p = &(parser){
		.source = input,
		.source_length = strlen(input),
		.source_counter = 0,
	};
	
	*error = input_parse_error_none;
		
	cmd *initial_cmd = cmd_alloc();
	cmd *cur_cmd = initial_cmd;
	
	while (1) {
		ignore_whitespace(p);
		char c = peek(p);
	
		if (c == EOF) {
			if (cur_cmd->command == NULL) {
				if (cur_cmd->pipe_from != NULL) {
					cur_cmd->pipe_from->pipe_to = NULL;
				}
				
				cmd_free(cur_cmd);
			}
			
			break;
		}
		
		if (c == '|') {
			if (cur_cmd->command == NULL) {
				fprintf(stderr, "Error: expected command name\n");
				*error = input_parse_error_syntax;
				cmd_free(initial_cmd);
			}
			
			get_char(p);
			cmd *new_cmd = cmd_alloc();
			cur_cmd->pipe_to = new_cmd;
			cur_cmd->should_pipe = 1;
			new_cmd->pipe_from = cur_cmd;
			cur_cmd = new_cmd;
			continue;
		}
		
		if (c == '&') {
			if (cur_cmd->command == NULL) {
				fprintf(stderr, "Error: expected command name\n");
				*error = input_parse_error_syntax;
				cmd_free(initial_cmd);
			}
			
			get_char(p);
			cmd *new_cmd = cmd_alloc();
			cur_cmd->pipe_to = new_cmd;
			cur_cmd->should_pipe = 0;
			cur_cmd->background = 1;
			new_cmd->pipe_from = cur_cmd;
			cur_cmd = new_cmd;
			continue;
		}
		
		if (!parse_redirection(cur_cmd, p)) {
			parse_keyword(cur_cmd, p);
		}
	}
	
	return initial_cmd;
}

static int parse_redirection(cmd *cur_cmd, parser *p) {
	enum {
		parse_stdin,
		parse_stdout,
		parse_stdout_append,
		parse_stderr,
	} action;
	
	char c = peek(p);
		
	if (c == '>' && peekn(p, 1) == '>') {
		action = parse_stdout_append; get_char(p); get_char(p);
	} else if (c == '>') {
		action = parse_stdout; get_char(p);
	} else if (c == '<') {
		action = parse_stdin; get_char(p);
	} else if (c == '2' && peekn(p, 1) == '>') {
		action = parse_stderr; get_char(p); get_char(p);
	} else {
		return 0;
	}
		
	ignore_whitespace(p);
		
	string_ref word = get_string(p);
	char string[word.length + 1];
	string[word.length] = 0;
	memcpy(string, p->source + word.start, word.length);
		
	switch (action) {
		case parse_stdin:	cmd_set_in(cur_cmd, string); break;
		case parse_stdout_append: cur_cmd->out_append = 1; /* no break */
		case parse_stdout:	cmd_set_out(cur_cmd, string); break;
		case parse_stderr:	cmd_set_err(cur_cmd, string); break;
	}
	
	return 1;
}

static void parse_keyword(cmd *cur_cmd, parser *p) {
	string_ref word = get_string(p);
	char string[word.length + 1];
	string[word.length] = 0;
	memcpy(string, p->source + word.start, word.length);
	
	cmd_append_argument(cur_cmd, string);
}

static int is_whitespace(char c) {
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void ignore_whitespace(parser *p) {
	while (is_whitespace(peek(p))) {
		p->source_counter++;
	}
}

static char peek(parser *p) {
	return peekn(p, 0);
}

static char peekn(parser *p, int n) {
	if (p->source_counter + n >= p->source_length) {
		return EOF;
	}
	
	return p->source[p->source_counter + n];
}

static char get_char(parser *p) {
	char c = peek(p);
	
	if (c != EOF) p->source_counter++;
	
	return c;
}

static string_ref get_string(parser *p) {
	ignore_whitespace(p);
	
	string_ref str = {};
	
	int is_string = (get_char(p) == '"');
	
	if (is_string) {
		str.start = p->source_counter;
	} else {
		str.start = p->source_counter - 1;
	}
	
	char c;
	while (is_string?
		c = get_char(p), c != '"' && c != EOF:
		!is_whitespace(get_char(p)));
	
	str.length = p->source_counter - 1 - str.start;
	
	return str;
}
