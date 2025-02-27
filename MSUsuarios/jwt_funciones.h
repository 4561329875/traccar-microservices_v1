#ifndef JWT_FUNCTIONS_H
#define JWT_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Función para generar un JWT
char* generate_jwt(const char* payload, const char* secret);

// Función para verificar un JWT
int verify_jwt(const char* token, const char* secret);

#ifdef __cplusplus
}
#endif

#endif

