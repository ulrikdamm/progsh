#include "cmd.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

cmd *cmd_alloc() {
	return salloc(sizeof(cmd));
}

void cmd_append_argument(cmd *c, const char *argument) {
	size_t len = strlen(argument) + 1;
	
	if (c->command == NULL) {
		c->command = salloc(sizeof(cmd_arg));
		c->command->string = salloc(sizeof(char) * len);
		memcpy(c->command->string, argument, len);
	} else {
		cmd_arg *arg_p = c->command;
		while (arg_p->next != NULL) {
			arg_p = arg_p->next;
		}
		
		arg_p->next = salloc(sizeof(cmd_arg));
		arg_p->next->string = salloc(sizeof(char) * len);
		memcpy(arg_p->next->string, argument, len);
	}
}

void cmd_set_in(cmd *c, const char *in) {
	size_t len = strlen(in) + 1;
	free(c->in);
	c->in = salloc(len);
	memcpy(c->in, in, len);
}

void cmd_set_out(cmd *c, const char *out) {
	size_t len = strlen(out) + 1;
	free(c->out);
	c->out = salloc(len);
	memcpy(c->out, out, len);
}

void cmd_set_err(cmd *c, const char *err) {
	size_t len = strlen(err) + 1;
	free(c->err);
	c->err = salloc(len);
	memcpy(c->err, err, len);
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
	
	free(c->in);
	free(c->out);
	free(c->err);
	
	cmd_free(c->pipe_to);
}
