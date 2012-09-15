#include "shell.h"
#include "input.h"
#include <stdio.h>

int main(void) {
    input_line *line;
    int status;
    if ((status = input_read_line(&line)) != 0) {
        fprintf(stderr, "input_read_line failed: %s\n", input_get_error(status));
        return 1;
    }
    
    int i;
    for (i = 0; i < line->token_count; i++) {
        printf("token: %s\n", line->tokens[i]);
    }
    
    input_line_free(line);
    return 0;
}
