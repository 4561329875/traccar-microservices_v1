#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

typedef struct {
    int id;
    char* description;
} Report;

typedef struct {
    int device_id;
    double total_distance;
    double total_duration;
    time_t start_time;
    time_t end_time;
} SummaryReportItem;

typedef struct {
    time_t start_time;
    time_t end_time;
    int duration;
    double distance;
} TripReportItem;

typedef struct {
    time_t start_time;
    time_t end_time;
    int duration;
} StopReportItem;

SummaryReportItem* generate_summary_report(int* device_ids, int device_count, time_t from, time_t to, int* report_count) {
    *report_count = device_count;
    SummaryReportItem* summary_items = malloc(*report_count * sizeof(SummaryReportItem));

    #pragma omp parallel for
    for (int i = 0; i < *report_count; i++) {
        summary_items[i].device_id = device_ids[i];
        summary_items[i].total_distance = 500.0 * (i + 1);
        summary_items[i].total_duration = 3600 * (i + 1);
        summary_items[i].start_time = from;
        summary_items[i].end_time = to;
    }

    return summary_items;
}

TripReportItem* generate_trips_report(int* device_ids, int device_count, time_t from, time_t to, int* report_count) {
    *report_count = device_count * 3;  // Multiple trips per device
    TripReportItem* trip_items = malloc(*report_count * sizeof(TripReportItem));

    #pragma omp parallel for
    for (int i = 0; i < *report_count; i++) {
        trip_items[i].start_time = from + (i * 7200);  // 2-hour intervals
        trip_items[i].end_time = trip_items[i].start_time + 3600;
        trip_items[i].duration = 3600;
        trip_items[i].distance = 100.0 * (i + 1);
    }

    return trip_items;
}

StopReportItem* generate_stops_report(int* device_ids, int device_count, time_t from, time_t to, int* report_count) {
    *report_count = device_count * 2;  // Multiple stops per device
    StopReportItem* stop_items = malloc(*report_count * sizeof(StopReportItem));

    #pragma omp parallel for
    for (int i = 0; i < *report_count; i++) {
        stop_items[i].start_time = from + (i * 10800);  // 3-hour intervals
        stop_items[i].end_time = stop_items[i].start_time + 1800;
        stop_items[i].duration = 1800;
    }

    return stop_items;
}

void export_summary_to_excel(SummaryReportItem* items, int count, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "Device ID,Total Distance,Total Duration\n");

    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        #pragma omp critical
        {
            fprintf(file, "%d,%.2f,%ld\n", 
                    items[i].device_id, 
                    items[i].total_distance, 
                    (long)items[i].total_duration);
        }
    }

    fclose(file);
}

int main() {
    int device_ids[] = {1, 2, 3, 4, 5};
    int device_count = sizeof(device_ids) / sizeof(device_ids[0]);

    time_t from = time(NULL) - (30 * 24 * 3600);  // 30 days ago
    time_t to = time(NULL);

    int summary_count, trips_count, stops_count;
    
    SummaryReportItem* summary_report = generate_summary_report(device_ids, device_count, from, to, &summary_count);
    TripReportItem* trips_report = generate_trips_report(device_ids, device_count, from, to, &trips_count);
    StopReportItem* stops_report = generate_stops_report(device_ids, device_count, from, to, &stops_count);

    export_summary_to_excel(summary_report, summary_count, "summary_report.csv");

    // Cleanup
    free(summary_report);
    free(trips_report);
    free(stops_report);

    return 0;
}