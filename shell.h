#ifndef SHELL_H
#define SHELL_H

#include "cmd.h"
#include <unistd.h>

typedef struct shell_struct shell;

/* Allocates a new shell instance. Should be free'd with shell_free() */
shell *shell_alloc();

/* Prints the prompt before user input */
void shell_print_prompt(shell *s);

/* Runs a command in a shell instance.
 * This function will wait for the process to finish */
void shell_run_command(shell *s, cmd *c);

/* Interrupt the shell with a terminal, force quitting all running processes */
int shell_handle_terminal_interrupt(shell *s);

/* Free a shell from memory */
void shell_free(shell *s);

#endif
