#ifndef CODE_REQUIRED_EXCEPTION_H
#define CODE_REQUIRED_EXCEPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* message;
} CodeRequiredException;

CodeRequiredException* create_code_required_exception() {
    CodeRequiredException* exception = malloc(sizeof(CodeRequiredException));
    exception->message = strdup("Code not provided");
    return exception;
}

void free_code_required_exception(CodeRequiredException* exception) {
    if (exception) {
        free(exception->message);
        free(exception);
    }
}

#endif // CODE_REQUIRED_EXCEPTION_H