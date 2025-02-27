// base_object_resource.c
#include "base_object_resource.h"
#include <string.h>
#include <stdlib.h>
#include <omp.h>

BaseObjectResource* base_object_resource_create(Storage *storage,
                                              CacheManager *cache_manager,
                                              ConnectionManager *connection_manager,
                                              PermissionsService *permissions_service,
                                              const char *base_class) {
    BaseObjectResource *resource = malloc(sizeof(BaseObjectResource));
    if (!resource) return NULL;
    
    resource->storage = storage;
    resource->cache_manager = cache_manager;
    resource->connection_manager = connection_manager;
    resource->permissions_service = permissions_service;
    resource->base_class = strdup(base_class);
    
    return resource;
}

void base_object_resource_destroy(BaseObjectResource *resource) {
    if (resource) {
        free((void*)resource->base_class);
        free(resource);
    }
}

json_t* base_object_resource_get_single(BaseObjectResource *resource, long id, long user_id) {
    if (!permissions_service_check_permission(resource->permissions_service, 
                                           resource->base_class, user_id, id)) {
        return NULL;
    }
    
    json_t *result = NULL;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            result = storage_get_object(resource->storage, resource->base_class, id);
        }
    }
    
    return result;
}

json_t* base_object_resource_add(BaseObjectResource *resource, json_t *entity, long user_id) {
    if (!permissions_service_check_edit(resource->permissions_service, user_id, entity, true, false)) {
        return NULL;
    }
    
    long id = 0;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            id = storage_add_object(resource->storage, resource->base_class, entity);
        }
    }
    
    if (id > 0) {
        json_object_set_new(entity, "id", json_integer(id));
        
        // Add permissions and invalidate cache
        if (user_id != SERVICE_ACCOUNT_USER_ID) {
            storage_add_permission(resource->storage, "user", user_id, resource->base_class, id);
            cache_manager_invalidate_permission(resource->cache_manager, true, "user", 
                                             user_id, resource->base_class, id, true);
            connection_manager_invalidate_permission(resource->connection_manager, true, 
                                                  "user", user_id, resource->base_class, id, true);
        }
        
        return entity;
    }
    
    return NULL;
}
