#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <eBike_err.h>
#include <math.h>


ledc_mode_t pwm_ledc_speed_mode = LEDC_HIGH_SPEED_MODE;
ledc_channel_t pwm_ledc_channel = LEDC_CHANNEL_0;
ledc_timer_t pwm_ledc_timer = LEDC_TIMER_0;
ledc_timer_bit_t pwm_ledc_duty_resolution = LEDC_TIMER_20_BIT;


void eBike_gpio_toggle_fault(uint32_t level) {
    gpio_set_level(CONFIG_BUZZER_GPIO, level);
    gpio_set_level(CONFIG_FAULT_LED_GPIO, level == 0 ? 1 : 0);
}

eBike_err_t eBike_gpio_pwm_toggle(bool enable) {
    eBike_err_t eBike_err;

    if (enable) {
        EBIKE_HANDLE_ERROR(ledc_timer_resume(pwm_ledc_speed_mode, pwm_ledc_timer), EBIKE_GPIO_PWM_TOGGLE_FAIL, eBike_err);
    }else{
        EBIKE_HANDLE_ERROR(ledc_stop(pwm_ledc_speed_mode, pwm_ledc_channel, 0), EBIKE_GPIO_PWM_TOGGLE_FAIL, eBike_err);
    }

eBike_clean:
    return eBike_err;
}

eBike_err_t eBike_gpio_pwm_set_duty(double percentage) {
    eBike_err_t eBike_err;

    if (percentage > 98) percentage = 98.0;
    if (percentage < 2) percentage = 2.0;

    percentage /= 100;
    uint32_t target = percentage * pow(2, pwm_ledc_duty_resolution);

    EBIKE_HANDLE_ERROR(ledc_set_duty(pwm_ledc_speed_mode, pwm_ledc_channel, target), EBIKE_GPIO_PWM_SET_DUTY_FAIL, eBike_err);
    return eBike_err;

eBike_clean:
    printf("[GPIO] - Failed to set PWM duty cycle!\n"
           "    eBike_err: %s (%i) esp_err: %s (%i)\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err);
    abort();  // Failure to set duty cycle could cause a stuck throttle situation. Reboot right away!
}


eBike_err_t eBike_gpio_init() {
    eBike_err_t eBike_err;

    gpio_config_t config = {
        .pin_bit_mask = (uint64_t) ((((uint64_t) 1) << CONFIG_BUZZER_GPIO) | (((uint64_t) 1) << CONFIG_FAULT_LED_GPIO)),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    EBIKE_HANDLE_ERROR(gpio_config(&config), EBIKE_GPIO_INIT_CONFIG_FAIL, eBike_err);
    eBike_gpio_toggle_fault(0);

    ledc_timer_config_t ledc_timer_configuration = {
        .speed_mode = pwm_ledc_speed_mode,
        .duty_resolution = pwm_ledc_duty_resolution,
        .timer_num = pwm_ledc_timer,
        .freq_hz = CONFIG_PWM_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    EBIKE_HANDLE_ERROR(ledc_timer_config(&ledc_timer_configuration), EBIKE_GPIO_INIT_PWM_CONFIG_FAIL, eBike_err);

    ledc_channel_config_t ledc_channel_configuration = {
        .gpio_num = CONFIG_PWM_GPIO,
        .speed_mode = pwm_ledc_speed_mode,
        .channel = pwm_ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = pwm_ledc_timer,
        .duty = 0,
        .hpoint = 0
    };

    EBIKE_HANDLE_ERROR(ledc_channel_config(&ledc_channel_configuration), EBIKE_GPIO_INIT_PWM_CONFIG_FAIL, eBike_err);
    eBike_err = eBike_gpio_pwm_toggle(false);

eBike_clean:
    return eBike_err;
}