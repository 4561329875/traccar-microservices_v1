
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




char* conCondUniConductores(PGconn *conn,char *condicion) {



    const char *plantilla = "SELECT d.id, d.name, d.uniqueid, d.attributes FROM tc_drivers d LEFT JOIN tc_user_driver u ON d.id = u.driverid LEFT JOIN tc_group_driver g ON d.id = g.driverid LEFT JOIN tc_device_driver de ON d.id = de.driverid WHERE ";
    const char *limit = " LIMIT 1";
    size_t query_len = strlen(plantilla) + strlen(condicion) + strlen(limit) + 1;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Construye la consulta
    strcpy(query, plantilla);
    strncat(query, condicion, query_len - strlen(query) - 1);
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

    // Reservar memoria para el JSON (ajusta el tamaño según el número de filas y columnas)
    char *json = (char*)malloc(BUFFER_SIZE * sizeof(char)); // 1MB de espacio, ajusta según tus necesidades
    if (json == NULL) {
        fprintf(stderr, "Error al asignar memoria para el JSON\n");
        PQclear(res);
        return NULL;
    }

    // Iniciar el string JSON
    snprintf(json,BUFFER_SIZE, "");


    size_t offset = 0;
    offset += snprintf(json + offset, BUFFER_SIZE - offset, "");
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
    offset += snprintf(json + offset, BUFFER_SIZE - offset, "");
    PQclear(res); // Liberar la memoria utilizada por el resultado

    return json; // Devolver el JSON generado
}
