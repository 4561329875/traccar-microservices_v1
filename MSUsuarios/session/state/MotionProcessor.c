#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include "motion_state.h"
#include "position.h"
#include "trips_config.h"
#include "event.h"

void update_motion_state(
    MotionState* state, 
    Position* position, 
    bool new_state, 
    TripsConfig* trips_config) {
    
    state->event = NULL;
    bool old_state = state->motion_state;

    #pragma omp parallel sections shared(state, position)
    {
        #pragma omp section
        {
            if (old_state == new_state) {
                if (state->motion_time != NULL) {
                    time_t old_time = state->motion_time->time;
                    time_t new_time = position->fix_time.time;
                    
                    double distance = position->total_distance - state->motion_distance;
                    bool ignition = false;
                    bool has_ignition = false;
                    
                    if (trips_config->use_ignition && position_has_attribute(position, "ignition")) {
                        has_ignition = true;
                        ignition = position_get_boolean(position, "ignition");
                    }
                    
                    bool generate_event = false;
                    if (new_state) {
                        if ((new_time - old_time) >= trips_config->minimal_trip_duration ||
                            distance >= trips_config->minimal_trip_distance) {
                            generate_event = true;
                        }
                    } else {
                        if ((new_time - old_time) >= trips_config->minimal_parking_duration ||
                            (has_ignition && !ignition)) {
                            generate_event = true;
                        }
                    }
                    
                    if (generate_event) {
                        const char* event_type = new_state ? "deviceMoving" : "deviceStopped";
                        Event* event = create_event(event_type, position);
                        
                        state->motion_streak = new_state;
                        state->motion_time = NULL;
                        state->motion_distance = 0;
                        state->event = event;
                    }
                }
            } else {
                state->motion_state = new_state;
                if (state->motion_streak == new_state) {
                    state->motion_time = NULL;
                    state->motion_distance = 0;
                } else {
                    state->motion_time = &position->fix_time;
                    state->motion_distance = position->total_distance;
                }
            }
        }
    }
}