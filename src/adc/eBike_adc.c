//#include <driver/adc.h>
//#include <esp_adc_cal.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <eBike_err.h>

adc_oneshot_unit_handle_t adc_unit_handle;
adc_cali_handle_t adc_calibration_handle;

eBike_err_t eBike_adc_init() {
    eBike_err_t eBike_err;

    // Initialize an ADC unit and configure the ADC channel's attenuation and bitwidth

    adc_oneshot_unit_init_cfg_t adc_unit_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12
    };

    EBIKE_HANDLE_ERROR(adc_oneshot_new_unit(&adc_unit_config, &adc_unit_handle), EBIKE_ADC_INIT_CREATE_UNIT_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(adc_oneshot_config_channel(adc_unit_handle, CONFIG_THROTTLE_ADC1_CHANNEL, &adc_channel_config), EBIKE_ADC_INIT_CREATE_UNIT_FAIL, eBike_err);

    // Setup ADC calibration

    adc_cali_line_fitting_config_t adc_calibration_config = {
        .unit_id = adc_unit_config.unit_id,
        .atten = adc_channel_config.atten,
        .bitwidth = adc_channel_config.bitwidth,
        .default_vref = 1100
    };

    EBIKE_HANDLE_ERROR(adc_cali_create_scheme_line_fitting(&adc_calibration_config, &adc_calibration_handle), EBIKE_ADC_INIT_CREATE_CALIBRATION_FAIL, eBike_err);

    // Check ADC calibration reference voltage source
    
    adc_cali_line_fitting_efuse_val_t vref_type; 
    EBIKE_HANDLE_ERROR(adc_cali_scheme_line_fitting_check_efuse(&vref_type), EBIKE_ADC_INIT_CHECK_VREF_TYPE_FAIL, eBike_err);

    if (vref_type == ADC_CALI_LINE_FITTING_EFUSE_VAL_EFUSE_VREF) {
        printf("[ADC] - Calibrated using Vref from eFuse.\n");
    }else if (vref_type == ADC_CALI_LINE_FITTING_EFUSE_VAL_EFUSE_TP) {
        printf("[ADC] - Calibrated using Two Point calibration.\n");
    }else{
        printf("[ADC] - Calibrated using Default Vref.\n");
    }

eBike_clean:
    return eBike_err;
}

eBike_err_t eBike_adc_read_throttle(double* throttle_percentage) {
    eBike_err_t eBike_err;

    uint32_t raw_multisampled = 0;
    int raw = 0;

    for (int i = 0; i < CONFIG_ADC1_MULTISAMPLING_COUNT; i++) {
        EBIKE_HANDLE_ERROR(adc_oneshot_read(adc_unit_handle, CONFIG_THROTTLE_ADC1_CHANNEL, &raw), EBIKE_ADC_ONESHOT_READ_FAIL, eBike_err);
        raw_multisampled += raw;
    }

    raw_multisampled /= CONFIG_ADC1_MULTISAMPLING_COUNT;
    int voltage_mv = 0;

    EBIKE_HANDLE_ERROR(adc_cali_raw_to_voltage(adc_calibration_handle, raw_multisampled, &voltage_mv), EBIKE_ADC_RAW_TO_VOLTAGE_CONVERSION_FAIL, eBike_err);

    *throttle_percentage = 100 * ((double) voltage_mv / 3300);

eBike_clean:
    return eBike_err;
}