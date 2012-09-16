#include "shell.h"
#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>

struct shell_command_struct;
struct shell_command_arg_struct;

typedef struct shell_command_struct {
	char *cmd;
	struct shell_command_arg_struct *args;
	struct shell_command_struct *out_pipe;
	
	int is_piping_output;
	int should_append;
	
	char *out_stream;
	char *in_stream;
	char *err_stream;
} shell_command;

typedef struct shell_command_arg_struct {
	char *arg;
	struct shell_command_arg_struct *next;
} shell_command_arg;

/* Interprets a command from an input line */
static shell_command *shell_interpret_command(shell *s, input_token *tokens);

/* Allocates space for the second and copies nbytes bytes from the first string */
static void copy_string(const char *from, char **to, int nbytes);

/* Appends a string as an argument to a command */
static void shell_command_append_argument(shell_command *cmd, const char *string);

/* Frees a shell_command */
static void shell_command_free(shell_command *cmd);

/* Frees a shell_command_arg */
static void shell_command_arg_free(shell_command_arg *arg);

/* Call when there's an error in the interpretation */
static void shell_interpreter_error(const char *error);

/* Run a command in a shell */
static void run_command(shell *s, shell_command *cmd);

#pragma mark - Public functions

void shell_run_command(shell *s, const char *cmd, ...) {
	va_list args;
	va_start(args, cmd);
	
	input_token *first_token = NULL;
	
	const char *str;
	
	while ((str = va_arg(args, const char *)) != NULL) {
		input_token *token = input_token_append_token(first_token, str);
		
		if (first_token == NULL) {
			first_token = token;
		}
	}
	
	va_end(args);
	
	shell_run_with_input_tokens(s, first_token);
	input_token_free(first_token);
}

void shell_run_with_input_tokens(shell *s, input_token *first_token) {
	shell_command *cmd = shell_interpret_command(s, first_token);
	
	run_command(s, cmd);
}

void shell_run_from_file(shell *s, FILE *f) {
	input_token *first_token = input_read_line();
	
	if (!first_token) {
		return;
	}
	
	shell_run_with_input_tokens(s, first_token);
	input_token_free(first_token);
}

void shell_run_from_stdin(shell *s) {
	return shell_run_from_file(s, stdin);
}

#pragma mark - Private functions

static void run_command(shell *s, shell_command *cmd) {
	int param_count = 1;
	
	shell_command_arg *arg = cmd->args;
	while (arg != NULL) {
		param_count++;
		arg = arg->next;
	}
	
	char *args[param_count];
	args[param_count - 1] = NULL;
	
	arg = cmd->args;
	int i = 0;
	while (arg != NULL) {
		args[i] = arg->arg;
		i++;
		arg = arg->next;
	}
	
	pid_t child = fork();
	
	if (child == -1) {
		printf("fork error\n");
		return;
	}
	
	if (!child) {
		if (cmd->out_stream != NULL) {
			close(fileno(stdout));
			dup(fileno(fopen(cmd->out_stream, (cmd->should_append? "a": "w"))));
		}
		
		if (cmd->in_stream != NULL) {
			close(fileno(stdin));
			dup(fileno(fopen(cmd->out_stream, "r")));
		}
		
		execvp(cmd->cmd, (char * const *)args);
		
		printf("error: exec\n");
		exit(1);
	}
	
	int status;
	waitpid(child, &status, 0);
	
	printf("Program '%s' exited with code %i\n", cmd->cmd, status);
}

static shell_command *shell_interpret_command(shell *s, input_token *tokens) {
	if (tokens == NULL) {
		return NULL;
	}
	
	shell_command *command = salloc(sizeof(shell_command));
	
	copy_string(tokens->string, &command->cmd, strlen(tokens->string) + 1);
	shell_command_append_argument(command, command->cmd);
	
	enum {
		no_operation,
		set_as_out_stream,
		set_as_in_stream,
		set_as_err_stream,
		set_as_append_out_stream,
	} operation_for_next = no_operation;
	
	input_token *arg = tokens->next;
	while (arg != NULL) {
		size_t length = strlen(arg->string);
		if (length == 0) continue;
		
		const char *string = arg->string;
		
		if (operation_for_next == no_operation) {
			switch (arg->string[0]) {
				case '<': {
					operation_for_next = set_as_in_stream;
					string++;
					if (length == 1) goto next_arg;
					break;
				}
				
				case '>': {
					command->should_append = (length >= 2 && arg->string[1] == '>');
					operation_for_next = set_as_out_stream;
					string += (command->should_append? 2: 1);
					if (length == (command->should_append? 2: 1)) goto next_arg;
					break;
				}
				
				case '2': {
					if (length < 2 || arg->string[1] != '>') break;
					operation_for_next = set_as_err_stream;
					string += 2;
					if (length == 2) goto next_arg;
					break;
				}
				
				case '|': {
					command->out_pipe = shell_interpret_command(s, arg->next);
					command->is_piping_output = 1;
					
					goto stop;
				}
					
				default: {
					shell_command_append_argument(command, string);
				}
			}
		}
		
		switch (operation_for_next) {
			case set_as_append_out_stream: command->should_append = 1;
			case set_as_out_stream: copy_string(string, &command->out_stream, length + 1); break;
			case set_as_in_stream: copy_string(string, &command->in_stream, length + 1); break;
			case set_as_err_stream: copy_string(string, &command->err_stream, length + 1); break;
			default: break;
		}
			
		operation_for_next = no_operation;
		
	next_arg:
		arg = arg->next;
		continue;
		
	stop:
		break;
	}
	
	return command;
}

static void shell_command_append_argument(shell_command *cmd, const char *string) {
	shell_command_arg *parent_arg = cmd->args;
	
	shell_command_arg *arg = salloc(sizeof(shell_command_arg));
	arg->next = NULL;
	copy_string(string, &arg->arg, strlen(string) + 1);
	
	if (!parent_arg) {
		cmd->args = arg;
	} else {
		while (parent_arg->next != NULL) {
			parent_arg = parent_arg->next;
		}
		
		parent_arg->next = arg;
	}
}

static void copy_string(const char *from, char **to, int nbytes) {
	*to = salloc(nbytes);
	memcpy(*to, from, nbytes);
}

static void shell_command_free(shell_command *cmd) {
	if (cmd == NULL) return;
	
	shell_command_arg_free(cmd->args);
	
	free(cmd->cmd);
	free(cmd->in_stream);
	free(cmd->err_stream);
	free(cmd->out_stream);
}

static void shell_command_arg_free(shell_command_arg *arg) {
	if (arg == NULL) return;
	
	shell_command_arg_free(arg->next);
	free(arg->arg);
	free(arg);
}
