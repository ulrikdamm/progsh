#include "shell.h"
#include "input.h"
#include <stdio.h>

int main(void) {
    input_token *token = input_read_line();
    if (token == NULL) {
        fprintf(stderr, "input_read_line failed\n");
        return 2;
    }
    
	shell_run_with_input_tokens(NULL, token);
    
    input_token_free(token);
    return 0;
}
