// CacheGraph.c
#include "CacheGraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75

struct CacheNode {
    BaseModel* value;
    CacheNode** forward_links;
    CacheNode** backward_links;
    int forward_count;
    int forward_capacity;
    int backward_count;
    int backward_capacity;
};

struct CacheGraph {
    CacheNode** roots;
    int root_count;
    int root_capacity;
    CacheKey* root_keys;
    
    CacheNode** nodes;
    CacheKey* node_keys;
    int node_count;
    int node_capacity;
    omp_lock_t lock;
};

static void resize_node_links(CacheNode* node, bool forward) {
    int new_capacity;
    if (forward) {
        new_capacity = node->forward_capacity * 2;
        node->forward_links = realloc(node->forward_links, sizeof(CacheNode*) * new_capacity);
        node->forward_capacity = new_capacity;
    } else {
        new_capacity = node->backward_capacity * 2;
        node->backward_links = realloc(node->backward_links, sizeof(CacheNode*) * new_capacity);
        node->backward_capacity = new_capacity;
    }
}

static void add_link(CacheNode* from, CacheNode* to, bool forward) {
    if (forward) {
        if (from->forward_count >= from->forward_capacity) {
            resize_node_links(from, true);
        }
        from->forward_links[from->forward_count++] = to;
    } else {
        if (from->backward_count >= from->backward_capacity) {
            resize_node_links(from, false);
        }
        from->backward_links[from->backward_count++] = to;
    }
}

void cache_graph_add_object(CacheGraph* graph, BaseModel* value) {
    omp_set_lock(&graph->lock);
    
    CacheKey key = cache_key_create(value);
    CacheNode* node = malloc(sizeof(CacheNode));
    node->value = value;
    node->forward_capacity = INITIAL_CAPACITY;
    node->backward_capacity = INITIAL_CAPACITY;
    node->forward_count = 0;
    node->backward_count = 0;
    node->forward_links = malloc(sizeof(CacheNode*) * INITIAL_CAPACITY);
    node->backward_links = malloc(sizeof(CacheNode*) * INITIAL_CAPACITY);

    if (graph->node_count >= graph->node_capacity * LOAD_FACTOR) {
        int new_capacity = graph->node_capacity * 2;
        graph->nodes = realloc(graph->nodes, sizeof(CacheNode*) * new_capacity);
        graph->node_keys = realloc(graph->node_keys, sizeof(CacheKey) * new_capacity);
        graph->node_capacity = new_capacity;
    }

    graph->nodes[graph->node_count] = node;
    graph->node_keys[graph->node_count] = key;
    graph->node_count++;

    omp_unset_lock(&graph->lock);
}