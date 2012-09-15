#ifndef SHELL_H
#define SHELL_H

#include "input.h"

/* Run a command with a number of string parameters */
int shell_run_command(const char *cmd, ...);

/* Run a command with parameters from user input */
int shell_run_from_input(input_token *first_token);

#endif
