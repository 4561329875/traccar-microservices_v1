// crypto_manager.c
#include "crypto_manager.h"
#include <string.h>
#include <stdlib.h>
#include <openmp.h>

#define SIGNATURE_MAX_LEN 128

static int initialize_keys(CryptoManager* manager) {
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!key) return 0;
    
    if (!EC_KEY_generate_key(key)) {
        EC_KEY_free(key);
        return 0;
    }
    
    manager->private_key = key;
    manager->public_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!manager->public_key) {
        EC_KEY_free(key);
        return 0;
    }
    
    if (!EC_KEY_set_public_key(manager->public_key, EC_KEY_get0_public_key(key))) {
        EC_KEY_free(manager->public_key);
        EC_KEY_free(key);
        return 0;
    }
    
    return 1;
}

CryptoManager* crypto_manager_create(void) {
    CryptoManager* manager = malloc(sizeof(CryptoManager));
    if (!manager) return NULL;
    
    manager->public_key = NULL;
    manager->private_key = NULL;
    
    if (!initialize_keys(manager)) {
        free(manager);
        return NULL;
    }
    
    return manager;
}

void crypto_manager_destroy(CryptoManager* manager) {
    if (manager) {
        if (manager->public_key) EC_KEY_free(manager->public_key);
        if (manager->private_key) EC_KEY_free(manager->private_key);
        free(manager);
    }
}

int crypto_manager_sign(CryptoManager* manager, const unsigned char* data, size_t data_len, 
                       unsigned char** combined, size_t* combined_len) {
    if (!manager || !manager->private_key) return 0;
    
    unsigned char signature[SIGNATURE_MAX_LEN];
    unsigned int sig_len;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Compute signature
            ECDSA_sign(0, data, data_len, signature, &sig_len, manager->private_key);
        }
    }
    
    *combined_len = 1 + sig_len + data_len;
    *combined = malloc(*combined_len);
    if (!*combined) return 0;
    
    (*combined)[0] = (unsigned char)sig_len;
    memcpy(*combined + 1, signature, sig_len);
    memcpy(*combined + 1 + sig_len, data, data_len);
    
    return 1;
}

int crypto_manager_verify(CryptoManager* manager, const unsigned char* combined, size_t combined_len,
                         unsigned char** original_data, size_t* original_len) {
    if (!manager || !manager->public_key || combined_len < 2) return 0;
    
    unsigned int sig_len = combined[0];
    if (combined_len < 1 + sig_len) return 0;
    
    *original_len = combined_len - 1 - sig_len;
    *original_data = malloc(*original_len);
    if (!*original_data) return 0;
    
    memcpy(*original_data, combined + 1 + sig_len, *original_len);
    
    int result = 0;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Verify signature
            result = ECDSA_verify(0, *original_data, *original_len,
                                combined + 1, sig_len, manager->public_key);
        }
    }
    
    if (result != 1) {
        free(*original_data);
        *original_data = NULL;
        return 0;
    }
    
    return 1;
}