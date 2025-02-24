#ifndef LOGIN_RESULT_H
#define LOGIN_RESULT_H

#include <stdlib.h>
#include <time.h>

typedef struct {
    long id;
    char* name;
    char* email;
    int administrator;
} User;

typedef struct {
    User* user;
    time_t* expiration;
} LoginResult;

LoginResult* create_login_result(User* user, time_t* expiration) {
    LoginResult* result = malloc(sizeof(LoginResult));
    result->user = user;
    result->expiration = expiration;
    return result;
}

void free_login_result(LoginResult* result) {
    if (result) {
        if (result->user) {
            free(result->user->name);
            free(result->user->email);
            free(result->user);
        }
        if (result->expiration) {
            free(result->expiration);
        }
        free(result);
    }
}

#endif // LOGIN_RESULT_H