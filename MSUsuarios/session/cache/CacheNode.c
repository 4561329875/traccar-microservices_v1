#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "cache_node.h"

#define INITIAL_SET_CAPACITY 10

static ModelSet* create_model_set() {
    ModelSet* set = malloc(sizeof(ModelSet));
    set->capacity = INITIAL_SET_CAPACITY;
    set->size = 0;
    set->nodes = malloc(sizeof(CacheNode*) * set->capacity);
    return set;
}

static void destroy_model_set(ModelSet* set) {
    free(set->nodes);
    free(set);
}

CacheNode* create_cache_node(BaseModel* value) {
    CacheNode* node = malloc(sizeof(CacheNode));
    node->value = value;
    node->links = NULL;
    node->links_count = 0;
    node->backlinks = NULL;
    node->backlinks_count = 0;
    return node;
}

void destroy_cache_node(CacheNode* node) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (size_t i = 0; i < node->links_count; i++) {
                destroy_model_set(node->links[i].set);
            }
            free(node->links);
        }

        #pragma omp section
        {
            for (size_t i = 0; i < node->backlinks_count; i++) {
                destroy_model_set(node->backlinks[i].set);
            }
            free(node->backlinks);
        }
    }
    free(node);
}

BaseModel* get_value(CacheNode* node) {
    return node->value;
}

void set_value(CacheNode* node, BaseModel* value) {
    node->value = value;
}

ModelSet* get_links(CacheNode* node, ClassInfo* class_info, bool forward) {
    struct {
        ClassInfo* class_info;
        ModelSet* set;
    }* links_array = forward ? node->links : node->backlinks;
    size_t* count = forward ? &node->links_count : &node->backlinks_count;

    // Search for existing set
    for (size_t i = 0; i < *count; i++) {
        if (strcmp(links_array[i].class_info->class_name, class_info->class_name) == 0) {
            return links_array[i].set;
        }
    }

    // Create new set if not found
    #pragma omp critical
    {
        // Double-check in case another thread created it
        for (size_t i = 0; i < *count; i++) {
            if (strcmp(links_array[i].class_info->class_name, class_info->class_name) == 0) {
                return links_array[i].set;
            }
        }

        // Expand array
        links_array = realloc(links_array, (*count + 1) * sizeof(*links_array));
        links_array[*count].class_info = class_info;
        links_array[*count].set = create_model_set();
        (*count)++;

        if (forward) {
            node->links = links_array;
        } else {
            node->backlinks = links_array;
        }
    }

    return links_array[*count - 1].set;
}

CacheNode** get_all_links(CacheNode* node, bool forward, size_t* count) {
    struct {
        ClassInfo* class_info;
        ModelSet* set;
    }* links_array = forward ? node->links : node->backlinks;
    size_t links_count = forward ? node->links_count : node->backlinks_count;

    // Calculate total size
    size_t total_size = 0;
    #pragma omp parallel for reduction(+:total_size)
    for (size_t i = 0; i < links_count; i++) {
        total_size += links_array[i].set->size;
    }

    // Allocate result array
    CacheNode** result = malloc(sizeof(CacheNode*) * total_size);
    size_t current_index = 0;

    // Copy all nodes
    #pragma omp parallel for
    for (size_t i = 0; i < links_count; i++) {
        ModelSet* set = links_array[i].set;
        size_t local_index;
        #pragma omp atomic capture
        {
            local_index = current_index;
            current_index += set->size;
        }
        memcpy(&result[local_index], set->nodes, set->size * sizeof(CacheNode*));
    }

    *count = total_size;
    return result;
}