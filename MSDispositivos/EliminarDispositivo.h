#ifndef ELIMINARDISPOSITIVO_H
#define ELIMINARDISPOSITIVO_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif



int EliminarDisp(PGconn *conn, char *id);

#ifdef __cplusplus
}
#endif



#endif // ELIMINARDISPOSITIVO_H
