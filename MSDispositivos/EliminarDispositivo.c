#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>
#include <stdbool.h>




int EliminarDisp(PGconn *conn, char *id) {
    size_t query_len = strlen("DELETE FROM tc_devices WHERE id=") + strlen(id) + 1; // Incluye el ID
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Crear la consulta DELETE
    strcpy(query, "DELETE FROM tc_devices WHERE id=");

    // Concatenar el ID del dispositivo
    strncat(query, id, query_len - strlen(query) - 1);


    PGresult *res = PQexec(conn, query);

    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    PQclear(res);
    return 0; // Devolver el JSON generado
}
