#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

// Basic model structure
typedef struct {
    long id;
    char* name;
} BaseModel;

// User structure
typedef struct {
    BaseModel base;
    char* email;
    bool admin;
} User;

// Collection structure
typedef struct {
    void** items;
    size_t size;
    size_t capacity;
} Collection;

// Condition structure
typedef struct Condition {
    enum {
        PERMISSION,
        MERGE
    } type;
    union {
        struct {
            long user_id;
            const char* class_name;
            bool exclude_groups;
        } permission;
        struct {
            struct Condition** conditions;
            size_t count;
        } merge;
    } data;
} Condition;

// Request structure
typedef struct {
    const char* columns;
    Condition* condition;
    const char* order;
} Request;

// Initialize collection
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

// Create permission condition
Condition* create_permission_condition(long user_id, const char* class_name) {
    Condition* condition = malloc(sizeof(Condition));
    condition->type = PERMISSION;
    condition->data.permission.user_id = user_id;
    condition->data.permission.class_name = strdup(class_name);
    condition->data.permission.exclude_groups = false;
    return condition;
}

// Get objects implementation
Collection* get_objects(bool all, long user_id, long current_user_id, 
                       const char* base_class, const char* sort_field) {
    Collection* conditions = create_collection();
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (all) {
                if (!is_admin(current_user_id)) {
                    Condition* condition = create_permission_condition(current_user_id, base_class);
                    collection_add(conditions, condition);
                }
            }
        }
        
        #pragma omp section
        {
            if (!all) {
                if (user_id == 0) {
                    user_id = current_user_id;
                } else {
                    check_user_permission(current_user_id, user_id);
                }
                Condition* condition = create_permission_condition(user_id, base_class);
                collection_add(conditions, condition);
            }
        }
    }

    // Create and execute request
    Request request = {
        .columns = "ALL",
        .condition = merge_conditions(conditions),
        .order = sort_field
    };

    // Get objects from storage
    Collection* result = storage_get_objects(base_class, &request);

    // Clean up conditions
    free_conditions(conditions);
    
    return result;
}

// Clean up resources
void free_collection(Collection* collection) {
    if (!collection) return;
    
    #pragma omp parallel for
    for (size_t i = 0; i < collection->size; i++) {
        free(collection->items[i]);
    }
    
    free(collection->items);
    free(collection);
}

void free_condition(Condition* condition) {
    if (!condition) return;
    
    if (condition->type == PERMISSION) {
        free((char*)condition->data.permission.class_name);
    } else if (condition->type == MERGE) {
        for (size_t i = 0; i < condition->data.merge.count; i++) {
            free_condition(condition->data.merge.conditions[i]);
        }
        free(condition->data.merge.conditions);
    }
    
    free(condition);
}