// async_socket.c
#include "async_socket.h"
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#define LWS_SEND_BUFFER_PRE_PADDING 16
#define LWS_SEND_BUFFER_POST_PADDING 16

static void send_data(AsyncSocket *socket, json_t *data) {
    if (!socket->wsi) return;
    
    char *json_str = json_dumps(data, JSON_COMPACT);
    if (!json_str) return;
    
    size_t len = strlen(json_str);
    unsigned char *buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING);
    memcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], json_str, len);
    
    #pragma omp critical
    {
        lws_write(socket->wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);
    }
    
    free(buf);
    free(json_str);
}

static void on_connect(AsyncSocket *socket) {
    json_t *data = json_object();
    json_t *positions = json_array();
    
    // Get latest positions
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Fetch positions from storage
            Position *pos_list = storage_get_latest_positions(socket->storage, socket->user_id);
            if (pos_list) {
                for (int i = 0; pos_list[i].id != 0; i++) {
                    json_array_append_new(positions, position_to_json(&pos_list[i]));
                }
                free(pos_list);
            }
        }
    }
    
    json_object_set_new(data, "positions", positions);
    send_data(socket, data);
    json_decref(data);
    
    connection_manager_add_listener(socket->connection_manager, socket->user_id, socket);
}

static void on_close(AsyncSocket *socket) {
    connection_manager_remove_listener(socket->connection_manager, socket->user_id, socket);
}

static void on_message(AsyncSocket *socket, const char *message) {
    json_error_t error;
    json_t *root = json_loads(message, 0, &error);
    if (!root) return;
    
    json_t *logs = json_object_get(root, "logs");
    if (logs) {
        socket->include_logs = json_is_true(logs);
    }
    
    json_decref(root);
}

AsyncSocket* async_socket_create(json_t *object_mapper, 
                               ConnectionManager *connection_manager,
                               Storage *storage, 
                               long user_id) {
    AsyncSocket *socket = malloc(sizeof(AsyncSocket));
    if (!socket) return NULL;
    
    socket->wsi = NULL;
    socket->object_mapper = object_mapper;
    socket->connection_manager = connection_manager;
    socket->storage = storage;
    socket->user_id = user_id;
    socket->include_logs = false;
    
    return socket;
}

void async_socket_destroy(AsyncSocket *socket) {
    if (socket) {
        free(socket);
    }
}

int async_socket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    AsyncSocket *socket = (AsyncSocket *)user;
    
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            socket->wsi = wsi;
            on_connect(socket);
            break;
            
        case LWS_CALLBACK_CLOSED:
            on_close(socket);
            socket->wsi = NULL;
            break;
            
        case LWS_CALLBACK_RECEIVE:
            if (in && len) {
                char *message = malloc(len + 1);
                memcpy(message, in, len);
                message[len] = '\0';
                on_message(socket, message);
                free(message);
            }
            break;
    }
    
    return 0;
}