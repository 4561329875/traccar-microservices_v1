// base_resource.c
#include "base_resource.h"
#include <stdlib.h>
#include <omp.h>

BaseResource* base_resource_create(Storage *storage,
                                 PermissionsService *permissions_service) {
    BaseResource *resource = malloc(sizeof(BaseResource));
    if (!resource) return NULL;
    
    resource->storage = storage;
    resource->permissions_service = permissions_service;
    resource->user_id = 0;
    
    return resource;
}

void base_resource_destroy(BaseResource *resource) {
    if (resource) {
        free(resource);
    }
}

void base_resource_set_user_id(BaseResource *resource, long user_id) {
    if (resource) {
        resource->user_id = user_id;
    }
}

long base_resource_get_user_id(BaseResource *resource) {
    return resource ? resource->user_id : 0;
}