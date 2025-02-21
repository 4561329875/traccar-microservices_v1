#ifndef ACTUALIZARDISPOSITIVO_H
#define ACTUALIZARDISPOSITIVO_H


#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

char* actuaDisp(PGconn *conn, char *parametrosYvalores, char *selecion);

#ifdef __cplusplus
}
#endif


#endif // ACTUALIZARDISPOSITIVO_H
