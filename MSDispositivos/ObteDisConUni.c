#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Add this for string functions
#include <libpq-fe.h>
#include <time.h>



static void trim_end(char *str) {
    int length = strlen(str);

    // Elimina los espacios en blanco al final
    while (length > 0 && str[length - 1] == ' ') {
        str[length - 1] = '\0';  // Reemplaza el espacio con el terminador de cadena
        length--;
    }
}

static void convertirTiempo(const char* input_time, char* output_time) {
    struct tm tm_time;
    int milliseconds;

    // Convierte el string de tiempo a un formato struct tm
    sscanf(input_time, "%4d-%2d-%2d %2d:%2d:%2d.%3d",
           &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
           &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec, &milliseconds);

    // Ajusta tm_year y tm_mon
    tm_time.tm_year -= 1900; // tm_year es años desde 1900
    tm_time.tm_mon -= 1;     // tm_mon es meses desde enero (0-11)

    // Formatea el tiempo en el formato deseado
    strftime(output_time, 30, "%Y-%m-%dT%H:%M:%S", &tm_time);

    // Añadir los milisegundos y la zona horaria
    sprintf(output_time + strlen(output_time), ".%03d+00:00", milliseconds);
}



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

    // Estimate the size needed for the JSON
    // A rough estimate: 200 bytes per column per row, plus some extra space
    size_t json_size = rows * cols * 200 + 1000;

    // Reservar memoria para el JSON
    char *json = (char*)malloc(json_size * sizeof(char));
    if (json == NULL) {
        fprintf(stderr, "Error al asignar memoria para el JSON\n");
        PQclear(res);
        return NULL;
    }

    // Initialize the json string to empty
    json[0] = '\0';

    // Track the current position in the buffer
    size_t pos = 0;

    const char *nomObjJson[] = {
        "id", "attributes", "groupId", "calendarId", "name", "uniqueId",
        "status", "lastUpdate", "positionId", "phone", "model", "contact",
        "category", "disabled", "expirationTime"
    };

    // Recorrer las filas
    for (int i = 0; i < rows; i++) {
        // Use snprintf for safer string operations, tracking position
        pos += snprintf(json + pos, json_size - pos, "{");

        // Recorrer las columnas de cada fila
        for (int j = 0; j < cols; j++) {
            const char *colname = PQfname(res, j);
            char *value = PQgetvalue(res, i, j);

            // Add field name
            pos += snprintf(json + pos, json_size - pos, "\"%s\":", nomObjJson[j]);

            // Handle empty values
            if (strcmp(value, "") != 0) {
                // Handle boolean values for disabled field
                if (strcmp(value, "f") == 0 && strcmp(colname, "disabled") == 0) {
                    pos += snprintf(json + pos, json_size - pos, "false");
                }
                else if (strcmp(value, "t") == 0 && strcmp(colname, "disabled") == 0) {
                    pos += snprintf(json + pos, json_size - pos, "true");
                }
                // Handle string values
                else if (strcmp(colname, "name") == 0 ||
                         strcmp(colname, "uniqueid") == 0 ||
                         strcmp(colname, "status") == 0 ||
                         strcmp(colname, "lastupdate") == 0) {

                    trim_end(value);

                    // Special handling for lastupdate
                    if (strcmp(colname, "lastupdate") == 0) {
                        char output_time[30];
                        convertirTiempo(value, output_time);
                        pos += snprintf(json + pos, json_size - pos, "\"%s\"", output_time);
                    } else {
                        pos += snprintf(json + pos, json_size - pos, "\"%s\"", value);
                    }
                }
                // Handle numeric values
                else {
                    pos += snprintf(json + pos, json_size - pos, "%s", value);
                }
            }
            // Handle null/default values
            else {
                if (strcmp(colname, "groupid") == 0 || strcmp(colname, "calendarid") == 0) {
                    pos += snprintf(json + pos, json_size - pos, "0");
                } else {
                    pos += snprintf(json + pos, json_size - pos, "null");
                }
            }

            // Add comma if not the last column
            if (j < cols - 1) {
                pos += snprintf(json + pos, json_size - pos, ", ");
            }
        }

        pos += snprintf(json + pos, json_size - pos, "}");

        // Add comma if not the last row
        if (i < rows - 1) {
            pos += snprintf(json + pos, json_size - pos, ", ");
        }
    }

    PQclear(res);
    return json;
}
