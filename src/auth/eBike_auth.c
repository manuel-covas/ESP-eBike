#include <esp_system.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_util.h>
#include <eBike_ble.h>
#include <mbedtls/pk.h>
#include <mbedtls/md_internal.h>
#include <mbedtls/error.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <sdkconfig.h>


void eBike_auth_refresh_challenge();

const unsigned char* public_key = (unsigned char *) "-----BEGIN PUBLIC KEY-----\n" CONFIG_AUTH_RSA_PUBLIC_KEY "\n-----END PUBLIC KEY-----";
const mbedtls_md_info_t* hash_info;
uint8_t* current_challenge;
mbedtls_pk_context rsa_context;


eBike_err_t eBike_auth_init() {
    eBike_err_t eBike_err = { .esp_err = ESP_OK, .eBike_err_type = EBIKE_OK };
    
    hash_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_pk_init(&rsa_context);
    current_challenge = malloc(CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);

    if (current_challenge == NULL) {
        char* error_message = "[Auth] - Failed to malloc challenge array!\n";
        eBike_log_add(error_message, strlen(error_message));
        eBike_err.eBike_err_type = EBIKE_AUTH_INIT_MALLOC_FAIL;
        return eBike_err;
    }

    eBike_auth_refresh_challenge();

    int error = mbedtls_pk_parse_public_key(&rsa_context, public_key, strlen((const char*) public_key) + 1);
    if (error != 0) {
        eBike_log_add("[Auth] - Failed to parse configured RSA public key!\n", 52);
        eBike_err.eBike_err_type = EBIKE_AUTH_INIT_PARSE_KEY_FAIL;
        return eBike_err;
    }

    return eBike_err;
}


void eBike_auth_refresh_challenge() {
    if (current_challenge != NULL)
        esp_fill_random(current_challenge, CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);
}


uint8_t* eBike_auth_get_challenge() {
    return current_challenge;
}


bool eBike_auth_solve_challenge(unsigned char* authed_cmd, size_t authed_cmd_length, const unsigned char* signature, size_t signature_length) {
    
    int error = 0;
    char* mbedtls_error_message = calloc(100, 1);
    char* error_message = calloc(100, 1);
    char* signed_content = calloc(authed_cmd_length + CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH, 1);
    unsigned char* hash = calloc(hash_info->size, 1);

    if (mbedtls_error_message == NULL || error_message == NULL || signed_content == NULL || hash == NULL) {
        error = -1;

        char* msg = "[Auth] - Malloc to carry out verification failed!\n";
        eBike_log_add(msg, strlen(msg));
        goto eBike_clean;
    }

    memcpy(signed_content, authed_cmd, authed_cmd_length);
    memcpy(signed_content + authed_cmd_length, current_challenge, CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);

    error = mbedtls_md(hash_info, (unsigned char*) signed_content, authed_cmd_length + CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH, hash);

    if (error != 0) {
        mbedtls_strerror(error, mbedtls_error_message, 99);
        strcat(error_message, "[Auth] - Error hashing authed command: ");
        goto challenge_failed;
    }  

    error = mbedtls_pk_verify(&rsa_context, MBEDTLS_MD_NONE, hash, hash_info->size, signature, signature_length);

    if (error != 0) {
        mbedtls_strerror(error, mbedtls_error_message, 99);
        strcat(error_message, "[Auth] - Signature verification failed: ");
        goto challenge_failed;
    }
    
    goto eBike_clean;

challenge_failed:
    if (error_message != NULL || mbedtls_error_message != NULL) {
        strcat(error_message, mbedtls_error_message);
        strcat(error_message, "\n");
        eBike_log_add(error_message, strlen(error_message));
    }
eBike_clean:
    if (error_message != NULL) free(error_message);
    if (mbedtls_error_message != NULL) free(mbedtls_error_message);
    if (signed_content != NULL) free(signed_content);
    if (hash != NULL) free(hash);

    eBike_auth_refresh_challenge();
    return error == 0;
}