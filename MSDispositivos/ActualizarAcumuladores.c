
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>
#include "ActualizarDispositivo.h"
#define BUFFER_SIZE 10000000000

static void trim_end(char *str) {
    int length = strlen(str);

    // Elimina los espacios en blanco al final
    while (length > 0 && str[length - 1] == ' ') {
        str[length - 1] = '\0';  // Reemplaza el espacio con el terminador de cadena
        length--;
    }
}




char* conUniAcumuladores(PGconn *conn,char *id) {
    const char *plantilla = "SELECT attributes FROM tc_positions p  WHERE p.deviceid=";
    const char *limit = " ORDER BY p.id DESC LIMIT 1 ";
    size_t query_len = strlen(plantilla) + strlen(id) + strlen(limit) + 3;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Construye la consulta
    strcpy(query, plantilla);
    strncat(query, id, query_len - strlen(query) - 1);
    strncat(query, limit, query_len - strlen(query) - 1);

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


    if (rows == 0) {
        fprintf(stderr, "No se encontraron resultados para el dispositivo.\n");
        PQclear(res);
        return NULL;
    }

    char *json = strdup(PQgetvalue(res, 0, 0));  // Obtener el valor como JSON

    // Limpiar los resultados de la consulta
    PQclear(res);

    return json;
}

int ActualizarAcumuladores(PGconn *conn,char *id, char * atributos) {
    const char *plantilla1 = "INSERT INTO tc_positions (attributes, longitude, altitude, speed, course, accuracy, deviceid, servertime, devicetime, fixtime, valid, latitude, protocol, geofenceids, network, address) (SELECT ";
    const char *plantilla2 = " , longitude, altitude, speed, course, accuracy, deviceid, servertime, devicetime, fixtime, valid, latitude, protocol, geofenceids, network, address FROM tc_positions p WHERE p.deviceid=";
    const char *limit = " ORDER BY p.id DESC LIMIT 1 ) RETURNING id";
    size_t query_len = strlen(plantilla1) + strlen(atributos) + strlen(plantilla2) + strlen(id) + strlen(limit) + 3;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    snprintf(query, query_len, "%s%s%s%s%s",
            plantilla1,
            atributos,
            plantilla2,
            id,
            limit);

    PGresult *res = PQexec(conn, query);


    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    char *Pocicionid = PQgetvalue(res, 0, 0);
    PQclear(res);
    char *condicion =malloc(1000 * sizeof(char));
    strcat(condicion,"positionid=");
    strcat(condicion,Pocicionid);


    char *condid =malloc(1000 * sizeof(char));
    strcat(condid,"id=");
    strcat(condid,id);
    actuaDisp(conn,condicion,condid);


    return 0;
}


