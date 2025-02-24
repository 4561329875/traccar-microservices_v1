#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

// Configuration structure
typedef struct {
    char* web_origin;
} Config;

// HTTP headers constants
#define ORIGIN_ALL "*"
#define HEADERS_ALL "origin, content-type, accept, authorization"
#define METHODS_ALL "GET, POST, PUT, DELETE, OPTIONS"

// Response headers structure
typedef struct {
    char** names;
    char** values;
    int count;
    int capacity;
} ResponseHeaders;

// Initialize response headers
ResponseHeaders* create_response_headers() {
    ResponseHeaders* headers = malloc(sizeof(ResponseHeaders));
    headers->capacity = 10;
    headers->count = 0;
    headers->names = malloc(sizeof(char*) * headers->capacity);
    headers->values = malloc(sizeof(char*) * headers->capacity);
    return headers;
}

// Add header to response
void add_header(ResponseHeaders* headers, const char* name, const char* value) {
    if (headers->count >= headers->capacity) {
        headers->capacity *= 2;
        headers->names = realloc(headers->names, sizeof(char*) * headers->capacity);
        headers->values = realloc(headers->values, sizeof(char*) * headers->capacity);
    }
    
    headers->names[headers->count] = strdup(name);
    headers->values[headers->count] = strdup(value);
    headers->count++;
}

// Check if header exists
bool contains_header(ResponseHeaders* headers, const char* name) {
    #pragma omp parallel for
    for (int i = 0; i < headers->count; i++) {
        if (strcmp(headers->names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

// Filter implementation
void cors_filter(Config* config, const char* request_origin, ResponseHeaders* response) {
    // Using OpenMP to parallelize header processing
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (!contains_header(response, "Access-Control-Allow-Headers")) {
                add_header(response, "Access-Control-Allow-Headers", HEADERS_ALL);
            }
        }

        #pragma omp section
        {
            if (!contains_header(response, "Access-Control-Allow-Credentials")) {
                add_header(response, "Access-Control-Allow-Credentials", "true");
            }
        }

        #pragma omp section
        {
            if (!contains_header(response, "Access-Control-Allow-Methods")) {
                add_header(response, "Access-Control-Allow-Methods", METHODS_ALL);
            }
        }
    }

    if (!contains_header(response, "Access-Control-Allow-Origin")) {
        if (request_origin == NULL) {
            add_header(response, "Access-Control-Allow-Origin", ORIGIN_ALL);
        } else if (config->web_origin == NULL || 
                   strcmp(config->web_origin, ORIGIN_ALL) == 0 || 
                   strstr(config->web_origin, request_origin) != NULL) {
            add_header(response, "Access-Control-Allow-Origin", request_origin);
        }
    }
}

// Clean up resources
void free_response_headers(ResponseHeaders* headers) {
    #pragma omp parallel for
    for (int i = 0; i < headers->count; i++) {
        free(headers->names[i]);
        free(headers->values[i]);
    }
    free(headers->names);
    free(headers->values);
    free(headers);
}