// token_manager.c
#include "token_manager.h"
#include <string.h>
#include <stdlib.h>
#include <openmp.h>
#include <jansson.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

static char* base64_encode(const unsigned char* input, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    char* result = malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = 0;
    
    BIO_free_all(bio);
    return result;
}

static unsigned char* base64_decode(const char* input, size_t* out_len) {
    BIO *bio, *b64;
    size_t length = strlen(input);
    unsigned char* result = malloc(length);
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, -1);
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *out_len = BIO_read(bio, result, length);
    
    BIO_free_all(bio);
    return result;
}

TokenManager* token_manager_create(void) {
    TokenManager* manager = malloc(sizeof(TokenManager));
    if (!manager) return NULL;
    
    manager->crypto_manager = crypto_manager_create();
    if (!manager->crypto_manager) {
        free(manager);
        return NULL;
    }
    
    return manager;
}

void token_manager_destroy(TokenManager* manager) {
    if (manager) {
        if (manager->crypto_manager) {
            crypto_manager_destroy(manager->crypto_manager);
        }
        free(manager);
    }
}

char* token_manager_generate(TokenManager* manager, int64_t user_id, time_t expiration) {
    if (!manager) return NULL;
    
    if (expiration == 0) {
        expiration = time(NULL) + (DEFAULT_EXPIRATION_DAYS * 24 * 60 * 60);
    }
    
    json_t* root = json_object();
    json_object_set_new(root, "u", json_integer(user_id));
    json_object_set_new(root, "e", json_integer(expiration));
    
    char* json_str = json_dumps(root, JSON_COMPACT);
    json_decref(root);
    
    unsigned char* signed_data;
    size_t signed_len;
    
    if (!crypto_manager_sign(manager->crypto_manager, 
                           (unsigned char*)json_str, strlen(json_str),
                           &signed_data, &signed_len)) {
        free(json_str);
        return NULL;
    }
    
    free(json_str);
    
    char* token = base64_encode(signed_data, signed_len);
    free(signed_data);
    
    return token;
}

TokenData* token_manager_verify(TokenManager* manager, const char* token) {
    if (!manager || !token) return NULL;
    
    size_t decoded_len;
    unsigned char* decoded = base64_decode(token, &decoded_len);
    if (!decoded) return NULL;
    
    unsigned char* original_data;
    size_t original_len;
    
    if (!crypto_manager_verify(manager->crypto_manager, decoded, decoded_len,
                             &original_data, &original_len)) {
        free(decoded);
        return NULL;
    }
    
    free(decoded);
    
    // Ensure null termination for json parsing
    unsigned char* json_str = malloc(original_len + 1);
    memcpy(json_str, original_data, original_len);
    json_str[original_len] = 0;
    free(original_data);
    
    json_error_t error;
    json_t* root = json_loads((char*)json_str, 0, &error);
    free(json_str);
    
    if (!root) return NULL;
    
    TokenData* data = malloc(sizeof(TokenData));
    if (!data) {
        json_decref(root);
        return NULL;
    }
    
    data->user_id = json_integer_value(json_object_get(root, "u"));
    data->expiration = json_integer_value(json_object_get(root, "e"));
    
    json_decref(root);
    
    if (data->expiration < time(NULL)) {
        free(data);
        return NULL;
    }
    
    return data;
}