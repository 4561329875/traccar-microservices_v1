#ifndef DEVICE_SESSION_H
#define DEVICE_SESSION_H

#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <omp.h>

struct Protocol;
struct Channel;
struct Command;

typedef struct {
    struct sockaddr_storage local_addr;
    struct sockaddr_storage remote_addr;
} ConnectionKey;

typedef struct DeviceSession {
    long device_id;
    char* unique_id;
    char* model;
    struct Protocol* protocol;
    struct Channel* channel;
    struct sockaddr_storage remote_address;
    
    struct {
        char* key;
        void* value;
        size_t value_size;
    } *locals;
    size_t locals_count;
    size_t locals_capacity;
    
    omp_lock_t locals_lock;
} DeviceSession;

DeviceSession* device_session_create(
    long device_id, const char* unique_id, const char* model,
    struct Protocol* protocol, struct Channel* channel, 
    const struct sockaddr_storage* remote_addr);

long device_session_get_device_id(const DeviceSession* session);
const char* device_session_get_unique_id(const DeviceSession* session);
ConnectionKey* device_session_get_connection_key(const DeviceSession* session);
bool device_session_supports_live_commands(const DeviceSession* session);
void device_session_send_command(DeviceSession* session, const struct Command* command);
bool device_session_contains(DeviceSession* session, const char* key);
void device_session_set(DeviceSession* session, const char* key, 
                        const void* value, size_t value_size);
void* device_session_get(DeviceSession* session, const char* key);
void device_session_free(DeviceSession* session);

#endif
