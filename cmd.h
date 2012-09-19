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
	
	int out_append;
};

struct cmd_arg_struct {
	char *string;
	cmd_arg *next;
};

cmd *cmd_alloc();
void cmd_append_argument(cmd *c, char *argument);
void cmd_free(cmd *c);

#endif
