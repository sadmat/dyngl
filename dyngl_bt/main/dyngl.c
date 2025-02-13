#include <stdint.h>
#include <stdio.h>
#include "dyngl_bt.h"
#include "dyngl_spi.h"
#include "esp_log_buffer.h"
#include "esp_log_level.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define TAG "DYNGL"

void kb_report_cb(uint8_t *report, uint16_t len);
void consumer_report_cb(uint8_t *report, uint16_t len);

void app_main(void) {
    dyngl_bt_init(kb_report_cb, consumer_report_cb, CONFIG_DYNGL_BT_KEYBOARD_ADDR);
    dyngl_spi_init();
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
