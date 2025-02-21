#ifndef OBTENERCONDICIUNICONDUCTORES_H
#define OBTENERCONDICIUNICONDUCTORES_H

#ifdef __cplusplus
extern "C" {
#endif

// Función que consulta la base de datos y devuelve los resultados en formato JSON
char* conCondUniConductores(PGconn *conn,char *condicion);
#ifdef __cplusplus
}
#endif


#endif // OBTENERCONDICIUNICONDUCTORES_H
