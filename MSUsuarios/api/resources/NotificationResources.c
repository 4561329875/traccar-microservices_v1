#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    int id;
    char username[50];
    char email[100];
} User;

typedef struct {
    char type[50];
} Typed;

typedef enum {
    SMS,
    EMAIL,
    PUSH
} NotificatorType;

void send_test_notifications(User* users, int user_count) {
    #pragma omp parallel for
    for (int i = 0; i < user_count; i++) {
        printf("Sending test notification to user %s\n", users[i].username);
    }
}

void get_notification_types(Typed* types, int* type_count) {
    *type_count = 3;
    strcpy(types[0].type, "SMS");
    strcpy(types[1].type, "EMAIL");
    strcpy(types[2].type, "PUSH");
}

void send_message(User* users, int user_count, const char* notificator) {
    #pragma omp parallel for
    for (int i = 0; i < user_count; i++) {
        printf("Sending %s notification to %s\n", notificator, users[i].username);
    }
}

int main() {
    User users[3] = {
        {1, "john_doe", "john@example.com"},
        {2, "jane_smith", "jane@example.com"},
        {3, "bob_wilson", "bob@example.com"}
    };

    Typed notification_types[3];
    int type_count;
    get_notification_types(notification_types, &type_count);

    send_test_notifications(users, 3);
    send_message(users, 3, "EMAIL");

    return 0;
}