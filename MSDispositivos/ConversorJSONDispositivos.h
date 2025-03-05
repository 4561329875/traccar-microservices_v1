#ifndef CONVERSORJSONDISPOSITIVOS_H
#define CONVERSORJSONDISPOSITIVOS_H


#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif



void procesarColumna(PGresult *res, int i, int j, const char *nomObjJson[], char **row_json); 
char *generarJSONDispositivo(PGresult *res, int rows, int cols, const char *nomObjJson[]);
#ifdef __cplusplus
}
#endif

#endif 
