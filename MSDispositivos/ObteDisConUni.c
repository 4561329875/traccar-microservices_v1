#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Add this for string functions
#include <libpq-fe.h>
#include <time.h>
#include "ConversorJSONDispositivos.h"



char* consultarDispositivosConUni(PGconn *conn, char *condicion) {
    size_t query_len = strlen("SELECT id,attributes,groupid,calendarid,name,uniqueid,status,lastupdate ,positionid,phone, model,contact,category, disabled,expirationtime  FROM tc_devices WHERE ") + strlen(condicion) + 5;
    char *query = (char*)malloc(query_len * sizeof(char));

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Inicializa query con la cadena básica
    strcpy(query, "SELECT id,attributes,groupid,calendarid,name,uniqueid,status,lastupdate ,positionid,phone, model,contact,category, disabled,expirationtime  FROM tc_devices WHERE ");

    // Concatenar la condición de forma segura
    strncat(query, condicion, query_len - strlen(query) - 1);

    PGresult *res = PQexec(conn, query);

    // Free the query string as we don't need it anymore
    free(query);

    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    // Obtener el número de filas y columnas
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    
 
    char *json = NULL;
    char *temp ;
    const char *nomObjJson[] = {
        "id", "attributes", "groupId", "calendarId", "name", "uniqueId",
        "status", "lastUpdate", "positionId", "phone", "model", "contact",
        "category", "disabled", "expirationTime"
    };

    asprintf(&json, "{");

    for (int j = 0; j < cols; j++)
        {
            procesarColumna(res, 0, j, nomObjJson, &json);

            // Si no es la última columna, agregar coma
            if (j < cols - 1)
            {
                temp = json;
                asprintf(&json, "%s, ", json);
                free(temp);
            }
        }
    temp = json;
    asprintf(&json, "%s}", json);
    free(temp);
    PQclear(res);
    return json;
}
