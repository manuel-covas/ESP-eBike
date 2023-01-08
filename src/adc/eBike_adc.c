#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <eBike_err.h>

static esp_adc_cal_characteristics_t* adc_characteristics;

eBike_err_t eBike_adc_init() {
    eBike_err_t eBike_err;
    
    EBIKE_HANDLE_ERROR(adc1_config_width(ADC_WIDTH_BIT_12), EBIKE_ADC_INIT_SET_WIDTH_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(adc1_config_channel_atten(CONFIG_THROTTLE_ADC1_CHANNEL, ADC_ATTEN_DB_11), EBIKE_ADC_INIT_SET_ATTEN_FAIL, eBike_err);

    //Characterize ADC at particular atten

    adc_characteristics = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t calibration_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_characteristics);
    
    //Check type of calibration value used to characterize ADC
    
    if (calibration_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("[ADC] - Calibrated using Vref from eFuse.\n");
    }else if (calibration_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("[ADC] - Calibrated using Two Point calibration.\n");
    }else{
        printf("[ADC] - Calibrated using Default Vref.\n");
    }

eBike_clean:
    return eBike_err;
}

double eBike_adc_read_throttle() {

    uint32_t raw_multisampled = 0;
    
    for (int i = 0; i < CONFIG_ADC1_MULTISAMPLING_COUNT; i++) {
        raw_multisampled += adc1_get_raw(CONFIG_THROTTLE_ADC1_CHANNEL);
    }
    raw_multisampled /= CONFIG_ADC1_MULTISAMPLING_COUNT;

    double voltage = ((double) esp_adc_cal_raw_to_voltage(raw_multisampled, adc_characteristics)) / 1000.0;
    return 100 * (voltage / 3.3);
}