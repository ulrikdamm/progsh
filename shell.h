#ifndef SHELL_H
#define SHELL_H

#include "input.h"

typedef struct {
	const char *working_directory;
} shell;

typedef int shell_error;

/* Run a command with a number of string parameters */
shell_error shell_run_command(shell *s, const char *cmd, ...);

/* Run a command with parameters from user input */
shell_error shell_run_with_input_tokens(shell *s, input_token *first_token);

/* Read a command from an input file steam and run it */
shell_error shell_run_from_file(shell *s, FILE *f);

/* Read a command from stdin and run it */
shell_error shell_run_from_stdin(shell *s);

/* Gets the error message associated with an error number */
const char *shell_get_error(shell_error err);

#endif
