#ifndef MOTION_STATE_H
#define MOTION_STATE_H

#include <stdbool.h>
#include <time.h>
#include "device.h"
#include "event.h"

typedef struct {
    bool changed;
    bool motion_streak;
    bool motion_state;
    struct tm* motion_time;
    double motion_distance;
    Event* event;
} MotionState;

// Function prototypes
MotionState* motion_state_from_device(Device* device);
void motion_state_to_device(MotionState* state, Device* device);
void set_motion_streak(MotionState* state, bool motion_streak);
void set_motion_state(MotionState* state, bool motion_state);
void set_motion_time(MotionState* state, struct tm* motion_time);
void set_motion_distance(MotionState* state, double motion_distance);
void set_motion_event(MotionState* state, Event* event);

#endif