#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

// Basic type definitions
typedef long long id_t;
typedef struct {
    id_t id;
    char* name;
} BaseModel;

typedef struct {
    id_t id;
    char* name;
    id_t userId;
} User;

typedef struct {
    id_t id;
    char* name;
    id_t groupId;
} Device;

typedef struct {
    id_t id;
    char* name;
} Group;

// Collection structure
typedef struct {
    void** items;
    size_t size;
    size_t capacity;
} Collection;

// Condition structure
typedef struct {
    enum {
        PERMISSION,
        EQUALS,
        MERGE
    } type;
    union {
        struct {
            id_t userId;
            char* className;
            bool excludeGroups;
        } permission;
        struct {
            char* field;
            void* value;
        } equals;
        struct {
            struct Condition** conditions;
            size_t count;
        } merge;
    } data;
} Condition;

// Create new collection
Collection* create_collection() {
    Collection* collection = malloc(sizeof(Collection));
    collection->capacity = 10;
    collection->size = 0;
    collection->items = malloc(sizeof(void*) * collection->capacity);
    return collection;
}

// Add item to collection
void collection_add(Collection* collection, void* item) {
    if (collection->size >= collection->capacity) {
        collection->capacity *= 2;
        collection->items = realloc(collection->items, sizeof(void*) * collection->capacity);
    }
    collection->items[collection->size++] = item;
}

// Get objects with permissions check
Collection* get_objects(id_t user_id, bool all, id_t requested_user_id, 
                       id_t group_id, id_t device_id, const char* sort_field) {
    Collection* result = create_collection();
    Collection* conditions = create_collection();

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Handle 'all' parameter
            if (all) {
                if (!is_admin(user_id)) {
                    Condition* condition = create_permission_condition(user_id, "BaseModel", false);
                    collection_add(conditions, condition);
                }
            } else {
                if (requested_user_id == 0) {
                    Condition* condition = create_permission_condition(user_id, "BaseModel", false);
                    collection_add(conditions, condition);
                } else {
                    check_user_permission(user_id, requested_user_id);
                    Condition* condition = create_permission_condition(requested_user_id, "BaseModel", true);
                    collection_add(conditions, condition);
                }
            }
        }

        #pragma omp section
        {
            // Handle group_id parameter
            if (group_id > 0) {
                check_group_permission(user_id, group_id);
                Condition* condition = create_permission_condition(group_id, "Group", true);
                collection_add(conditions, condition);
            }
        }

        #pragma omp section
        {
            // Handle device_id parameter
            if (device_id > 0) {
                check_device_permission(user_id, device_id);
                Condition* condition = create_permission_condition(device_id, "Device", true);
                collection_add(conditions, condition);
            }
        }
    }

    // Merge conditions and execute query
    Condition* merged = merge_conditions(conditions);
    execute_query(result, merged, sort_field);

    return result;
}

// Clean up resources
void free_collection(Collection* collection) {
    #pragma omp parallel for
    for (size_t i = 0; i < collection->size; i++) {
        free(collection->items[i]);
    }
    free(collection->items);
    free(collection);
}