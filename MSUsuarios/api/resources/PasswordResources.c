#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

typedef struct {
    int id;
    char username[50];
    char email[100];
    char hashed_password[256];
} User;

typedef struct {
    char token[100];
    int user_id;
    time_t expiration;
} PasswordResetToken;

void generate_reset_token(PasswordResetToken* token, int user_id) {
    snprintf(token->token, sizeof(token->token), "reset_%d_%ld", user_id, time(NULL));
    token->user_id = user_id;
    token->expiration = time(NULL) + (24 * 60 * 60); // 24 hours validity
}

int reset_password_requests(User* users, int user_count) {
    int successful_resets = 0;
    PasswordResetToken* tokens = malloc(user_count * sizeof(PasswordResetToken));

    #pragma omp parallel for reduction(+:successful_resets)
    for (int i = 0; i < user_count; i++) {
        generate_reset_token(&tokens[i], users[i].id);
        
        // Simulate email sending
        printf("Password reset token generated for %s: %s\n", 
               users[i].username, tokens[i].token);
        
        successful_resets++;
    }

    free(tokens);
    return successful_resets;
}

int update_password(User* user, const char* token, const char* new_password) {
    PasswordResetToken reset_token;
    generate_reset_token(&reset_token, user->id);

    if (strcmp(token, reset_token.token) == 0 && time(NULL) <= reset_token.expiration) {
        // In a real scenario, use secure password hashing
        strncpy(user->hashed_password, new_password, sizeof(user->hashed_password));
        printf("Password updated for user %s\n", user->username);
        return 1;
    }
    return 0;
}

int main() {
    User users[3] = {
        {1, "john_doe", "john@example.com", ""},
        {2, "jane_smith", "jane@example.com", ""},
        {3, "bob_wilson", "bob@example.com", ""}
    };

    int reset_count = reset_password_requests(users, 3);
    printf("Successful password reset requests: %d\n", reset_count);

    return 0;
}