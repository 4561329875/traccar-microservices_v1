#ifndef OBTENERCONDICICONDUCTORES_H
#define OBTENERCONDICICONDUCTORES_H
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Función que consulta la base de datos y devuelve los resultados en formato JSON
char* conCondConductores(PGconn *conn,char *condicion);
#ifdef __cplusplus
}
#endif

#endif // OBTENERCONDICICONDUCTORES_H
