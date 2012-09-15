#include "shell.h"
#include "input.h"
#include <stdio.h>

int main(void) {
    int error;
    input_line *line = input_read_line(&error);
    if (error != 0) {
        fprintf(stderr, "input_read_line failed: %s\n", input_get_error(error));
        return 1;
    }
    
    input_line *l = line;
    while (l != NULL) {
        printf("token: %s\n", l->token);
        l = l->next;
    }
    
    input_line_free(line);
    return 0;
}
