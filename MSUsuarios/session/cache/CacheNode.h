#ifndef CACHE_NODE_H
#define CACHE_NODE_H

#include <stdbool.h>
#include "base_model.h"

// Forward declaration for the linked list of models
typedef struct ModelSet ModelSet;
typedef struct CacheNode CacheNode;

// Structure to hold class type information
typedef struct {
    const char* class_name;
    size_t size;
} ClassInfo;

// Structure for model sets
struct ModelSet {
    CacheNode** nodes;
    size_t capacity;
    size_t size;
};

// Main cache node structure
struct CacheNode {
    BaseModel* value;
    struct {
        ClassInfo* class_info;
        ModelSet* set;
    }* links;
    size_t links_count;
    struct {
        ClassInfo* class_info;
        ModelSet* set;
    }* backlinks;
    size_t backlinks_count;
};

// Function prototypes
CacheNode* create_cache_node(BaseModel* value);
void destroy_cache_node(CacheNode* node);
BaseModel* get_value(CacheNode* node);
void set_value(CacheNode* node, BaseModel* value);
ModelSet* get_links(CacheNode* node, ClassInfo* class_info, bool forward);
CacheNode** get_all_links(CacheNode* node, bool forward, size_t* count);

#endif