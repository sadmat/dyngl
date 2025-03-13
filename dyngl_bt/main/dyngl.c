#include <stdint.h>
#include <stdio.h>
#include "dyngl_bt.h"
#include "dyngl_spi.h"
#include "dyngl_gpio.h"
#include "dyngl_common.h"
#include "esp_log_buffer.h"
#include "esp_log_level.h"
#include "freertos/projdefs.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define TAG "DYNGL"

void kb_report_cb(uint8_t *report, uint16_t len);
void consumer_report_cb(uint8_t *report, uint16_t len);
void state_chg_cb(dyngl_state_t new_state);

void app_main(void) {
    dyngl_spi_init();
    dyngl_bt_init(
        kb_report_cb,
        consumer_report_cb,
        state_chg_cb,
        CONFIG_DYNGL_BT_KEYBOARD_ADDR
    );
    dyngl_gpio_init();

    dyngl_gpio_state_t input_state = {
        .pairing_mode = false
    };

    while (true) {
        dyngl_gpio_handle_input(&input_state);  
        if (input_state.pairing_mode) {
            input_state.pairing_mode = false;
            dyngl_bt_pairing_mode();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void kb_report_cb(uint8_t *report, uint16_t len) {
    ESP_LOGD(TAG, "kb report received (%dB)", len);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, report, len, ESP_LOG_DEBUG);
    dyngl_spi_send_kb_report(report, len);
}

void consumer_report_cb(uint8_t *report, uint16_t len) {
    ESP_LOGD(TAG, "consumer report received (%dB)", len);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, report, len, ESP_LOG_DEBUG);
    dyngl_spi_send_consumer_report(report, len);
}

void state_chg_cb(dyngl_state_t new_state) {
    ESP_LOGD(TAG, "state changed: %d", new_state);
    dyngl_spi_send_state_chg(new_state);
}
