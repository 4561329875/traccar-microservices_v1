#include "DeviceSession.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define KEY_TIMEZONE "timezone"

struct sockaddr_storage* get_channel_address(struct Channel* channel) {
    return NULL;
}

ConnectionKey* connection_key_create(struct sockaddr_storage* local_addr, 
                                     const struct sockaddr_storage* remote_addr) {
    ConnectionKey* key = malloc(sizeof(ConnectionKey));
    if (key) {
        if (local_addr) {
            memcpy(&key->local_addr, local_addr, sizeof(struct sockaddr_storage));
        } else {
            memset(&key->local_addr, 0, sizeof(struct sockaddr_storage));
        }
        memcpy(&key->remote_addr, remote_addr, sizeof(struct sockaddr_storage));
    }
    return key;
}

bool has_http_decoder(struct Channel* channel) {
    return false;
}

void protocol_send_command(struct Protocol* protocol, struct Channel* channel, 
                           struct sockaddr_storage* remote_addr, const struct Command* command) {
}

DeviceSession* device_session_create(
        long device_id, const char* unique_id, const char* model,
        struct Protocol* protocol, struct Channel* channel, 
        const struct sockaddr_storage* remote_addr) {
    
    DeviceSession* session = malloc(sizeof(DeviceSession));
    session->device_id = device_id;
    session->unique_id = strdup(unique_id);
    session->model = model ? strdup(model) : NULL;
    session->protocol = protocol;
    session->channel = channel;
    memcpy(&session->remote_address, remote_addr, sizeof(struct sockaddr_storage));
    
    session->locals_capacity = 10;
    session->locals_count = 0;
    session->locals = malloc(sizeof(*session->locals) * session->locals_capacity);
    
    omp_init_lock(&session->locals_lock);
    
    return session;
}

long device_session_get_device_id(const DeviceSession* session) {
    return session->device_id;
}

const char* device_session_get_unique_id(const DeviceSession* session) {
    return session->unique_id;
}

ConnectionKey* device_session_get_connection_key(const DeviceSession* session) {
    struct sockaddr_storage* channel_addr = get_channel_address(session->channel);
    return connection_key_create(channel_addr, &session->remote_address);
}

bool device_session_supports_live_commands(const DeviceSession* session) {
    return !has_http_decoder(session->channel);
}

void device_session_send_command(DeviceSession* session, const struct Command* command) {
    protocol_send_command(session->protocol, session->channel, 
                         &session->remote_address, command);
}

bool device_session_contains(DeviceSession* session, const char* key) {
    bool found = false;
    
    omp_set_lock(&session->locals_lock);
    for (size_t i = 0; i < session->locals_count; i++) {
        if (strcmp(session->locals[i].key, key) == 0) {
            found = true;
            break;
        }
    }
    omp_unset_lock(&session->locals_lock);
    
    return found;
}

void device_session_set(DeviceSession* session, const char* key, 
                       const void* value, size_t value_size) {
    omp_set_lock(&session->locals_lock);
    
    size_t index = session->locals_count;
    for (size_t i = 0; i < session->locals_count; i++) {
        if (strcmp(session->locals[i].key, key) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == session->locals_count) {
        if (session->locals_count == session->locals_capacity) {
            session->locals_capacity *= 2;
            session->locals = realloc(session->locals, 
                sizeof(*session->locals) * session->locals_capacity);
        }
        session->locals_count++;
    }
    
    if (value) {
        session->locals[index].key = strdup(key);
        session->locals[index].value = malloc(value_size);
        memcpy(session->locals[index].value, value, value_size);
        session->locals[index].value_size = value_size;
    } else {
        free(session->locals[index].key);
        free(session->locals[index].value);
        memmove(&session->locals[index], &session->locals[index + 1],
                (session->locals_count - index - 1) * sizeof(*session->locals));
        session->locals_count--;
    }
    
    omp_unset_lock(&session->locals_lock);
}

void* device_session_get(DeviceSession* session, const char* key) {
    void* result = NULL;
    
    omp_set_lock(&session->locals_lock);
    for (size_t i = 0; i < session->locals_count; i++) {
        if (strcmp(session->locals[i].key, key) == 0) {
            result = session->locals[i].value;
            break;
        }
    }
    omp_unset_lock(&session->locals_lock);
    
    return result;
}

void device_session_free(DeviceSession* session) {
    if (!session) return;
    
    free(session->unique_id);
    free(session->model);
    
    for (size_t i = 0; i < session->locals_count; i++) {
        free(session->locals[i].key);
        free(session->locals[i].value);
    }
    free(session->locals);
    
    omp_destroy_lock(&session->locals_lock);
    free(session);
}

int main() {
    struct sockaddr_storage remote_addr;
    struct sockaddr_in* addr_in = (struct sockaddr_in*)&remote_addr;
    addr_in->sin_family = AF_INET;
    addr_in->sin_port = htons(8080);
    inet_pton(AF_INET, "192.168.1.100", &(addr_in->sin_addr));

    struct Protocol* dummy_protocol = NULL;
    struct Channel* dummy_channel = NULL;

    DeviceSession* session = device_session_create(
        12345,                  
        "unique_device_123",    
        "TestModel",            
        dummy_protocol,         
        dummy_channel,          
        &remote_addr            
    );

    int timezone = -5;
    device_session_set(session, KEY_TIMEZONE, &timezone, sizeof(int));

    printf("Device ID: %ld\n", device_session_get_device_id(session));
    printf("Unique ID: %s\n", device_session_get_unique_id(session));

    int* stored_timezone = (int*)device_session_get(session, KEY_TIMEZONE);
    if (stored_timezone) {
        printf("Stored Timezone: %d\n", *stored_timezone);
    }

    device_session_free(session);

    return 0;
}
