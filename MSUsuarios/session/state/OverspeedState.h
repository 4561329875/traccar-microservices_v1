#ifndef OVERSPEED_STATE_H
#define OVERSPEED_STATE_H

#include <stdbool.h>
#include <time.h>
#include "device.h"
#include "event.h"

typedef struct {
    bool changed;
    bool overspeed_state;
    struct tm* overspeed_time;
    long overspeed_geofence_id;
    Event* event;
} OverspeedState;

// Function prototypes
OverspeedState* overspeed_state_from_device(Device* device);
void overspeed_state_to_device(OverspeedState* state, Device* device);
void set_overspeed_state(OverspeedState* state, bool overspeed_state);
void set_overspeed_time(OverspeedState* state, struct tm* overspeed_time);
void set_overspeed_geofence_id(OverspeedState* state, long overspeed_geofence_id);
void set_overspeed_event(OverspeedState* state, Event* event);

#endif