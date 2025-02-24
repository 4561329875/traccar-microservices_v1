#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

// Connection key structure
typedef struct {
    struct sockaddr_storage local_address;
    struct sockaddr_storage remote_address;
} ConnectionKey;

// Create connection key from addresses
ConnectionKey* connection_key_create(
        const struct sockaddr_storage* local_addr,
        const struct sockaddr_storage* remote_addr) {
    ConnectionKey* key = malloc(sizeof(ConnectionKey));
    memcpy(&key->local_address, local_addr, sizeof(struct sockaddr_storage));
    memcpy(&key->remote_address, remote_addr, sizeof(struct sockaddr_storage));
    return key;
}

// Create connection key from channel
ConnectionKey* connection_key_create_from_channel(
        const struct sockaddr_storage* channel_addr,
        const struct sockaddr_storage* remote_addr) {
    return connection_key_create(channel_addr, remote_addr);
}

// Compare two connection keys
int connection_key_equals(const ConnectionKey* key1, const ConnectionKey* key2) {
    return memcmp(&key1->local_address, &key2->local_address, 
                 sizeof(struct sockaddr_storage)) == 0 &&
           memcmp(&key1->remote_address, &key2->remote_address, 
                 sizeof(struct sockaddr_storage)) == 0;
}

// Hash function for connection key
size_t connection_key_hash(const ConnectionKey* key) {
    size_t hash = 0;
    const unsigned char* bytes = (const unsigned char*)key;
    
    #pragma omp parallel for reduction(^:hash)
    for (size_t i = 0; i < sizeof(ConnectionKey); i++) {
        hash ^= ((size_t)bytes[i] << ((i % 8) * 8));
    }
    
    return hash;
}

// Free connection key
void connection_key_free(ConnectionKey* key) {
    free(key);
}

int main() {
    // Crear direcciones de ejemplo
    struct sockaddr_storage local_addr = {0};
    struct sockaddr_storage remote_addr = {0};

    // Simular configuración de direcciones
    struct sockaddr_in* local_in = (struct sockaddr_in*)&local_addr;
    struct sockaddr_in* remote_in = (struct sockaddr_in*)&remote_addr;

    local_in->sin_family = AF_INET;
    local_in->sin_port = htons(8080);
    local_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    remote_in->sin_family = AF_INET;
    remote_in->sin_port = htons(9090);
    remote_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Crear connection keys
    ConnectionKey* key1 = connection_key_create(&local_addr, &remote_addr);
    ConnectionKey* key2 = connection_key_create(&local_addr, &remote_addr);
    ConnectionKey* key3 = connection_key_create_from_channel(&local_addr, &remote_addr);

    // Probar igualdad
    printf("key1 == key2: %d\n", connection_key_equals(key1, key2));
    
    // Calcular y mostrar hash
    printf("Hash de key1: %zu\n", connection_key_hash(key1));

    // Liberar memoria
    connection_key_free(key1);
    connection_key_free(key2);
    connection_key_free(key3);

    return 0;
}
