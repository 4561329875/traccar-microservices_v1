#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jwt_funciones.h"

// Función simulada para generar un JWT (en una implementación real se usaría OpenSSL)
char* generate_jwt(const char* payload, const char* secret) {
    size_t length = strlen(payload) + strlen(secret) + 10;
    char* jwt = (char*) malloc(length);
    if (jwt) {
        snprintf(jwt, length, "%s.%s", payload, secret);
    }
    return jwt;
}

// Función simulada para verificar un JWT (en una implementación real se decodificaría y validaría la firma)
int verify_jwt(const char* token, const char* secret) {
    return strstr(token, secret) != NULL ? 1 : 0;
}

