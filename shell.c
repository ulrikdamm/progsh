#include "shell.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>

shell *shell_alloc() {
	shell *s = salloc(sizeof(shell));
	s->path = "~/";
	return s;
}

void shell_free(shell *s) {
	free(s);
}

void shell_print_prompt(shell *s) {
	s = NULL;
	printf("%s@computer %s>", getenv("USER"), getenv("PWD"));
}

void shell_run_command(shell *s, cmd *c) {
	s = NULL;
	
	if (!fork()) {
		cmd_arg *args_p = c->command;
		int args_count = 0;
		while (args_p != NULL) {
			args_count++;
			args_p = args_p->next;
		}
		
		char **args = salloc(sizeof(const char *) * (args_count + 1));
		
		args_p = c->command;
		int i = 0;
		while (args_p != NULL) {
			args[i] = args_p->string;
			i++;
			args_p = args_p->next;
		}
		args[i] = NULL;
		
		if (c->in) {
			close(fileno(stdin));
			dup(fileno(fopen(c->in, "r")));
		}
		
		if (c->out) {
			close(fileno(stdout));
			dup(fileno(fopen(c->out, (c->out_append? "a": "w"))));
		}
		
		if (c->err) {
			close(fileno(stderr));
			dup(fileno(fopen(c->err, "w")));
		}
		
		execvp(c->command->string, (char * const *)args);
		
		printf("%s: command not found\n", c->command->string);
		free(args);
	}
	
	wait(NULL);
}
