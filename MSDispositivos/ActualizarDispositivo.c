#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>
#include "ObtenerCondiciUniConductores.h"


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


//UPDATE cars
//SET color = 'white', year = 1970
//                     WHERE brand = 'Toyota';
//   RETURNING id;





char* actuaDisp(PGconn *conn, char *parametrosYvalores, char *selecion) {

    size_t query_len = strlen("UPDATE tc_devices SET ") + strlen(parametrosYvalores) + strlen(" WHERE ") + strlen(selecion) + strlen(" RETURNING id") + 1;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    strcpy(query, "UPDATE tc_devices SET ");

    // Concatenar parámetros y valores
    strncat(query, parametrosYvalores, query_len - strlen(query) - 1);
    strncat(query, " WHERE ", query_len - strlen(query) - 1);
    strncat(query, selecion, query_len - strlen(query) - 1);
    strncat(query, " RETURNING id", query_len - strlen(query) - 1);


    PGresult *res = PQexec(conn, query);

    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }


    // Obtener el valor del id desde la primera fila y primera columna
    const char *id = PQgetvalue(res, 0, 0);  // 0, 0 porque estamos esperando un solo valor

    // Convertir a un formato adecuado si es necesario
    char *id_str = strdup(id);  // Copiar el valor de id a una nueva cadena

    // Liberar los recursos de la consulta
    PQclear(res);



    // Reservar memoria para la cadena final
    char sec[100];  // Ajusta el tamaño según sea necesario

    // Inicializa la cadena con "("
    snprintf(sec, sizeof(sec), "id=%s", id_str);


    return consultarDispositivosConUni(conn,sec); // Devolver el JSON generado
}
