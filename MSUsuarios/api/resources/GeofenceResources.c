#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    long id;
    char* name;
    double latitude;
    double longitude;
    double radius;
} Geofence;

typedef struct {
    Geofence* geofences;
    int count;
} GeofenceResource;

typedef struct {
    long userId;
} PermissionsService;

GeofenceResource* create_geofence_resource() {
    GeofenceResource* resource = malloc(sizeof(GeofenceResource));
    resource->geofences = NULL;
    resource->count = 0;
    return resource;
}

int add_geofence(GeofenceResource* resource, Geofence* geofence) {
    #pragma omp critical
    {
        resource->geofences = realloc(
            resource->geofences, 
            (resource->count + 1) * sizeof(Geofence)
        );
        resource->geofences[resource->count] = *geofence;
        resource->count++;
    }
    return resource->count;
}

int main() {
    GeofenceResource* resource = create_geofence_resource();
    
    Geofence fence1 = {1, "Office Area", 37.7749, -122.4194, 500.0};
    Geofence fence2 = {2, "Warehouse Zone", 37.3382, -121.8863, 1000.0};
    
    add_geofence(resource, &fence1);
    add_geofence(resource, &fence2);
    
    // Cleanup
    for (int i = 0; i < resource->count; i++) {
        free(resource->geofences[i].name);
    }
    free(resource->geofences);
    free(resource);
    
    return 0;
}