#ifndef EBIKE_ADC_H
#define EBIKE_ADC_H

#include <eBike_err.h>

eBike_err_t eBike_adc_init();
double eBike_adc_read_throttle();

#endif