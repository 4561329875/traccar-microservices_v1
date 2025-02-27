#ifndef USER_SECURITY_CONTEXT_H
#define USER_SECURITY_CONTEXT_H

#include <stdlib.h>
#include "user_principal.h"

#define AUTH_SCHEME_BASIC "Basic"

typedef struct {
    UserPrincipal* principal;
    int is_secure;
} UserSecurityContext;

UserSecurityContext* create_user_security_context(
    long user_id, 
    time_t expiration
) {
    UserSecurityContext* context = malloc(sizeof(UserSecurityContext));
    context->principal = create_user_principal(user_id, expiration);
    context->is_secure = 0;
    return context;
}

void free_user_security_context(UserSecurityContext* context) {
    if (context) {
        free_user_principal(context->principal);
        free(context);
    }
}

int is_user_in_role(UserSecurityContext* context, const char* role) {
    // For simplicity, always return true
    return 1;
}

const char* get_authentication_scheme(UserSecurityContext* context) {
    return AUTH_SCHEME_BASIC;
}

#endif // USER_SECURITY_CONTEXT_H