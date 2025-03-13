#include "dyngl_gpio.h"
#include "driver/gpio.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
#include "esp_timer.h"
#include <stdint.h>

#define DEBOUNCE_MILLIS 100

void dyngl_gpio_init() {
    const gpio_config_t config = {
        .pin_bit_mask = BIT64(GPIO_NUM_0),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
}

void dyngl_gpio_handle_input(dyngl_gpio_state_t *state) {
    static int32_t last_pressed = 0;
    static bool was_held = false;

    if (esp_timer_get_time() / 1000 - last_pressed < DEBOUNCE_MILLIS) {
        return;
    } 

    bool is_button_pressed = !gpio_get_level(GPIO_NUM_0);

    if (is_button_pressed && !was_held) {
        last_pressed = esp_timer_get_time() / 1000;
        was_held = true;
        state->pairing_mode = true;
    } else if (!is_button_pressed) {
        was_held = false;
    }
}
