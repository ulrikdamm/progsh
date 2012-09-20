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

void shell_run_command_with_pipe(shell *s, cmd *c, int read_pipe) {
	int fds[2];
	int proc_pid;
	
	if (c->pipe_to != NULL) {
		pipe(fds);
	}
	
	if (!(proc_pid = fork())) {
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
		
		if (read_pipe > 0) {
			close(fileno(stdin));
			dup(read_pipe);
		} else if (c->in) {
			close(fileno(stdin));
			dup(fileno(fopen(c->in, "r")));
		}
		
		if (c->pipe_to) {
			close(fileno(stdout));
			dup(fds[1]);
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
	
	if (c->pipe_to != NULL) {
		shell_run_command_with_pipe(s, c->pipe_to, fds[0]);
		
		close(fds[0]);
		close(fds[1]);
	}
	
	int exit_code;
	waitpid(proc_pid, &exit_code, 0);
	
	printf("Process %s ended with exit code %i\n", c->command->string, exit_code);
}
