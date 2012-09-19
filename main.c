#include "input.h"
#include "shell.h"
#include <stdio.h>

#define MAX_LINE_LENGTH 2048

int main(void) {
	shell *s = shell_alloc();
	
    while (1) {
        shell_print_prompt(s);
		char buffer[MAX_LINE_LENGTH];
        if (fgets(buffer, sizeof(buffer), stdin) < 0) {
            fprintf(stderr,"Error reading from input\n");
            return 1;
        }
		
        cmd *c = parse_input(buffer);
		shell_run_command(s, c);
		cmd_free(c);
    }
	
	shell_free(s);
	
    return 0;
}
