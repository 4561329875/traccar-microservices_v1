#ifndef PERMISSIONS_SERVICE_H
#define PERMISSIONS_SERVICE_H

#include <stdlib.h>
#include <string.h>
#include <openmp.h>

typedef struct {
    int is_administrator;
    int device_limit;
    int user_limit;
    int readonly;
    int device_readonly;
} UserPermissions;

typedef struct {
    UserPermissions* users;
    int user_count;
} PermissionsService;

int check_admin(
    PermissionsService* service, 
    long user_id
) {
    int is_admin = 0;
    
    #pragma omp parallel
    {
        #pragma omp critical
        {
            for (int i = 0; i < service->user_count; i++) {
                if (service->users[i].is_administrator) {
                    is_admin = 1;
                    break;
                }
            }
        }
    }
    
    return is_admin;
}

int check_user_update_permissions(
    PermissionsService* service, 
    long editor_id, 
    UserPermissions* before, 
    UserPermissions* after
) {
    int permission_granted = 0;
    
    #pragma omp parallel
    {
        #pragma omp critical
        {
            // Check for critical permission changes
            if (before->is_administrator != after->is_administrator ||
                before->device_limit != after->device_limit ||
                before->user_limit != after->user_limit) {
                permission_granted = check_admin(service, editor_id);
            }
        }
    }
    
    return permission_granted;
}

int check_device_edit_permissions(
    PermissionsService* service, 
    long user_id, 
    int is_addition
) {
    int can_edit = 1;
    
    #pragma omp parallel
    {
        #pragma omp critical
        {
            UserPermissions* user = NULL;
            for (int i = 0; i < service->user_count; i++) {
                if (service->users[i].is_administrator) {
                    user = &service->users[i];
                    break;
                }
            }
            
            if (user && !user->is_administrator) {
                if (user->device_readonly || 
                    (is_addition && user->device_limit == 0)) {
                    can_edit = 0;
                }
            }
        }
    }
    
    return can_edit;
}

#endif // PERMISSIONS_SERVICE_H