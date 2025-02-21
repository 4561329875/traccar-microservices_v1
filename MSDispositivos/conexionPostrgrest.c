#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

PGconn* conectarDB(void) {
    const char *params[] = {
        "PG_DBNAME", "PG_USER", "PG_PASSWORD", "PG_HOST", "PG_PORT"
    };
    const char *values[5];

    // Verificar variables de entorno
    for (int i = 0; i < 5; i++) {
        values[i] = getenv(params[i]);
        if (!values[i]) {
            fprintf(stderr, "Error: Variable de entorno %s no definida\n", params[i]);
            return NULL;
        }
    }

    // Construir cadena de conexión de forma segura
    const char *keywords[] = {"dbname", "user", "password", "host", "port", NULL};
    const char *values_conn[] = {values[0], values[1], values[2], values[3], values[4], NULL};

    // Conectar usando el método más seguro
    PGconn *conn = PQconnectdbParams(keywords, values_conn, 0);

    if (!conn) {
        fprintf(stderr, "Error: No se pudo asignar memoria para la conexión\n");
        return NULL;
    }

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Error de conexión: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    printf("Conexión exitosa a la base de datos\n");
    return conn;
}
