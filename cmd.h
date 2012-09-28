#ifndef CMD_H
#define CMD_H

#include "cmd.h"

typedef struct cmd_arg_struct cmd_arg;
typedef struct cmd_struct cmd;

struct cmd_struct {
	cmd_arg *command;
	
	char *in;
	char *out;
	char *err;
	
	cmd *pipe_to;
	cmd *pipe_from;
	
	int should_pipe;
	int out_append;
	int background;
};

struct cmd_arg_struct {
	char *string;
	cmd_arg *next;
};

/* Allocates a new command instance. Should be free'd with cmd_free() */
cmd *cmd_alloc();

/* Appends an argument to the command.
 * The first argument is both the executable name and first argument. */
void cmd_append_argument(cmd *c, const char *argument);

/* Sets the in stream */
void cmd_set_in(cmd *c, const char *in);

/* Sets the out stream */
void cmd_set_out(cmd *c, const char *out);

/* Sets the error stream */
void cmd_set_err(cmd *c, const char *err);

/* Free's a command instance */
void cmd_free(cmd *c);

#endif
