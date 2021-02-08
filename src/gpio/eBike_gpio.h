#ifndef EBIKE_GPIO_H
#define EBIKE_GPIO_H

#include <eBike_err.h>

void eBike_gpio_toggle_fault(uint32_t level);

eBike_err_t eBike_gpio_pwm_set_duty(double percentage);
eBike_err_t eBike_gpio_pwm_toggle(bool enable);
eBike_err_t eBike_gpio_init();

#endif