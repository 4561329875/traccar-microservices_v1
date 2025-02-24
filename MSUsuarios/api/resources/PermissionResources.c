#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    long owner_id;
    long property_id;
    char* owner_class;
    char* property_class;
} Permission;

typedef struct {
    int readonly;
    int disable_reports;
} UserRestrictions;

typedef struct {
    int id;
    int is_admin;
    char* username;
    UserRestrictions restrictions;
} User;

int check_permission(User* user, Permission* permission) {
    if (!user->is_admin) {
        // Simplified permission check
        return 1;
    }
    return 0;
}

int add_bulk_permissions(User* current_user, Permission** permissions, int count) {
    int successful_additions = 0;

    #pragma omp parallel for reduction(+:successful_additions)
    for (int i = 0; i < count; i++) {
        if (check_permission(current_user, permissions[i]) == 0) {
            // Simulate adding permission
            printf("Adding permission for %s\n", permissions[i]->property_class);
            successful_additions++;
        }
    }

    return successful_additions;
}

int remove_bulk_permissions(User* current_user, Permission** permissions, int count) {
    int successful_removals = 0;

    #pragma omp parallel for reduction(+:successful_removals)
    for (int i = 0; i < count; i++) {
        if (check_permission(current_user, permissions[i]) == 0) {
            // Simulate removing permission
            printf("Removing permission for %s\n", permissions[i]->property_class);
            successful_removals++;
        }
    }

    return successful_removals;
}

int main() {
    User admin_user = {1, 1, "admin_user", {0, 0}};
    User non_admin_user = {2, 0, "non_admin_user", {1, 0}};

    Permission* permissions[3];
    for (int i = 0; i < 3; i++) {
        permissions[i] = malloc(sizeof(Permission));
        permissions[i]->owner_id = admin_user.id;
        permissions[i]->property_id = i + 1;
        permissions[i]->owner_class = strdup("User");
        permissions[i]->property_class = strdup("Device");
    }

    int added = add_bulk_permissions(&admin_user, permissions, 3);
    printf("Successful permission additions: %d\n", added);

    int removed = remove_bulk_permissions(&admin_user, permissions, 3);
    printf("Successful permission removals: %d\n", removed);

    // Cleanup
    for (int i = 0; i < 3; i++) {
        free(permissions[i]->owner_class);
        free(permissions[i]->property_class);
        free(permissions[i]);
    }

    return 0;
}