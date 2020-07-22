#ifndef EBIKE_AUTH_H
#define EBIKE_AUTH_H

#include <eBike_err.h>

eBike_err_t eBike_auth_init();
uint8_t* eBike_auth_get_challenge();
bool eBike_auth_solve_challenge(unsigned char* message, size_t message_length, const unsigned char* signature, size_t signature_length);

#endif