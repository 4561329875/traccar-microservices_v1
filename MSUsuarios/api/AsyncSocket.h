// async_socket.h
#ifndef ASYNC_SOCKET_H
#define ASYNC_SOCKET_H

#include <libwebsockets.h>
#include <jansson.h>
#include "connection_manager.h"
#include "storage.h"

typedef struct {
    struct lws *wsi;
    json_t *object_mapper;
    ConnectionManager *connection_manager;
    Storage *storage;
    long user_id;
    bool include_logs;
} AsyncSocket;

AsyncSocket* async_socket_create(json_t *object_mapper, 
                               ConnectionManager *connection_manager,
                               Storage *storage, 
                               long user_id);
void async_socket_destroy(AsyncSocket *socket);
int async_socket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len);

#endif // ASYNC_SOCKET_H