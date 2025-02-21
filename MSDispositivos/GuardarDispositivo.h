#ifndef GUARDARDISPOSITIVO_H
#define GUARDARDISPOSITIVO_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

char* guardarDisp(PGconn *conn, char *parametros, char *valores);

#ifdef __cplusplus
}
#endif



#endif // GUARDARDISPOSITIVO_H
