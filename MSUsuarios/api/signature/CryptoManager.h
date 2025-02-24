// crypto_manager.h
#ifndef CRYPTO_MANAGER_H
#define CRYPTO_MANAGER_H

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/err.h>
#include <stdint.h>

typedef struct {
    EC_KEY *public_key;
    EC_KEY *private_key;
} CryptoManager;

CryptoManager* crypto_manager_create(void);
void crypto_manager_destroy(CryptoManager* manager);
int crypto_manager_sign(CryptoManager* manager, const unsigned char* data, size_t data_len, 
                       unsigned char** combined, size_t* combined_len);
int crypto_manager_verify(CryptoManager* manager, const unsigned char* combined, size_t combined_len,
                         unsigned char** original_data, size_t* original_len);

#endif // CRYPTO_MANAGER_H