#include "shell.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

struct shell_struct {
	pid_t running_processes[SHELL_MAX_PROCESSES];
};

/* Runs a single command. All piped commands are run recursively.
 * If read_pipe is non-zero, it should be used as stdin. */
static void shell_run_command_with_pipe(shell *s, cmd *c, int read_pipe);

/* Appends a process id to the shells list of running processes. */
static int shell_append_pid(shell *s, pid_t proc);

shell *shell_alloc() {
	shell *s = salloc(sizeof(shell));
	return s;
}

void shell_free(shell *s) {
	free(s);
}

void shell_print_prompt(shell *s) {
	s = NULL;
	
	char cur_path_buffer[1024];
	char *cur_path = getcwd(cur_path_buffer, sizeof(cur_path_buffer));
	
	printf("%s@computer %s>", getenv("USER"), cur_path? cur_path: "???");
}

void shell_run_command(shell *s, cmd *c) {
	if (strcmp(c->command->string, "exit") == 0) {
		exit(0);
	}
	
	if (strcmp(c->command->string, "(╯°□°）╯︵┻━┻") == 0) {
		printf("┬─┬ ノ( ゜-゜ノ)\n");
		return;
	}
	
	if (strcmp(c->command->string, "cd") == 0 && c->command->next != NULL) {
		const char *newdir = c->command->next->string;
		int error = chdir(newdir);
		
		switch (error) {
			case 0: break;
			case EACCES: printf("%s: access denied.\n", newdir); break;
			case ENOENT: printf("%s: no such file or directory.\n", newdir); break;
			case ENOTDIR: printf("%s: not a directory.\n", newdir); break;
			default: printf("%s: error %i\n", newdir, error); break;
		}
		return;
	}
	
	shell_run_command_with_pipe(s, c, 0);
	
	memset(s->running_processes, 0, sizeof(pid_t) * SHELL_MAX_PROCESSES);
}

int shell_handle_terminal_interrupt(shell *s) {
	int i;
	pid_t *process;
	for (i = 0; *(process = &s->running_processes[i]) > 0 && i < SHELL_MAX_PROCESSES; i++) {
		kill(*process, SIGINT);
		*process = 0;
	}
	
	return (i > 0);
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
	
	if (shell_append_pid(s, proc_pid) > 0) {
		kill(proc_pid, SIGINT);
		return;
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

int shell_append_pid(shell *s, pid_t proc) {
	int i = 0;
	pid_t *process;
	while (*(process = &(s->running_processes[i])) > 0 && i < SHELL_MAX_PROCESSES) i++;
	
	if (i == SHELL_MAX_PROCESSES) {
		fprintf(stderr, "Error: too many processes\n");
		shell_handle_terminal_interrupt(s);
		return 1;
	} else {
		*process = proc;
	}
	
	return 0;
}
