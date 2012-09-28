#ifndef INPUT_H
#define INPUT_H

#include "cmd.h"

typedef enum {
	input_parse_error_none = 0,
	input_parse_error_syntax,
	input_parse_error_empty,
} input_parse_error;

/* Parses a command line input to a command.
 * The command should be free'd with cmd_free() */
cmd *parse_input(const char *input, input_parse_error *error);

#endif
