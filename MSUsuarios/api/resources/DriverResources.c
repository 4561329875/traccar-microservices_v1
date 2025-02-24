#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    long id;
    char* name;
} Driver;

typedef struct {
    Driver* drivers;
    int count;
} DriverResource;

typedef struct {
    long userId;
} PermissionsService;

DriverResource* create_driver_resource() {
    DriverResource* resource = malloc(sizeof(DriverResource));
    resource->drivers = NULL;
    resource->count = 0;
    return resource;
}

int add_driver(DriverResource* resource, Driver* driver) {
    #pragma omp critical
    {
        resource->drivers = realloc(
            resource->drivers, 
            (resource->count + 1) * sizeof(Driver)
        );
        resource->drivers[resource->count] = *driver;
        resource->count++;
    }
    return resource->count;
}

int main() {
    DriverResource* resource = create_driver_resource();
    
    Driver driver1 = {1, "John Doe"};
    Driver driver2 = {2, "Jane Smith"};
    
    add_driver(resource, &driver1);
    add_driver(resource, &driver2);
    
    // Cleanup
    for (int i = 0; i < resource->count; i++) {
        free(resource->drivers[i].name);
    }
    free(resource->drivers);
    free(resource);
    
    return 0;
}