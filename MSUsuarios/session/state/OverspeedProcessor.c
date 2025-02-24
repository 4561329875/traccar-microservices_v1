#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include "overspeed_state.h"
#include "position.h"
#include "event.h"

#define ATTRIBUTE_SPEED "speed"
#define KEY_SPEED_LIMIT "speedLimit"

static void check_event(OverspeedState* state, Position* position, double speed_limit, long minimal_duration) {
    if (state->overspeed_time != NULL) {
        time_t old_time = state->overspeed_time->time;
        time_t new_time = position->fix_time.time;
        
        if (new_time - old_time >= minimal_duration) {
            Event* event = create_event("deviceOverspeed", position);
            event_set_double(event, ATTRIBUTE_SPEED, position->speed);
            event_set_double(event, KEY_SPEED_LIMIT, speed_limit);
            event_set_geofence_id(event, state->overspeed_geofence_id);
            
            state->overspeed_time = NULL;
            state->overspeed_geofence_id = 0;
            state->event = event;
        }
    }
}

void update_overspeed_state(
    OverspeedState* state,
    Position* position,
    double speed_limit,
    double multiplier,
    long minimal_duration,
    long geofence_id) {
    
    state->event = NULL;
    
    #pragma omp parallel sections shared(state, position)
    {
        #pragma omp section
        {
            bool old_state = state->overspeed_state;
            if (old_state) {
                bool new_state = position->speed > speed_limit * multiplier;
                if (new_state) {
                    check_event(state, position, speed_limit, minimal_duration);
                } else {
                    state->overspeed_state = false;
                    state->overspeed_time = NULL;
                    state->overspeed_geofence_id = 0;
                }
            } else if (position != NULL && position->speed > speed_limit * multiplier) {
                state->overspeed_state = true;
                state->overspeed_time = &position->fix_time;
                state->overspeed_geofence_id = geofence_id;
                
                check_event(state, position, speed_limit, minimal_duration);
            }
        }
    }
}