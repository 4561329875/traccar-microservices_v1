#ifndef ACTUALIZARACUMULADORES_H
#define ACTUALIZARACUMULADORES_H


#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

char* conUniAcumuladores(PGconn *conn,char *id);

int ActualizarAcumuladores(PGconn *conn,char *id, char * atributos) ;

#ifdef __cplusplus
}
#endif




#endif // ACTUALIZARACUMULADORES_H
