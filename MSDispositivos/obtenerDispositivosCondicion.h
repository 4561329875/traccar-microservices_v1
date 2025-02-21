#ifndef OBTENERDISPOSITIVOSCONDICION_H
#define OBTENERDISPOSITIVOSCONDICION_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif
// Función que consulta la base de datos y devuelve los resultados en formato JSON
char* consultarDispositivosCon(PGconn *conn, const  char *condicion);

#ifdef __cplusplus
}
#endif

#endif // OBTENERDISPOSITIVOSCONDICION_H
