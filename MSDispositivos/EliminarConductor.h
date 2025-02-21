#ifndef ELIMINARCONDUCTOR_H
#define ELIMINARCONDUCTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif



int eliminarConductor(PGconn *conn, char *id);

#ifdef __cplusplus
}
#endif

#endif // ELIMINARCONDUCTOR_H
