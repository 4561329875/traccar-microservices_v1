#ifndef MANEJADORDEIMG_H
#define MANEJADORDEIMG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UPLOAD_FOLDER "/app/media"
#define IMAGE_SIZE_LIMIT 5 * 1024 * 1024 // 5MB


#ifdef __cplusplus
extern "C" {
#endif

const char* get_extension(const char *content_type) ;
int save_image(const char *subfolder, const char *filename, const unsigned char *data, size_t size);

#ifdef __cplusplus
}
#endif


#endif
