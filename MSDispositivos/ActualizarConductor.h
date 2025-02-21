#ifndef ACTUALIZARCONDUCTOR_H
#define ACTUALIZARCONDUCTOR_H


#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

char* actuaConductor(PGconn *conn, char *parametrosYvalores, char *selecion);

#ifdef __cplusplus
}
#endif



#endif // ACTUALIZARCONDUCTOR_H
