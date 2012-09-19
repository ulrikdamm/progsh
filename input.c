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

static int is_whitespace(char c);
static void ignore_whitespace(parser *p);
static char peek(parser *p);
static char peekn(parser *p, int n);
static char get_char(parser *p);
static string_ref get_string(parser *p);

cmd *parse_input(const char *input) {
	parser *p = &(parser){ .source = input, .source_length = strlen(input), .source_counter = 0 };
	
	cmd *initial_cmd = cmd_alloc();
	cmd *cur_cmd = initial_cmd;
	
	while (1) {
		ignore_whitespace(p);
		char c = peek(p);
	
		if (c == EOF) {
			break;
		}
		
		if (c == '|') {
			get_char(p);
			cmd *new_cmd = cmd_alloc();
			cur_cmd->pipe_to = new_cmd;
			new_cmd->pipe_from = cur_cmd;
			cur_cmd = new_cmd;
			continue;
		}
		
		enum { parse_keyword, parse_stdin, parse_stdout, parse_stdout_append, parse_stderr} action;
		
		if (c == '>'
			&& peekn(p, 1) == '>')	{ action = parse_stdout_append; get_char(p); get_char(p); }
		else if (c == '>')			{ action = parse_stdout; get_char(p); }
		else if (c == '<')			{ action = parse_stdin; get_char(p); }
		else if (c == '2'
			&& peekn(p, 1) == '>')	{ action = parse_stderr; get_char(p); get_char(p); }
		else						{ action = parse_keyword; }
		
		ignore_whitespace(p);
		
		string_ref word = get_string(p);
		char *string = salloc(word.length + 1);
		memcpy(string, p->source + word.start, word.length);
		
		switch (action) {
			case parse_keyword: cmd_append_argument(cur_cmd, string); break;
			case parse_stdin: free(cur_cmd->in); cur_cmd->in = string; break;
			case parse_stdout_append: cur_cmd->out_append = 1;
			case parse_stdout: free(cur_cmd->out); cur_cmd->out = string; break;
			case parse_stderr: free(cur_cmd->err); cur_cmd->err = string; break;
		}
	}
	
	/*while (cur_cmd) {
		printf("[");
		cmd_arg *arg = cur_cmd->command;
		while (arg) {
			printf("'%s' ", arg->string);
			arg = arg->next;
		}
		printf(">%s'%s' <'%s' 2>'%s']\n", cur_cmd->out_append? ">": "", cur_cmd->out, cur_cmd->in, cur_cmd->err);
		cur_cmd = cur_cmd->pipe_from;
	}*/
	
	return initial_cmd;
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
	return (p->source_counter + n >= p->source_length? EOF: p->source[p->source_counter + n]);
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
	while (is_string? c = get_char(p), c != '"' && c != EOF: !is_whitespace(get_char(p)));
	
	str.length = p->source_counter - 1 - str.start;
	
	return str;
}