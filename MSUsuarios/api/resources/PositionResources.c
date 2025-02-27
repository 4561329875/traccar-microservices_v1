#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

typedef struct {
    int id;
    int device_id;
    double latitude;
    double longitude;
    time_t fix_time;
} Position;

typedef struct {
    int id;
    char* name;
} Device;

typedef struct {
    int id;
    int is_admin;
} User;

Position* get_positions_by_device(Device* device, time_t from, time_t to, int* count) {
    // Simulate fetching positions
    *count = 5;
    Position* positions = malloc(*count * sizeof(Position));

    #pragma omp parallel for
    for (int i = 0; i < *count; i++) {
        positions[i].id = i + 1;
        positions[i].device_id = device->id;
        positions[i].latitude = 40.0 + (i * 0.1);
        positions[i].longitude = -70.0 + (i * 0.1);
        positions[i].fix_time = from + (i * 3600); // Hourly intervals
    }

    return positions;
}

void export_kml(Position* positions, int count, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    fprintf(file, "<Document>\n");

    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        #pragma omp critical
        {
            fprintf(file, "<Placemark>\n");
            fprintf(file, "<Point>\n");
            fprintf(file, "<coordinates>%f,%f,0</coordinates>\n", 
                    positions[i].longitude, positions[i].latitude);
            fprintf(file, "</Point>\n");
            fprintf(file, "</Placemark>\n");
        }
    }

    fprintf(file, "</Document>\n");
    fprintf(file, "</kml>\n");
    fclose(file);
}

int remove_device_positions(Device* device, time_t from, time_t to) {
    int removed_count = 0;

    #pragma omp parallel for reduction(+:removed_count)
    for (int i = 0; i < 100; i++) {
        // Simulate position removal
        time_t current_time = from + (i * 3600);
        if (current_time >= from && current_time <= to) {
            removed_count++;
        }
    }

    printf("Removed %d positions for device %d\n", removed_count, device->id);
    return removed_count;
}

int main() {
    Device device = {1, "Tracker001"};
    User user = {1, 1};  // Admin user

    time_t from = time(NULL) - (30 * 24 * 3600);  // 30 days ago
    time_t to = time(NULL);

    int positions_count;
    Position* positions = get_positions_by_device(&device, from, to, &positions_count);

    // Export to KML
    export_kml(positions, positions_count, "device_positions.kml");

    // Remove positions
    remove_device_positions(&device, from, to);

    free(positions);
    return 0;
}