// async_socket_servlet.h
#ifndef ASYNC_SOCKET_SERVLET_H
#define ASYNC_SOCKET_SERVLET_H

#include <libwebsockets.h>
#include <jansson.h>
#include "connection_manager.h"
#include "storage.h"
#include "login_service.h"
#include "config.h"

typedef struct {
    Config *config;
    json_t *object_mapper;
    ConnectionManager *connection_manager;
    Storage *storage;
    LoginService *login_service;
} AsyncSocketServlet;

AsyncSocketServlet* async_socket_servlet_create(Config *config,
                                              json_t *object_mapper,
                                              ConnectionManager *connection_manager,
                                              Storage *storage,
                                              LoginService *login_service);
void async_socket_servlet_destroy(AsyncSocketServlet *servlet);
struct lws_protocols* async_socket_servlet_get_protocols(AsyncSocketServlet *servlet);

#endif // ASYNC_SOCKET_SERVLET_H