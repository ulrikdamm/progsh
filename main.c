#include "input.h"
#include "shell.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>

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
	
    while (1) {
		setjmp(buf);
		
        shell_print_prompt();
		char buffer[MAX_LINE_LENGTH];
        if (fgets(buffer, sizeof(buffer), stdin) < 0) {
            fprintf(stderr,"Error reading from input\n");
            return 1;
        }
		
		input_parse_error error;
		cmd *c = parse_input(buffer, &error);
		
		if (error == 2) {
			exit(0);
		}
		
		while (c->pipe_to != NULL) {
			c = c->pipe_to;
		}
		shell_run_command(s, c);
		cmd_free(c);
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
