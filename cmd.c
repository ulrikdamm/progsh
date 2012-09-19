#include "cmd.h"
#include "util.h"
#include <stdlib.h>

cmd *cmd_alloc() {
	return salloc(sizeof(cmd));
}

void cmd_append_argument(cmd *c, char *argument) {
	if (c->command == NULL) {
		c->command = salloc(sizeof(cmd_arg));
		c->command->string = argument;
	} else {
		cmd_arg *arg_p = c->command;
		while (arg_p->next != NULL) {
			arg_p = arg_p->next;
		}
		
		arg_p->next = salloc(sizeof(cmd_arg));
		arg_p->next->string = argument;
	}
}

void cmd_free(cmd *c) {
	if (c == NULL) return;
	
	cmd_arg *arg_p = c->command;
	
	while (arg_p != NULL) {
		cmd_arg *next = arg_p->next;
		free(arg_p->string);
		free(arg_p);
		arg_p = next;
	}
	
	free(c->stdin);
	free(c->stdout);
	free(c->stderr);
	
	cmd_free(c->pipe_to);
}
