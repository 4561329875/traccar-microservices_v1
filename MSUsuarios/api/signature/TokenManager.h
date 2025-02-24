// token_manager.h
#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include "crypto_manager.h"
#include <time.h>

#define DEFAULT_EXPIRATION_DAYS 7

typedef struct {
    int64_t user_id;
    time_t expiration;
} TokenData;

typedef struct {
    CryptoManager* crypto_manager;
} TokenManager;

TokenManager* token_manager_create(void);
void token_manager_destroy(TokenManager* manager);
char* token_manager_generate(TokenManager* manager, int64_t user_id, time_t expiration);
TokenData* token_manager_verify(TokenManager* manager, const char* token);

#endif // TOKEN_MANAGER_H