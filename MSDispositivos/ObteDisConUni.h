#ifndef OBTEDISCONUNI_H
#define OBTEDISCONUNI_H

#ifdef __cplusplus
extern "C" {
#endif
// Función que consulta la base de datos y devuelve los resultados en formato JSON
char* consultarDispositivosConUni(PGconn *conn, char *condicion);

#ifdef __cplusplus
}
#endif


#endif // OBTEDISCONUNI_H
