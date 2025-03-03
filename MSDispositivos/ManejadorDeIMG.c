
#include <libpq-fe.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UPLOAD_FOLDER "/app/media"
#define IMAGE_SIZE_LIMIT 5 * 1024 * 1024 // 5MB

void create_upload_folder(const char *subfolder) {
    // Concatenate the subfolder to the base path
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", UPLOAD_FOLDER, subfolder);

    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700); // Create the folder if it doesn't exist
    }
}

int save_image(PGconn *conn,const char *IdDispocitivo, const char *filename, const unsigned char *data, size_t size) {
    if (size > IMAGE_SIZE_LIMIT) {
        return 0; // Image too large
    }

    const char *plantilla ="SELECT uniqueid FROM tc_devices WHERE id=";
    size_t query_len = strlen(plantilla) + strlen(IdDispocitivo) + 5;
    char *query = (char*)malloc(query_len * sizeof(char)); // Reserva memoria

    if (query == NULL) {
        fprintf(stderr, "Error al reservar memoria para la consulta.\n");
        return NULL;
    }

    // Inicializa query con la cadena básica
    strcpy(query, plantilla);

    // Concatenar la condición de forma segura
    strncat(query, IdDispocitivo, query_len - strlen(query) - 1);

    PGresult *res = PQexec(conn, query);


    // Verificar si la consulta fue exitosa
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Consulta fallida: %s", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        fprintf(stderr, "No se encontraron resultados.\n");
        PQclear(res);
        return NULL;
    }

    char *subfolder = strdup(PQgetvalue(res, 0, 0));

    // Create the subfolder before saving the image
    create_upload_folder(subfolder);

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s/%s", UPLOAD_FOLDER, subfolder, filename);

    FILE *file = fopen(filepath, "wb");
    if (!file) return 0;
    fwrite(data, 1, size, file);
    fclose(file);
    return 1;
}

const char* get_extension(const char *content_type) {
    if (strcmp(content_type, "image/jpeg") == 0) return ".jpg";
    if (strcmp(content_type, "image/png") == 0) return ".png";
    if (strcmp(content_type, "image/gif") == 0) return ".gif";
    if (strcmp(content_type, "image/webp") == 0) return ".webp";
    return "";
}
