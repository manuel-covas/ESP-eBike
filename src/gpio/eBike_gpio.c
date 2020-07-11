#include <sdkconfig.h>
#include <driver/gpio.h>
#include <eBike_err.h>


eBike_err_t eBike_gpio_init() {
    eBike_err_t eBike_err;

    EBIKE_HANDLE_ERROR(gpio_set_direction(CONFIG_BUZZER_GPIO, GPIO_MODE_OUTPUT), EBIKE_GPIO_INIT_SET_DIRECTION_BUZZER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(gpio_set_direction(CONFIG_FAULT_LED_GPIO, GPIO_MODE_OUTPUT), EBIKE_GPIO_INIT_SET_DIRECTION_LED_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(gpio_set_level(CONFIG_BUZZER_GPIO, 0), EBIKE_GPIO_INIT_SET_LEVEL_BUZZER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(gpio_set_level(CONFIG_FAULT_LED_GPIO, 0), EBIKE_GPIO_INIT_SET_LEVEL_LED_FAIL, eBike_err);
eBike_clean:
    return eBike_err;
}


void eBike_gpio_toggle_fault(uint32_t level) {
    gpio_set_level(CONFIG_BUZZER_GPIO, level);
    gpio_set_level(CONFIG_FAULT_LED_GPIO, level);
}