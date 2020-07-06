#ifndef EBIKE_GPIO_H
#define EBIKE_GPIO_H

#include <eBike_err.h>

eBike_err_t eBike_gpio_init();
void eBike_gpio_toggle_fault(uint32_t level);

#endif