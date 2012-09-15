#include "shell.h"
#include "input.h"
#include <stdio.h>

int main(void) {
	input_error ierr;
    input_token *token = input_read_line(&ierr);
    if (ierr != 0) {
        fprintf(stderr, "input_read_line failed: %s\n", input_get_error(ierr));
        return 1;
    }
    
	shell_error serr;
	if ((serr = shell_run_with_input_tokens(NULL, token)) != 0) {
		fprintf(stderr, "shell_run_command_from_input failed: %i %s\n", serr, shell_get_error(serr));
		return 2;
	}
    
    input_token_free(token);
    return 0;
}
