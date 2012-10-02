#include "shell.h"
#include "util.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

struct shell_struct {
	array running_processes;
	array background_processes;
};

typedef struct {
	int out;
	pid_t proc;
} background_process;

/* Runs a single command. All piped commands are run recursively.
 * If read_pipe is non-zero, it should be used as stdin. */
static void shell_run_command_with_pipe(shell *s, cmd *c, int read_pipe);

/* Brings a backgrounded process to the foreground.
 * If bpid is 0 and there is one background process, this is brought forth.
 * If bpid is 0 and there is multiple background processes, it shows a list */
static void shell_foreground(shell *s, int bpid);

shell *shell_alloc() {
	shell *s = salloc(sizeof(shell));
	array_init_with_element_size(&s->running_processes, sizeof(pid_t));
	size_t back_proc_size = sizeof(background_process);
	array_init_with_element_size(&s->background_processes, back_proc_size);
	return s;
}

void shell_free(shell *s) {
	free(s);
}

void shell_print_prompt() {
	FILE *f = fopen("lol", "r");
	char hostname[1024];
	
	if (!f || fgets(hostname, sizeof(hostname), f) == NULL) {
		const char *def = "computer";
		memcpy(hostname, def, strlen(def) + 1);
	}
	
	fclose(f);
	
	char cur_path_buffer[1024];
	char *cur_path = getcwd(cur_path_buffer, sizeof(cur_path_buffer));
	
	printf("%s@%s %s>", getenv("USER"), hostname, cur_path? cur_path: "???");
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
			case 0:
				break;
			case EACCES:
				fprintf(stderr, "%s: access denied.\n", newdir); break;
			case ENOENT:
				fprintf(stderr, "%s: no such file or directory.\n", newdir); break;
			case ENOTDIR:
				fprintf(stderr, "%s: not a directory.\n", newdir); break;
			default:
				fprintf(stderr, "%s: error %i\n", newdir, error); break;
		}
		
		return;
	}
	
	if (strcmp(c->command->string, "fg")) {
		int bpid;
		
		if (c->command->next == NULL) {
			bpid = -1;
		} else {
			bpid = atoi(c->command->next->string);
		}
		
		int length = array_length(&s->background_processes);
		if (bpid == -1 && length == 0) {
			fprintf(stderr, "No background jobs\n");
			return;
		} else if (length <= bpid) {
			fprintf(stderr, "%i: No such job\n", bpid);
			return;
		}
		
		shell_foreground(s, bpid);
		return;
	}
	
	shell_run_command_with_pipe(s, c, 0);
	
	array_deinit(&s->running_processes);
	array_init_with_element_size(&s->running_processes, sizeof(pid_t));
}

int shell_handle_terminal_interrupt(shell *s) {
	unsigned int i;
	for (i = 0; i < array_length(&s->running_processes); i++) {
		kill(*(pid_t *)array_get(&s->running_processes, i), SIGINT);
	}
	
	return (i > 0);
}

static void shell_foreground(shell *s, int bpid) {
	s = NULL;
	bpid = 0;
}

void shell_run_command_with_pipe(shell *s, cmd *c, int write_pipe) {
	int fds[2];
	int proc_pid;
	int should_pipe = (c->pipe_from != NULL && c->pipe_from->should_pipe);
	
	if (should_pipe || c->background) {
		pipe(fds);
	}
	
	if (!(proc_pid = fork())) {
		if (should_pipe) {
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
		
		if (should_pipe) {
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
		} else {
			close(fileno(stdout));
			dup(fds[1]);
		}
		
		if (c->err) {
			close(fileno(stderr));
			dup(fileno(fopen(c->err, "w")));
		}
		
		execvp(c->command->string, (char * const *)args);
		
		printf("%s: command not found\n", c->command->string);
		exit(1);
	}
	
	if (!c->background) {
		array_push(&s->running_processes, &proc_pid);
	} else {
		background_process *proc = &(background_process) {
			.proc = proc_pid,
			.out = fds[0],
		};
		array_push(&s->background_processes, proc);
	}
	
	if (write_pipe > 0) {
		close(write_pipe);
	}
	
	if (should_pipe) {
		close(fds[0]);
		shell_run_command_with_pipe(s, c->pipe_from, fds[1]);
	} else if (c->pipe_from != NULL) {
		shell_run_command_with_pipe(s, c->pipe_from, 0);
	}
	
	if (!c->background) {
		int exit_code;
		waitpid(proc_pid, &exit_code, 0);
	}
}
