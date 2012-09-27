#ifndef INPUT_H
#define INPUT_H

#include "cmd.h"

/* Parses a command line input to a command.
 * The command should be free'd with cmd_free() */
cmd *parse_input(const char *input);

#endif
