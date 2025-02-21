#ifndef OBTENERTODOSCONDUCTORES_H
#define OBTENERTODOSCONDUCTORES_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Función que consulta la base de datos y devuelve los resultados en formato JSON
char* conAllConductores(PGconn *conn);
#ifdef __cplusplus
}
#endif



#endif // OBTENERTODOSCONDUCTORES_H
