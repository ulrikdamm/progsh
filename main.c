#include "input.h"
#include "shell.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdlib.h>
#include <readline/readline.h>

#ifdef __linux
#include <wait.h>
#endif

#define MAX_LINE_LENGTH 2048

/* SIGINT signal handler */
static void int_handler(int sig);

/* SIGCHLD signal handler */
static void child_handler(int sig);

shell *s;
static jmp_buf buf;

int main(void) {
	signal(SIGINT, int_handler);
	signal(SIGCHLD, child_handler);
	
	s = shell_alloc();
	
	char *input = NULL;
	cmd *cur_cmd = NULL;
	
    while (1) {
		setjmp(buf);
		
		if (input != NULL) free(input), input = NULL;
		if (cur_cmd != NULL) cmd_free(cur_cmd), cur_cmd = NULL;
		
		char buffer[512];
		shell_get_prompt(buffer, sizeof(buffer));
		input = readline(buffer);
		
		if (!input) {
			printf("Input error\n");
			continue;
		}
		
		input_parse_error error;
		cur_cmd = parse_input(input, &error);
		free(input), input = NULL;
		
		if (error == 2) {
			exit(0);
		}
		
		while (cur_cmd->pipe_to != NULL) {
			cur_cmd = cur_cmd->pipe_to;
		}
		shell_run_command(s, cur_cmd);
		cmd_free(cur_cmd), cur_cmd = NULL;
    }
	
	shell_free(s);
	
    return 0;
}

void int_handler(int sig) {
	sig = 0;
	
	if (!shell_handle_terminal_interrupt(s)) {
		printf("\n");
		longjmp(buf, 0);
	}
}

static void child_handler(int sig) {
	sig = 0;
	
	int exit_code;
	while (waitpid(-1, &exit_code, WNOHANG) > 0);
}
