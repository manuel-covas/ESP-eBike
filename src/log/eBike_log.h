#ifndef EBIKE_LOG_H
#define EBIKE_LOG_H

#include <stdbool.h>
#include <eBike_err.h>

eBike_err_t eBike_log_init();

void eBike_log_add(char* message, uint16_t length);
int eBike_log_length();
char* eBike_log_content();
void eBike_log_send();

#endif