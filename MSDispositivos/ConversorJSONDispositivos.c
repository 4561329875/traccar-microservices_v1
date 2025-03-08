#define _GNU_SOURCE  // Add this at the top of the file
#include <stdio.h>   
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <time.h>
#include <omp.h>

static void trim_end(char *str)
{
    int length = strlen(str);

    // Elimina los espacios en blanco al final
    while (length > 0 && str[length - 1] == ' ')
    {
        str[length - 1] = '\0'; // Reemplaza el espacio con el terminador de cadena
        length--;
    }
}


static void convertirTiempo(const char *input_time, char *output_time)
{
    struct tm tm_time;
    int milliseconds;

    // Convierte el string de tiempo a un formato struct tm
    sscanf(input_time, "%4d-%2d-%2d %2d:%2d:%2d.%3d",
           &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
           &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec, &milliseconds);

    // Ajusta tm_year y tm_mon
    tm_time.tm_year -= 1900; // tm_year es años desde 1900
    tm_time.tm_mon -= 1;     // tm_mon es meses desde enero (0-11)

    // Formatea el tiempo en el formato deseado
    strftime(output_time, 30, "%Y-%m-%dT%H:%M:%S", &tm_time);

    // Añadir los milisegundos y la zona horaria
    sprintf(output_time + strlen(output_time), ".%03d+00:00", milliseconds);
}

 void procesarColumna(PGresult *res,const int i,const int j, const char *nomObjJson[], char **row_json)
{
    const char *colname = PQfname(res, j); // Nombre de la columna
    char *value = PQgetvalue(res, i, j);   // Valor de la celda

    char *col_json = NULL;
    char *temp = NULL; // Variable temporal para poder liberar la memoria de los strings
    asprintf(&col_json, "\"%s\":", nomObjJson[j]);

    // Convertir vacíos a null y dar formato según el id
    if (strcmp(value, "") != 0)
    {
        // Convertir 'f' a false y 't' a true solo en disabled
        if (strcmp(value, "f") == 0 && strcmp(colname, "disabled") == 0)
        {
            temp = col_json;
            asprintf(&col_json, "%sfalse", col_json);
            free(temp);
        }
        else if (strcmp(value, "t") == 0 && strcmp(colname, "disabled") == 0)
        {
            temp = col_json;
            asprintf(&col_json, "%strue", col_json);
            free(temp);
        }
        else if (strcmp(colname, "name") == 0 || strcmp(colname, "uniqueid") == 0 || strcmp(colname, "status") == 0 || strcmp(colname, "lastupdate") == 0)
        {
            trim_end(value); // Para quitar caracteres vacíos al final de un string
            if (strcmp(colname, "lastupdate") == 0)
            { // Convertir al formato de fecha adecuado
                char output_time[30];
                convertirTiempo(value, output_time);
                value = output_time;
            }
            temp = col_json;
            asprintf(&col_json, "%s\"%s\"", col_json, value);
            free(temp);
        }
        else
        {
            temp = col_json;
            asprintf(&col_json, "%s%s", col_json, value); // Todo lo que no sea string se guarda directamente
            free(temp);
        }
    }
    else
    {
        if (strcmp(colname, "groupid") == 0 || strcmp(colname, "calendarid") == 0)
        {
            temp = col_json;
            asprintf(&col_json, "%s0", col_json);
            free(temp);
        }
        else if (strcmp(colname, "status") == 0)
        {
            temp = col_json;
            asprintf(&col_json, "%s\"offline\"", col_json);
            free(temp);
        }
        else
        {
            temp = col_json;
            asprintf(&col_json, "%snull", col_json);
            free(temp);
        }
    }

    // Agregar columna al JSON de la fila
    temp = *row_json;
    asprintf(row_json, "%s%s", *row_json, col_json);
    free(temp);
    free(col_json);
    
}

char *generarJSONDispositivo(PGresult *res, int rows, int cols, const char *nomObjJson[]) {
    char *json = NULL;
    char *tempJson = NULL; // Variable temporal para poder liberar la memoria de json
    asprintf(&json, "[");

    char **rows_json = malloc(rows * sizeof(char *));

    // Recorrer las filas
    #pragma omp parallel for //schedule(static, 1)  un ciclo por hilo
    for (int i = 0; i < rows; i++)
    {
        //Para verificar el paralelismo
        int thread_id = omp_get_thread_num();
        printf("Thread %d processing index %d\n", thread_id, i);


        char *row_json = NULL;
        char *temp = NULL; // Variable temporal para poder liberar la memoria de los strings
        asprintf(&row_json, "{");

        // Recorrer las columnas de cada fila
        for (int j = 0; j < cols; j++)
        {
            procesarColumna(res, i, j, nomObjJson, &row_json);
            
            // Si no es la última columna, agregar coma
            if (j < cols - 1)
            {
                temp = row_json;
                asprintf(&row_json, "%s, ", row_json);
                free(temp);
            }
        }

        // Cerrar el objeto JSON de la fila
        temp = row_json;
        asprintf(&row_json, "%s}", row_json);
        free(temp);
        rows_json[i] = row_json;

    }


    for (int i = 0; i < rows; i++) {
        char *tempJson = json;
        asprintf(&json, "%s%s", json, rows_json[i]);
        free(tempJson);
        free(rows_json[i]);

        // Agregar coma si no es la última fila
        if (i < rows - 1) {
            tempJson = json;
            asprintf(&json, "%s, ", json);
            free(tempJson);
        }
    }

    // Cerrar el arreglo de objetos JSON
    tempJson = json;
    asprintf(&json, "%s]", json);
    free(tempJson);

    return json;
}
