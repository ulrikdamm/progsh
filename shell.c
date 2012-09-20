#include "shell.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>

void shell_run_command_with_pipe(shell *s, cmd *c, int read_pipe);

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
	shell_run_command_with_pipe(s, c, 0);
}

void shell_run_command_with_pipe(shell *s, cmd *c, int write_pipe) {
	int fds[2];
	int proc_pid;
	
	if (c->pipe_from != NULL) {
		pipe(fds);
	}
	
	if (!(proc_pid = fork())) {
		if (c->pipe_from != NULL) {
			close(fds[1]);
		}
		
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
		
		if (c->pipe_from) {
			close(fileno(stdin));
			dup(fds[0]);
		} else if (c->in) {
			close(fileno(stdin));
			dup(fileno(fopen(c->in, "r")));
		}
		
		if (write_pipe > 0) {
			close(fileno(stdout));
			dup(write_pipe);
		} else if (c->out) {
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
	
	if (write_pipe > 0) {
		close(write_pipe);
	}
	
	if (c->pipe_from != NULL) {
		close(fds[0]);
		shell_run_command_with_pipe(s, c->pipe_from, fds[1]);
	}
	
	int exit_code;
	waitpid(proc_pid, &exit_code, 0);
}
