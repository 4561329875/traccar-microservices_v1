
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>
#define BUFFER_SIZE 100000000

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



char* conAllConductores(PGconn *conn) {
    const char *query = "SELECT id,name,uniqueid,attributes FROM tc_drivers";  // Cambia la consulta según tus necesidades
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

    // Reservar memoria para el JSON (ajusta el tamaño según el número de filas y columnas)
    char *json = (char*)malloc(BUFFER_SIZE * sizeof(char)); // 1MB de espacio, ajusta según tus necesidades
    if (json == NULL) {
        fprintf(stderr, "Error al asignar memoria para el JSON\n");
        PQclear(res);
        return NULL;
    }

    // Iniciar el string JSON
    snprintf(json,BUFFER_SIZE, "[");


    size_t offset = 0;
    offset += snprintf(json + offset, BUFFER_SIZE - offset, "[");
    const char *nomObjJson[] = {
        "id",
        "name",
        "uniqueId",
        "attributes"
    };


    // Recorrer las filas
    for (int i = 0; i < rows; i++) {
        //strcat(json, "{");
        offset += snprintf(json + offset, BUFFER_SIZE - offset, "{");
        // Recorrer las columnas de cada fila
        for (int j = 0; j < cols; j++) {
            const char *colname = PQfname(res, j);  // Nombre de la columna
            char *value = PQgetvalue(res, i, j); // Valor de la celda

            // Agregar nombre de columna y valor al JSON
            //strcat(json, "\"");
            //strcat(json, nomObjJson[j]);
            //strcat(json, "\":");

            offset += snprintf(json + offset, BUFFER_SIZE - offset, "\"");
            offset += snprintf(json + offset, BUFFER_SIZE - offset, nomObjJson[j]);
            offset += snprintf(json + offset, BUFFER_SIZE - offset, "\":");
            //convertir vacios a null y dar formato segun  que id estamos lideando
            if(strcmp(value,"")!=0){
                if(  //guardar los valores que deven ser string como strings
                    strcmp(colname,"name")==0 ||
                    strcmp(colname,"uniqueid")==0){

                    trim_end(value); // para quitar caracteres  vacios al final de un string

                    //strcat(json, "\"");
                    //strcat(json, value);
                    //strcat(json, "\"");
                    offset += snprintf(json + offset, BUFFER_SIZE - offset, "\"");
                    offset += snprintf(json + offset, BUFFER_SIZE - offset, value);
                    offset += snprintf(json + offset, BUFFER_SIZE - offset, "\"");
                }
                else{
                    //strcat(json, value);  // todo lo que no sea string se guarda directamente
                    offset += snprintf(json + offset, BUFFER_SIZE - offset, value);
                }

            }else{
                //strcat(json, "null");
                offset += snprintf(json + offset, BUFFER_SIZE - offset, "null");
            }

            // Si no es la última columna, agregar coma
            if (j < cols - 1) {
                //strcat(json, ", ");
                offset += snprintf(json + offset, BUFFER_SIZE - offset, ", ");
            }
        }
        //strcat(json, "}");
        offset += snprintf(json + offset, BUFFER_SIZE - offset, "}");
        // Si no es la última fila, agregar coma
        if (i < rows - 1) {
            //strcat(json, ", ");
            offset += snprintf(json + offset, BUFFER_SIZE - offset, ", ");
        }
    }

    //strcat(json, "]"); // Cerrar el arreglo de objetos JSON
    offset += snprintf(json + offset, BUFFER_SIZE - offset, "]");
    PQclear(res); // Liberar la memoria utilizada por el resultado

    return json; // Devolver el JSON generado
}
