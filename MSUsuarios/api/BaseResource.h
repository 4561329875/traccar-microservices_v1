// base_resource.h
#ifndef BASE_RESOURCE_H
#define BASE_RESOURCE_H

#include "storage.h"
#include "permissions_service.h"

typedef struct {
    Storage *storage;
    PermissionsService *permissions_service;
    long user_id;
} BaseResource;

BaseResource* base_resource_create(Storage *storage,
                                 PermissionsService *permissions_service);
void base_resource_destroy(BaseResource *resource);
void base_resource_set_user_id(BaseResource *resource, long user_id);
long base_resource_get_user_id(BaseResource *resource);

#endif // BASE_RESOURCE_H