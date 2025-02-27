// keystore_model.c
#include "keystore_model.h"
#include <string.h>
#include <openmp.h>

KeystoreModel* keystore_model_create(void) {
    KeystoreModel* model = malloc(sizeof(KeystoreModel));
    if (!model) return NULL;
    
    model->id = 0;
    model->public_key = NULL;
    model->public_key_len = 0;
    model->private_key = NULL;
    model->private_key_len = 0;
    
    return model;
}

void keystore_model_destroy(KeystoreModel* model) {
    if (model) {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                if (model->public_key) {
                    free(model->public_key);
                }
            }
            #pragma omp section
            {
                if (model->private_key) {
                    free(model->private_key);
                }
            }
        }
        free(model);
    }
}

void keystore_model_set_public_key(KeystoreModel* model, const unsigned char* key, size_t len) {
    if (!model) return;
    
    if (model->public_key) {
        free(model->public_key);
    }
    
    model->public_key = malloc(len);
    if (model->public_key) {
        memcpy(model->public_key, key, len);
        model->public_key_len = len;
    }
}

void keystore_model_set_private_key(KeystoreModel* model, const unsigned char* key, size_t len) {
    if (!model) return;
    
    if (model->private_key) {
        free(model->private_key);
    }
    
    model->private_key = malloc(len);
    if (model->private_key) {
        memcpy(model->private_key, key, len);
        model->private_key_len = len;
    }
}