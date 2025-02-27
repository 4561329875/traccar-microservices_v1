#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char name[100];
} Calendar;

typedef struct {
    // Simulación de un servicio de permisos
    int user_id;
} PermissionsService;

typedef struct {
    PermissionsService permissionsService;
} CalendarResource;

CalendarResource* createCalendarResource() {
    CalendarResource* resource = (CalendarResource*)malloc(sizeof(CalendarResource));
    // Inicializar dependencias (simulado)
    return resource;
}

void addCalendar(CalendarResource* resource, Calendar* calendar) {
    // Verificar permisos (simulado)
    printf("Checking permissions for user %d\n", resource->permissionsService.user_id);

    // Simular la adición de un calendario
    printf("Calendar added: %s\n", calendar->name);
}

int main() {
    CalendarResource* resource = createCalendarResource();
    Calendar calendar = {1, "Test Calendar"};
    addCalendar(resource, &calendar);
    free(resource);
    return 0;
}