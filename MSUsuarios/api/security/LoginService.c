#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openmp.h>
#include <time.h>

#include "login_result.h"
#include "code_required_exception.h"

typedef struct {
    char* service_account_token;
    int force_ldap;
    int force_openid;
} LoginConfig;

typedef struct {
    LoginConfig* config;
    User* users;
    int user_count;
} LoginService;

LoginResult* login_by_scheme(
    LoginService* service, 
    const char* scheme, 
    const char* credentials
) {
    if (strcmp(scheme, "bearer") == 0) {
        return login_by_token(service, credentials);
    }
    
    if (strcmp(scheme, "basic") == 0) {
        // Simulate basic authentication decoding
        char* decoded = base64_decode(credentials);
        char* email = strtok(decoded, ":");
        char* password = strtok(NULL, ":");
        
        LoginResult* result = login_by_credentials(service, email, password, 0);
        free(decoded);
        return result;
    }
    
    return NULL;
}

LoginResult* login_by_token(
    LoginService* service, 
    const char* token
) {
    #pragma omp parallel
    {
        // Token verification logic
        if (service->config->service_account_token && 
            strcmp(token, service->config->service_account_token) == 0) {
            User* service_user = create_service_account_user();
            return create_login_result(service_user, NULL);
        }
    }
    
    return NULL;
}

LoginResult* login_by_credentials(
    LoginService* service, 
    const char* email, 
    const char* password,
    int code
) {
    if (service->config->force_openid) {
        return NULL;
    }

    LoginResult* result = NULL;
    
    #pragma omp parallel
    {
        #pragma omp critical
        {
            // Find user by email or login
            User* user = find_user_by_email_or_login(service, email);
            
            if (user) {
                if (verify_user_credentials(user, password)) {
                    // Validate 2FA if required
                    if (validate_two_factor(user, code)) {
                        result = create_login_result(user, NULL);
                    }
                }
            }
        }
    }
    
    return result;
}

int validate_two_factor(User* user, int code) {
    // TOTP validation logic
    return 1; // Simplified
}

#endif // LOGIN_SERVICE_H