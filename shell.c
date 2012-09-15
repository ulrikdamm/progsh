#include "shell.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
	ERROR_NO_ERROR,
	ERROR_MEM,
	ERROR_INPUT,
	ERROR_COUNT,
} errors;
	
static const char *error_msg[] = {
	[ERROR_NO_ERROR] = "No error",
	[ERROR_MEM] = "Out of memory",
};

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
static shell_command *shell_interpret_command(shell *s, input_token *tokens, shell_error *error);

/* Allocates space for the second and copies nbytes bytes from the first string */
static shell_error copy_string(const char *from, char **to, int nbytes);

/* Appends a string as an argument to a command */
static shell_error shell_command_append_argument(shell_command *cmd, const char *string);

/* Frees a shell_command */
static void shell_command_free(shell_command *cmd);

/* Frees a shell_command_arg */
static void shell_command_arg_free(shell_command_arg *arg);

#pragma mark - Public functions

shell_error shell_run_command(shell *s, const char *cmd, ...) {
	va_list args;
	va_start(args, cmd);
	
	input_token *first_token = NULL;
	
	const char *str;
	input_error error;
	
	while ((str = va_arg(args, const char *)) != NULL) {
		input_token *token = input_token_append_token(first_token, str, &error);
		
		if (error) {
			input_token_free(first_token);
			return ERROR_INPUT;
		}
		
		if (first_token == NULL) {
			first_token = token;
		}
	}
	
	va_end(args);
	
	shell_run_with_input_tokens(s, first_token);
	input_token_free(first_token);
	
	return ERROR_NO_ERROR;
}

shell_error shell_run_with_input_tokens(shell *s, input_token *first_token) {
	shell_error error = ERROR_NO_ERROR;
	shell_command *cmd = shell_interpret_command(s, first_token, &error);
	
	if (error) {
		printf("%i\n", __LINE__); fflush(stdin);
		return error;
	}
	
	printf("command: '%s'\n", cmd->cmd);
	
	shell_command_arg *arg = cmd->args;
	while (arg != NULL) {
		printf("arg: '%s'\n", arg->arg);
		arg = arg->next;
	}
	
	printf(">%s '%s'\n", cmd->should_append? ">": "", cmd->out_stream);
	printf("< '%s'\n", cmd->in_stream);
	printf("2> '%s'\n", cmd->err_stream);
	printf("| %s\n", cmd->is_piping_output? cmd->out_pipe->cmd: "(none)");
	
	return ERROR_NO_ERROR;
}

shell_error shell_run_from_file(shell *s, FILE *f) {
	input_error error = ERROR_NO_ERROR;
	input_token *first_token = input_read_line(&error);
	
	if (error) {
		return ERROR_INPUT;
	}
	
	shell_run_with_input_tokens(s, first_token);
	input_token_free(first_token);
	
	return ERROR_NO_ERROR;
}

shell_error shell_run_from_stdin(shell *s) {
	return shell_run_from_file(s, stdin);
}

const char *shell_get_error(shell_error err) {
	if (err < 0 || err >= ERROR_COUNT) {
		return NULL;
	}
	
	return error_msg[err];
}

#pragma mark - Private functions

static shell_command *shell_interpret_command(shell *s, input_token *tokens, shell_error *error) {
	if (tokens == NULL) {
		printf("%i\n", __LINE__); fflush(stdin);
		return NULL;
	}
	
	shell_command *command = calloc(1, sizeof(shell_command));
	
	if (command == NULL) {
		printf("%i\n", __LINE__); fflush(stdin);
		*error = ERROR_MEM;
		return NULL;
	}
	
	shell_error strcpy_error = ERROR_NO_ERROR;
	if ((strcpy_error = copy_string(tokens->string, &command->cmd, strlen(tokens->string) + 1)) != ERROR_NO_ERROR) {
		*error = strcpy_error;
		shell_command_free(command);
		printf("%i\n", __LINE__); fflush(stdin);
		return NULL;
	}
	
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
					shell_error interpret_error = ERROR_NO_ERROR;
					command->out_pipe = shell_interpret_command(s, arg->next, &interpret_error);
					command->is_piping_output = 1;
					
					if (interpret_error != ERROR_NO_ERROR) {
						*error = interpret_error;
						shell_command_free(command);
						printf("%i\n", __LINE__); fflush(stdin);
						return NULL;
					}
					
					goto stop;
				}
					
				default: {
					shell_error append_error = ERROR_NO_ERROR;
					if ((append_error = shell_command_append_argument(command, string)) != ERROR_NO_ERROR) {
						shell_command_free(command);
						*error = append_error;
						printf("%i\n", __LINE__); fflush(stdin);
						return NULL;
					}
				}
			}
		}
		
		switch (operation_for_next) {
			case set_as_append_out_stream: command->should_append = 1;
			case set_as_out_stream: strcpy_error = copy_string(string, &command->out_stream, length + 1); break;
			case set_as_in_stream: strcpy_error = copy_string(string, &command->in_stream, length + 1); break;
			case set_as_err_stream: strcpy_error = copy_string(string, &command->err_stream, length + 1); break;
			default: break;
		}
		
		if (strcpy_error != ERROR_NO_ERROR) {
			shell_command_free(command);
			*error = strcpy_error;
			printf("%i\n", __LINE__); fflush(stdin);
			return NULL;
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

static shell_error shell_command_append_argument(shell_command *cmd, const char *string) {
	shell_command_arg *parent_arg = cmd->args;
	
	shell_command_arg *arg = malloc(sizeof(shell_command_arg));
	
	if (arg == NULL) {
		return ERROR_MEM;
	}
	
	arg->next = NULL;
	shell_error error = ERROR_NO_ERROR;
	if ((error = copy_string(string, &arg->arg, strlen(string) + 1)) != ERROR_NO_ERROR) {
		return error;
	}
	
	if (!parent_arg) {
		cmd->args = arg;
	} else {
		while (parent_arg->next != NULL) {
			parent_arg = parent_arg->next;
		}
		
		parent_arg->next = arg;
	}
	
	return ERROR_NO_ERROR;
}

static shell_error copy_string(const char *from, char **to, int nbytes) {
	*to = malloc(nbytes);
	
	if (*to == NULL) {
		return ERROR_MEM;
	}
	
	memcpy(*to, from, nbytes);
	
	return ERROR_NO_ERROR;
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
