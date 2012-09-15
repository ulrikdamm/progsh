#include "shell.h"
#include "input.h"
#include <stdio.h>

int main(void) {
    int error;
    input_token *token = input_read_line(&error);
    if (error != 0) {
        fprintf(stderr, "input_read_line failed: %s\n", input_get_error(error));
        return 1;
    }
    
    input_token *t = token;
    while (t != NULL) {
        printf("token: %s\n", t->string);
        t = t->next;
    }
    
    input_token_free(token);
    return 0;
}
