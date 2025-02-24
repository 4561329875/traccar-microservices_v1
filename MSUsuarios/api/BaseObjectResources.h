// base_object_resource.h
#ifndef BASE_OBJECT_RESOURCE_H
#define BASE_OBJECT_RESOURCE_H

#include "storage.h"
#include "cache_manager.h"
#include "connection_manager.h"
#include "permissions_service.h"

typedef struct {
    Storage *storage;
    CacheManager *cache_manager;
    ConnectionManager *connection_manager;
    PermissionsService *permissions_service;
    const char *base_class;
} BaseObjectResource;

BaseObjectResource* base_object_resource_create(Storage *storage,
                                              CacheManager *cache_manager,
                                              ConnectionManager *connection_manager,
                                              PermissionsService *permissions_service,
                                              const char *base_class);
void base_object_resource_destroy(BaseObjectResource *resource);

json_t* base_object_resource_get_single(BaseObjectResource *resource, long id, long user_id);
json_t* base_object_resource_add(BaseObjectResource *resource, json_t *entity, long user_id);
json_t* base_object_resource_update(BaseObjectResource *resource, json_t *entity, long user_id);
bool base_object_resource_remove(BaseObjectResource *resource, long id, long user_id);

#endif // BASE_OBJECT_RESOURCE_H