#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openmp.h>

typedef struct {
    long id;
    char name[100];
    char email[100];
    int administrator;
    int userLimit;
    char totpKey[50];
} User;

typedef struct {
    User* users;
    int userCount;
} Storage;

typedef struct {
    int registration;
    int forceTOTP;
} ServerConfig;

typedef struct {
    long userId;
    int isAdmin;
} PermissionsService;

User* add_user(
    Storage* storage, 
    PermissionsService* permissions, 
    ServerConfig* server_config, 
    User* new_user
) {
    User* current_user = NULL;
    
    // Check user permissions and limits
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                if (permissions->userId > 0) {
                    current_user = find_user_by_id(storage, permissions->userId);
                }
            }

            #pragma omp section
            {
                if (current_user == NULL || !current_user->administrator) {
                    // User creation logic with various checks
                    if (server_config->registration == 0) {
                        // Registration disabled
                        new_user = NULL;
                    }

                    if (server_config->forceTOTP && new_user->totpKey[0] == '\0') {
                        // TOTP key required
                        new_user = NULL;
                    }
                }
            }
        }
    }

    if (new_user != NULL) {
        // Add user to storage
        storage->users = realloc(
            storage->users, 
            (storage->userCount + 1) * sizeof(User)
        );
        storage->users[storage->userCount] = *new_user;
        storage->userCount++;
    }

    return new_user;
}

User* find_user_by_id(Storage* storage, long userId) {
    User* found_user = NULL;

    #pragma omp parallel for
    for (int i = 0; i < storage->userCount; i++) {
        if (storage->users[i].id == userId) {
            #pragma omp critical
            {
                found_user = &storage->users[i];
            }
        }
    }

    return found_user;
}

int remove_user(Storage* storage, long userId) {
    int removed = 0;

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < storage->userCount; i++) {
            if (storage->users[i].id == userId) {
                #pragma omp critical
                {
                    // Remove user from storage
                    memmove(&storage->users[i], &storage->users[i+1], 
                            (storage->userCount - i - 1) * sizeof(User));
                    storage->userCount--;
                    removed = 1;
                }
            }
        }
    }

    return removed;
}