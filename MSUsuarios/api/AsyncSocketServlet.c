// async_socket_servlet.c
#include "async_socket_servlet.h"
#include "async_socket.h"
#include <string.h>
#include <stdlib.h>
#include <omp.h>

static int callback(struct lws *wsi, enum lws_callback_reasons reason,
                   void *user, void *in, size_t len) {
    AsyncSocket *socket = (AsyncSocket *)user;
    return async_socket_callback(wsi, reason, user, in, len);
}

static struct lws_protocols protocols[] = {
    {
        "traccar-protocol",
        callback,
        sizeof(AsyncSocket),
        4096,
    },
    { NULL, NULL, 0, 0 }
};

AsyncSocketServlet* async_socket_servlet_create(Config *config,
                                              json_t *object_mapper,
                                              ConnectionManager *connection_manager,
                                              Storage *storage,
                                              LoginService *login_service) {
    AsyncSocketServlet *servlet = malloc(sizeof(AsyncSocketServlet));
    if (!servlet) return NULL;
    
    servlet->config = config;
    servlet->object_mapper = object_mapper;
    servlet->connection_manager = connection_manager;
    servlet->storage = storage;
    servlet->login_service = login_service;
    
    return servlet;
}

void async_socket_servlet_destroy(AsyncSocketServlet *servlet) {
    if (servlet) {
        free(servlet);
    }
}

struct lws_protocols* async_socket_servlet_get_protocols(AsyncSocketServlet *servlet) {
    return protocols;
}

// Función auxiliar para validar token
static long validate_token(AsyncSocketServlet *servlet, const char *token) {
    if (!token) return 0;
    
    long user_id = 0;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Validate token and get user ID
            user_id = login_service_validate_token(servlet->login_service, token);
        }
    }
    
    return user_id;
}