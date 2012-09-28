#ifndef INPUT_H
#define INPUT_H

#include "cmd.h"

/* Parses a command line input to a command.
 * The command should be free'd with cmd_free().
 * error: 0 = no error, 1 = invalid input, 2 = empty input */
cmd *parse_input(const char *input, int *error);

#endif
