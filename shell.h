#ifndef SHELL_H
#define SHELL_H

#include "cmd.h"

typedef struct shell_struct shell;

struct shell_struct {
	char *path;
};

shell *shell_alloc();
void shell_print_prompt(shell *s);
void shell_run_command(shell *s, cmd *c);
void shell_free(shell *s);

#endif
