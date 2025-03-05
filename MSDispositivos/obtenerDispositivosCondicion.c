#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>
#include "ConversorJSONDispositivos.h"




char* consultarDispositivosCon(PGconn *conn, const char *condicion) {
    const char *plantilla ="SELECT d.id,d.attributes,d.groupid,d.calendarid,d.name,d.uniqueid,d.status,d.lastupdate ,d.positionid,d.phone, d.model,d.contact,d.category, d.disabled,d.expirationtime  FROM tc_devices d, tc_user_device u WHERE d.id=u.deviceid AND ";
    size_t query_len = strlen(plantilla) + strlen(condicion) + 1;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Inicializa query con la cadena básica
    strcpy(query, plantilla);

    // Concatenar la condición de forma segura
    strncat(query, condicion, query_len - strlen(query) - 1);

    PGresult *res = PQexec(conn, query);

    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    // Obtener el número de filas y columnas
    int rows = PQntuples(res);
    int cols = PQnfields(res);


    const char *nomObjJson[] = {
        "id",
        "attributes",
        "groupId", // Corregido 'grupId' a 'groupId'
        "calendarId",
        "name",
        "uniqueId",
        "status",
        "lastUpdate",
        "positionId",
        "phone",
        "model",
        "contact",
        "category",
        "disabled",
        "expirationTime"
    };


    
    char *json = generarJSONDispositivo(res, rows, cols, nomObjJson);
    PQclear(res); // Liberar la memoria utilizada por el resultado

    return json; // Devolver el JSON generado
}
