#ifndef WEAK_VALUE_MAP_H
#define WEAK_VALUE_MAP_H

#include <stddef.h>

typedef struct WeakValueMap WeakValueMap;
typedef void* Key;
typedef void* Value;

// Function pointer type for custom key comparison
typedef int (*KeyCompareFunc)(const Key, const Key);
// Function pointer type for custom key hashing
typedef size_t (*KeyHashFunc)(const Key);

// Create a new weak value map
WeakValueMap* weak_map_create(KeyCompareFunc compare_func, KeyHashFunc hash_func);

// Destroy a weak value map
void weak_map_destroy(WeakValueMap* map);

// Put a value in the map
void weak_map_put(WeakValueMap* map, Key key, Value value);

// Get a value from the map
Value weak_map_get(WeakValueMap* map, Key key);

// Remove a value from the map
Value weak_map_remove(WeakValueMap* map, Key key);

// Clean up any expired weak references
void weak_map_clean(WeakValueMap* map);

#endif