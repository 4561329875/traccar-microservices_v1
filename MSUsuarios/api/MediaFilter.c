#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

// HTTP status codes
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403

// Session structure
typedef struct {
    char* id;
    void* attributes;
} Session;

// Request structure
typedef struct {
    char* path;
    Session* session;
} Request;

// Response structure
typedef struct {
    int status;
    char* content_type;
    char* body;
} Response;

// Device structure
typedef struct {
    long id;
    char* unique_id;
} Device;

// Statistics manager
typedef struct {
    omp_lock_t lock;
    int* request_counts;
} StatisticsManager;

// Initialize statistics manager
StatisticsManager* create_statistics_manager() {
    StatisticsManager* manager = malloc(sizeof(StatisticsManager));
    omp_init_lock(&manager->lock);
    manager->request_counts = calloc(1000, sizeof(int)); // Assume max 1000 users
    return manager;
}

// Register request in statistics
void register_request(StatisticsManager* manager, long user_id) {
    omp_set_lock(&manager->lock);
    manager->request_counts[user_id]++;
    omp_unset_lock(&manager->lock);
}

// Media filter implementation
void do_filter(Request* request, Response* response, StatisticsManager* manager) {
    // Get user ID from session
    long user_id = 0;
    if (request->session) {
        user_id = (long)request->session->attributes;
    }

    // Check authentication
    if (user_id == 0) {
        response->status = HTTP_UNAUTHORIZED;
        return;
    }

    // Register request in statistics
    #pragma omp task
    {
        register_request(manager, user_id);
    }

    // Parse path and check permissions
    char* path = request->path;
    if (!path) {
        response->status = HTTP_FORBIDDEN;
        return;
    }

    // Split path and process
    char* path_copy = strdup(path);
    char* token = strtok(path_copy, "/");
    
    bool access_granted = false;
    Device* device = NULL;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Get second path component
            token = strtok(NULL, "/");
            if (token) {
                device = find_device_by_unique_id(token);
            }
        }

        #pragma omp section
        {
            if (device) {
                // Check device permissions
                if (check_device_permission(user_id, device->id)) {
                    access_granted = true;
                }
            }
        }
    }

    free(path_copy);

    if (access_granted) {
        response->status = 200; // OK
    } else {
        response->status = HTTP_FORBIDDEN;
    }
}

// Clean up resources
void free_statistics_manager(StatisticsManager* manager) {
    omp_destroy_lock(&manager->lock);
    free(manager->request_counts);
    free(manager);
}