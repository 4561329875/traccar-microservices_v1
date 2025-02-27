#ifndef SECURITY_REQUEST_FILTER_H
#define SECURITY_REQUEST_FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openmp.h>
#include <time.h>

#include "login_service.h"
#include "user_security_context.h"

typedef struct {
    LoginService* login_service;
    StatisticsManager* stats_manager;
    HttpServletRequest* request;
    ResourceInfo* resource_info;
} SecurityRequestFilter;

UserSecurityContext* authenticate_request(
    SecurityRequestFilter* filter, 
    const char* auth_header
) {
    UserSecurityContext* security_context = NULL;

    #pragma omp parallel
    {
        #pragma omp critical
        {
            if (auth_header) {
                char* scheme = strtok((char*)auth_header, " ");
                char* credentials = strtok(NULL, "");

                LoginResult* login_result = login_service_login(
                    filter->login_service, 
                    scheme, 
                    credentials
                );

                if (login_result && login_result->user) {
                    // Register request in statistics
                    statistics_manager_register_request(
                        filter->stats_manager, 
                        login_result->user->id
                    );

                    // Create security context
                    security_context = create_user_security_context(
                        login_result->user->id, 
                        login_result->expiration
                    );
                }
            }
        }
    }

    return security_context;
}

int validate_request_access(
    SecurityRequestFilter* filter,
    UserSecurityContext* security_context
) {
    int access_granted = 0;

    #pragma omp parallel
    {
        #pragma omp critical
        {
            // Check if method requires authentication
            if (is_method_permit_all(filter->resource_info)) {
                access_granted = 1;
            } else if (security_context) {
                access_granted = 1;
            }
        }
    }

    return access_granted;
}

void process_security_filter(SecurityRequestFilter* filter) {
    if (strcmp(filter->request->method, "OPTIONS") == 0) {
        return;
    }

    UserSecurityContext* security_context = NULL;

    // Authenticate request
    char* auth_header = get_authorization_header(filter->request);
    security_context = authenticate_request(filter, auth_header);

    // If no authentication from header, check session
    if (!security_context) {
        security_context = authenticate_from_session(filter->request);
    }

    // Validate access
    if (!validate_request_access(filter, security_context)) {
        handle_unauthorized_access(filter->request);
    }

    // Set security context for request processing
    if (security_context) {
        set_request_security_context(filter->request, security_context);
    }
}

#endif // SECURITY_REQUEST_FILTER_H