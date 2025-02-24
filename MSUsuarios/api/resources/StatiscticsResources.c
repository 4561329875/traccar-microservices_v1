#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openmp.h>

typedef struct {
    time_t captureTime;
    // Add other relevant statistics fields
} Statistics;

typedef struct {
    Statistics* stats;
    int statsCount;
} Storage;

typedef struct {
    long userId;
    int isAdmin;
} PermissionsService;

Statistics* get_statistics(
    Storage* storage, 
    PermissionsService* permissions, 
    time_t from, 
    time_t to, 
    int* result_count
) {
    // Check admin permissions
    if (!permissions->isAdmin) {
        *result_count = 0;
        return NULL;
    }

    Statistics* results = NULL;
    *result_count = 0;

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < storage->statsCount; i++) {
            if (storage->stats[i].captureTime >= from && 
                storage->stats[i].captureTime <= to) {
                #pragma omp critical
                {
                    results = realloc(results, (*result_count + 1) * sizeof(Statistics));
                    results[*result_count] = storage->stats[i];
                    (*result_count)++;
                }
            }
        }
    }

    return results;
}