// keystore_model.h
#ifndef KEYSTORE_MODEL_H
#define KEYSTORE_MODEL_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int64_t id;
    unsigned char* public_key;
    size_t public_key_len;
    unsigned char* private_key;
    size_t private_key_len;
} KeystoreModel;

KeystoreModel* keystore_model_create(void);
void keystore_model_destroy(KeystoreModel* model);
void keystore_model_set_public_key(KeystoreModel* model, const unsigned char* key, size_t len);
void keystore_model_set_private_key(KeystoreModel* model, const unsigned char* key, size_t len);

#endif // KEYSTORE_MODEL_H