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




//INSERT INTO dispositivos (nombre, tipo)
//   VALUES ('Dispositivo A', 'Tipo X')
//   RETURNING id;





char* guardarConductor(PGconn *conn, char *parametros, char *valores) {
    size_t query_len = strlen("INSERT INTO tc_drivers") + strlen(parametros)+ strlen(" VALUES ")+strlen(valores)+strlen("RETURNING id  ") + 1;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    strcpy(query, "INSERT INTO tc_drivers ");

    // Concatenar parámetros, valores y el resto de la consulta
    strncat(query, parametros, query_len - strlen(query) - 1); // Concatenar los parámetros
    strncat(query, " VALUES ", query_len - strlen(query) - 1); // Concatenar " VALUES "
    strncat(query, valores, query_len - strlen(query) - 1); // Concatenar los valores
    strncat(query, " RETURNING id ", query_len - strlen(query) - 1);




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
    char sec[1000];  // Ajusta el tamaño según sea necesario

    // Inicializa la cadena con "("
    snprintf(sec, sizeof(sec), "(d.id=%s)", id_str);


    return conCondUniConductores(conn,sec); // Devolver el JSON generado
}
