#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    long id;
    long deviceId;
} Event;

typedef struct {
    long userId;
} PermissionsService;

typedef struct {
    Event* events;
    int count;
} EventStorage;

Event* get_event_by_id(EventStorage* storage, long id, PermissionsService* permService) {
    Event* found_event = NULL;

    #pragma omp parallel
    {
        #pragma omp critical
        {
            for (int i = 0; i < storage->count; i++) {
                if (storage->events[i].id == id) {
                    found_event = &storage->events[i];
                    break;
                }
            }
        }
    }

    if (found_event == NULL) {
        fprintf(stderr, "Event not found\n");
        return NULL;
    }

    // Simulate permission check
    if (permService->userId != found_event->deviceId) {
        fprintf(stderr, "Permission denied\n");
        return NULL;
    }

    return found_event;
}

int main() {
    EventStorage storage = {0};
    storage.events = malloc(3 * sizeof(Event));
    storage.count = 3;

    storage.events[0] = (Event){1, 100};
    storage.events[1] = (Event){2, 200};
    storage.events[2] = (Event){3, 300};

    PermissionsService permService = {100};

    Event* event = get_event_by_id(&storage, 2, &permService);
    
    free(storage.events);
    return 0;
}