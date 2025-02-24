#ifndef USER_PRINCIPAL_H
#define USER_PRINCIPAL_H

#include <stdlib.h>
#include <time.h>

typedef struct {
    long user_id;
    time_t expiration;
} UserPrincipal;

UserPrincipal* create_user_principal(long user_id, time_t expiration) {
    UserPrincipal* principal = malloc(sizeof(UserPrincipal));
    principal->user_id = user_id;
    principal->expiration = expiration;
    return principal;
}

void free_user_principal(UserPrincipal* principal) {
    if (principal) {
        free(principal);
    }
}

int is_principal_expired(UserPrincipal* principal) {
    return principal && 
           principal->expiration != 0 && 
           time(NULL) > principal->expiration;
}

#endif // USER_PRINCIPAL_H