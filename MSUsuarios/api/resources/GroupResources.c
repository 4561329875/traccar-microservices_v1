#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    long id;
    char* name;
} Group;

typedef struct {
    Group* groups;
    int count;
} GroupResource;

typedef struct {
    long userId;
} PermissionsService;

GroupResource* create_group_resource() {
    GroupResource* resource = malloc(sizeof(GroupResource));
    resource->groups = NULL;
    resource->count = 0;
    return resource;
}

int add_group(GroupResource* resource, Group* group) {
    #pragma omp critical
    {
        resource->groups = realloc(
            resource->groups, 
            (resource->count + 1) * sizeof(Group)
        );
        resource->groups[resource->count] = *group;
        resource->count++;
    }
    return resource->count;
}

int main() {
    GroupResource* resource = create_group_resource();
    
    Group group1 = {1, "Sales Team"};
    Group group2 = {2, "Engineering Department"};
    
    add_group(resource, &group1);
    add_group(resource, &group2);
    
    // Cleanup
    for (int i = 0; i < resource->count; i++) {
        free(resource->groups[i].name);
    }
    free(resource->groups);
    free(resource);
    
    return 0;
}