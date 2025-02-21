#ifndef GUARDARCONDUCTOR_H
#define GUARDARCONDUCTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

char* guardarConductor(PGconn *conn, char *parametros, char *valores);

#ifdef __cplusplus
}
#endif


#endif // GUARDARCONDUCTOR_H
