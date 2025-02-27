#ifndef SERVICE_ACCOUNT_USER_H
#define SERVICE_ACCOUNT_USER_H

#include <stdlib.h>
#include <string.h>

#define SERVICE_ACCOUNT_USER_ID 9000000000000000000LL

typedef struct {
    long id;
    char* name;
    char* email;
    int administrator;
} ServiceAccountUser;

ServiceAccountUser* create_service_account_user() {
    ServiceAccountUser* user = malloc(sizeof(ServiceAccountUser));
    
    user->id = SERVICE_ACCOUNT_USER_ID;
    user->name = strdup("Service Account");
    user->email = strdup("none");
    user->administrator = 1;
    
    return user;
}

void free_service_account_user(ServiceAccountUser* user) {
    if (user) {
        free(user->name);
        free(user->email);
        free(user);
    }
}

#endif // SERVICE_ACCOUNT_USER_H