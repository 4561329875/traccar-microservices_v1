
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>
#include "ConversorJSONDispositivos.h"


char *consultarDispositivos(PGconn *conn){
    const char *query = "SELECT id,attributes,groupid,calendarid,name,uniqueid,status,lastupdate ,positionid,phone, model,contact,category, disabled,expirationtime  FROM tc_devices"; // Cambia la consulta según tus necesidades
    PGresult *res = PQexec(conn, query);

    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
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
        "expirationTime"};

    char *json = generarJSONDispositivo(res, rows, cols, nomObjJson);
    PQclear(res); // Liberar la memoria utilizada por el resultado

    return json; // Devolver el JSON generado
}