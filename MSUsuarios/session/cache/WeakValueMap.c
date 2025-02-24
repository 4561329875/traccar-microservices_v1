#include <stdlib.h>
#include <omp.h>
#include "weak_value_map.h"

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct WeakRef {
    Value value;
    int is_valid;
} WeakRef;

typedef struct Entry {
    Key key;
    WeakRef* ref;
    struct Entry* next;
} Entry;

struct WeakValueMap {
    Entry** buckets;
    size_t capacity;
    size_t size;
    KeyCompareFunc compare;
    KeyHashFunc hash;
    omp_lock_t lock;
};

WeakValueMap* weak_map_create(KeyCompareFunc compare_func, KeyHashFunc hash_func) {
    WeakValueMap* map = malloc(sizeof(WeakValueMap));
    map->capacity = INITIAL_CAPACITY;
    map->size = 0;
    map->buckets = calloc(INITIAL_CAPACITY, sizeof(Entry*));
    map->compare = compare_func;
    map->hash = hash_func;
    omp_init_lock(&map->lock);
    return map;
}

static void free_entry_chain(Entry* entry) {
    while (entry != NULL) {
        Entry* next = entry->next;
        free(entry->ref);
        free(entry);
        entry = next;
    }
}

void weak_map_destroy(WeakValueMap* map) {
    if (map == NULL) return;

    #pragma omp parallel for
    for (size_t i = 0; i < map->capacity; i++) {
        free_entry_chain(map->buckets[i]);
    }

    omp_destroy_lock(&map->lock);
    free(map->buckets);
    free(map);
}

static void resize_if_needed(WeakValueMap* map) {
    if ((double)map->size / map->capacity > LOAD_FACTOR_THRESHOLD) {
        size_t new_capacity = map->capacity * 2;
        Entry** new_buckets = calloc(new_capacity, sizeof(Entry*));

        #pragma omp parallel for
        for (size_t i = 0; i < map->capacity; i++) {
            Entry* entry = map->buckets[i];
            while (entry != NULL) {
                Entry* next = entry->next;
                size_t new_index = map->hash(entry->key) % new_capacity;
                
                #pragma omp critical
                {
                    entry->next = new_buckets[new_index];
                    new_buckets[new_index] = entry;
                }
                
                entry = next;
            }
        }

        free(map->buckets);
        map->buckets = new_buckets;
        map->capacity = new_capacity;
    }
}

void weak_map_put(WeakValueMap* map, Key key, Value value) {
    omp_set_lock(&map->lock);
    
    size_t index = map->hash(key) % map->capacity;
    Entry* entry = map->buckets[index];
    
    while (entry != NULL) {
        if (map->compare(entry->key, key) == 0) {
            if (entry->ref == NULL) {
                entry->ref = malloc(sizeof(WeakRef));
            }
            entry->ref->value = value;
            entry->ref->is_valid = 1;
            omp_unset_lock(&map->lock);
            return;
        }
        entry = entry->next;
    }

    entry = malloc(sizeof(Entry));
    entry->key = key;
    entry->ref = malloc(sizeof(WeakRef));
    entry->ref->value = value;
    entry->ref->is_valid = 1;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;

    resize_if_needed(map);
    omp_unset_lock(&map->lock);
}

Value weak_map_get(WeakValueMap* map, Key key) {
    size_t index = map->hash(key) % map->capacity;
    Entry* entry = map->buckets[index];

    while (entry != NULL) {
        if (map->compare(entry->key, key) == 0) {
            if (entry->ref != NULL && entry->ref->is_valid) {
                return entry->ref->value;
            }
            return NULL;
        }
        entry = entry->next;
    }
    return NULL;
}

Value weak_map_remove(WeakValueMap* map, Key key) {
    omp_set_lock(&map->lock);
    
    size_t index = map->hash(key) % map->capacity;
    Entry* entry = map->buckets[index];
    Entry* prev = NULL;
    Value result = NULL;

    while (entry != NULL) {
        if (map->compare(entry->key, key) == 0) {
            if (entry->ref != NULL && entry->ref->is_valid) {
                result = entry->ref->value;
            }
            
            if (prev == NULL) {
                map->buckets[index] = entry->next;
            } else {
                prev->next = entry->next;
            }
            
            free(entry->ref);
            free(entry);
            map->size--;
            break;
        }
        prev = entry;
        entry = entry->next;
    }

    omp_unset_lock(&map->lock);
    return result;
}

void weak_map_clean(WeakValueMap* map) {
    omp_set_lock(&map->lock);
    
    #pragma omp parallel for
    for (size_t i = 0; i < map->capacity; i++) {
        Entry* entry = map->buckets[i];
        Entry* prev = NULL;

        while (entry != NULL) {
            if (entry->ref == NULL || !entry->ref->is_valid) {
                Entry* to_remove = entry;
                
                #pragma omp critical
                {
                    if (prev == NULL) {
                        map->buckets[i] = entry->next;
                    } else {
                        prev->next = entry->next;
                    }
                    map->size--;
                }
                
                entry = entry->next;
                free(to_remove->ref);
                free(to_remove);
            } else {
                prev = entry;
                entry = entry->next;
            }
        }
    }

    omp_unset_lock(&map->lock);
}