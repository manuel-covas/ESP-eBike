#include <sdkconfig.h>
#include <driver/gpio.h>
#include <eBike_err.h>


eBike_err_t eBike_gpio_init() {
    eBike_err_t eBike_err;

    gpio_config_t config;
    config.pin_bit_mask = (uint64_t) (((uint64_t) 1) << CONFIG_BUZZER_GPIO) || (((uint64_t) 1) << CONFIG_FAULT_LED_GPIO);
    config.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    EBIKE_HANDLE_ERROR(gpio_config(&config), EBIKE_GPIO_INIT_CONFIG_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(gpio_set_level(CONFIG_BUZZER_GPIO, 0), EBIKE_GPIO_INIT_SET_LEVEL_BUZZER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(gpio_set_level(CONFIG_FAULT_LED_GPIO, 0), EBIKE_GPIO_INIT_SET_LEVEL_LED_FAIL, eBike_err);
eBike_clean:
    return eBike_err;
}


void eBike_gpio_toggle_fault(uint32_t level) {
    gpio_set_level(CONFIG_BUZZER_GPIO, level);
    gpio_set_level(CONFIG_FAULT_LED_GPIO, level);
}