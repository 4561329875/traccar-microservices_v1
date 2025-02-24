#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>

// HTTP status codes
#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_INTERNAL_ERROR 500

// Maximum length for error messages
#define MAX_ERROR_LENGTH 4096

// Response structure
typedef struct {
    int status_code;
    char* body;
} Response;

// Exception types
typedef enum {
    GENERIC_EXCEPTION,
    WEB_APPLICATION_EXCEPTION
} ExceptionType;

// Exception structure
typedef struct {
    ExceptionType type;
    int status_code;
    char* message;
    char* stack_trace;
} Exception;

// Function to generate stack trace
char* generate_exception_stack(const Exception* e) {
    char* stack = malloc(MAX_ERROR_LENGTH);
    
    #pragma omp critical
    {
        snprintf(stack, MAX_ERROR_LENGTH, "Error: %s\nStack trace:\n%s",
                e->message, e->stack_trace ? e->stack_trace : "No stack trace available");
    }
    
    return stack;
}

// Create new response
Response* create_response(int status_code, const char* body) {
    Response* response = malloc(sizeof(Response));
    response->status_code = status_code;
    response->body = body ? strdup(body) : NULL;
    return response;
}

// Main error handler function
Response* handle_error(const Exception* e) {
    char* error_stack = NULL;
    int status_code;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            error_stack = generate_exception_stack(e);
        }
        
        #pragma omp section
        {
            if (e->type == WEB_APPLICATION_EXCEPTION) {
                status_code = e->status_code;
            } else {
                status_code = HTTP_BAD_REQUEST;
            }
        }
    }
    
    Response* response = create_response(status_code, error_stack);
    free(error_stack);
    
    return response;
}

// Clean up resources
void free_response(Response* response) {
    if (response) {
        free(response->body);
        free(response);
    }
}

void free_exception(Exception* e) {
    if (e) {
        free(e->message);
        free(e->stack_trace);
        free(e);
    }
}