#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int user_id;
    char message[256];
    char sent_at[20];
} Notification;

typedef struct {
    int id;
    int device_id;
    char command_text[256];
    char executed_at[20];
} Command;

typedef struct {
    int id;
    char type[50];
    char description[256];
} Attribute;

typedef struct {
    int device_id;
    char protocol[50];
} Position;

typedef struct {
    int user_id;
} PermissionsService;

typedef struct {
    Position* (*getObject)(int device_id);
} Storage;

typedef struct {
    void (*addDevice)(int device_id, void* key);
    void (*removeDevice)(int device_id, void* key);
} CacheManager;

typedef struct {
    void* (*computeAttribute)(Attribute* attribute, Position* position);
} ComputedAttributesHandler;

typedef struct {
    PermissionsService permissionsService;
    Storage storage;
    CacheManager cacheManager;
    ComputedAttributesHandler computedAttributesHandler;
} AttributeResource;

AttributeResource* createAttributeResource() {
    AttributeResource* resource = (AttributeResource*)malloc(sizeof(AttributeResource));
    // Inicializar dependencias (simulado)
    return resource;
}

void testAttribute(AttributeResource* resource, int device_id, Attribute* attribute) {
    // Verificar permisos (simulado)
    printf("Checking permissions for user %d\n", resource->permissionsService.user_id);

    // Obtener la última posición del dispositivo
    Position* position = resource->storage.getObject(device_id);

    // Simular el cálculo del atributo
    void* key = malloc(1); // Clave simulada
    resource->cacheManager.addDevice(position->device_id, key);
    void* result = resource->computedAttributesHandler.computeAttribute(attribute, position);
    if (result != NULL) {
        printf("Result: %s\n", (char*)result);
    } else {
        printf("No content\n");
    }
    resource->cacheManager.removeDevice(position->device_id, key);
    free(key);
}

int main() {
    AttributeResource* resource = createAttributeResource();
    Attribute attribute = {1, "number", "Test attribute"};
    testAttribute(resource, 123, &attribute);
    free(resource);
    return 0;
}