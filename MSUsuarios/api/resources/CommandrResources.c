#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int device_id;
    char type[50];
    char description[256];
} Command;

typedef struct {
    int id;
    char protocol[50];
} Position;

typedef struct {
    int user_id;
} PermissionsService;

typedef struct {
    Position* (*getObject)(int device_id);
} Storage;

typedef struct {
    void (*sendCommand)(Command* command);
} CommandsManager;

typedef struct {
    PermissionsService permissionsService;
    Storage storage;
    CommandsManager commandsManager;
} CommandResource;

CommandResource* createCommandResource() {
    CommandResource* resource = (CommandResource*)malloc(sizeof(CommandResource));
    // Inicializar dependencias (simulado)
    return resource;
}

void sendCommand(CommandResource* resource, Command* command, int group_id) {
    // Verificar permisos (simulado)
    printf("Checking permissions for user %d\n", resource->permissionsService.user_id);

    // Simular el envío de un comando
    if (group_id > 0) {
        printf("Sending command to group %d\n", group_id);
    } else {
        printf("Sending command to device %d\n", command->device_id);
    }
    resource->commandsManager.sendCommand(command);
}

int main() {
    CommandResource* resource = createCommandResource();
    Command command = {1, 123, "TYPE_CUSTOM", "Test Command"};
    sendCommand(resource, &command, 0);
    free(resource);
    return 0;
}