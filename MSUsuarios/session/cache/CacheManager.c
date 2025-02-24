// CacheManager.c
#include "CacheManager.h"
#include <stdlib.h>
#include <omp.h>

struct CacheManager {
    CacheGraph* graph;
    Server* server;
    Position** device_positions;
    int* device_reference_counts;
    void*** device_references;
    int device_capacity;
    omp_lock_t lock;
};

CacheManager* cache_manager_create(void) {
    CacheManager* manager = malloc(sizeof(CacheManager));
    manager->graph = cache_graph_create();
    manager->device_capacity = 1024;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            manager->device_positions = calloc(manager->device_capacity, sizeof(Position*));
        }
        #pragma omp section
        {
            manager->device_reference_counts = calloc(manager->device_capacity, sizeof(int));
        }
        #pragma omp section
        {
            manager->device_references = calloc(manager->device_capacity, sizeof(void**));
        }
    }
    
    #pragma omp parallel for
    for (int i = 0; i < manager->device_capacity; i++) {
        manager->device_references[i] = malloc(sizeof(void*) * 8);
    }
    
    omp_init_lock(&manager->lock);
    return manager;
}

void cache_manager_update_position(CacheManager* manager, Position* position) {
    omp_set_lock(&manager->lock);
    
    if (position->device_id < manager->device_capacity) {
        if (manager->device_positions[position->device_id] != NULL) {
            free(manager->device_positions[position->device_id]);
        }
        manager->device_positions[position->device_id] = position;
    }
    
    omp_unset_lock(&manager->lock);
}

void cache_manager_invalidate_object(CacheManager* manager, ModelClass class, 
                                   int64_t id, ObjectOperation operation) {
    omp_set_lock(&manager->lock);
    
    if (operation == OPERATION_DELETE) {
        cache_graph_remove_object(manager->graph, class, id);
    } else if (operation == OPERATION_UPDATE) {
        if (class == MODEL_SERVER) {
            // Actualizar servidor
            if (manager->server != NULL) {
                free(manager->server);
            }
            // Cargar nuevo servidor desde almacenamiento
            // manager->server = storage_load_server();
        } else {
            BaseModel* before = cache_graph_get_object(manager->graph, class, id);
            if (before != NULL) {
                // Cargar objeto actualizado desde almacenamiento
                // BaseModel* after = storage_load_object(class, id);
                // cache_graph_update_object(manager->graph, after);
            }
        }
    }
    
    omp_unset_lock(&manager->lock);
}

void cache_manager_add_device(CacheManager* manager, int64_t device_id, void* key) {
    omp_set_lock(&manager->lock);
    
    if (device_id >= manager->device_capacity) {
        int new_capacity = device_id * 2;
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                manager->device_positions = realloc(manager->device_positions, 
                    new_capacity * sizeof(Position*));
            }
            #pragma omp section
            {
                manager->device_reference_counts = realloc(manager->device_reference_counts,
                    new_capacity * sizeof(int));
            }
            #pragma omp section
            {
                manager->device_references = realloc(manager->device_references,
                    new_capacity * sizeof(void**));
            }
        }
        
        #pragma omp parallel for
        for (int i = manager->device_capacity; i < new_capacity; i++) {
            manager->device_positions[i] = NULL;
            manager->device_reference_counts[i] = 0;
            manager->device_references[i] = malloc(sizeof(void*) * 8);
        }
        
        manager->device_capacity = new_capacity;
    }
    
    int count = manager->device_reference_counts[device_id];
    if (count == 0) {
        // Cargar nuevo dispositivo
        // Device* device = storage_load_device(device_id);
        // cache_graph_add_object(manager->graph, (BaseModel*)device);
    }
    
    manager->device_references[device_id][count] = key;
    manager->device_reference_counts[device_id]++;
    
    omp_unset_lock(&manager->lock);
}