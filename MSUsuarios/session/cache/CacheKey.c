// CacheKey.c
#include "CacheKey.h"
#include <omp.h>

CacheKey cache_key_create(BaseModel* object) {
    return (CacheKey) {
        .class = object->class,
        .id = object->id
    };
}

uint64_t cache_key_hash(const CacheKey* key) {
    uint64_t hash = 14695981039346656037ULL;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            hash ^= key->class;
            hash *= 1099511628211ULL;
        }
        #pragma omp section
        {
            hash ^= key->id;
            hash *= 1099511628211ULL;
        }
    }
    
    return hash;
}

bool cache_key_equals(const CacheKey* a, const CacheKey* b) {
    return a->class == b->class && a->id == b->id;
}
