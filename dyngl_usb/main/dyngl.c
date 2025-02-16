#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "dyngl_spi.h"
#include "dyngl_usb.h"
#include "dyngl_common.h"
#include "esp_log.h"
#include "esp_log_buffer.h"
#include "esp_log_level.h"

#define TAG "DYNGL"

void kb_report_cb(uint8_t modifier_keys, const uint8_t *report);
void consumer_report_cb(uint8_t *report, uint16_t len);

void app_main(void) {
    dyngl_spi_init(kb_report_cb, consumer_report_cb);
    dyngl_usb_init();
    while (true) {
        dyngl_spi_run_task();
    }
}

void kb_report_cb(uint8_t modifier_keys, const uint8_t *report) {
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, report, DYNGL_KB_REPORT_LEN, ESP_LOG_DEBUG);
    dyngl_usb_send_kb_report(modifier_keys, report);
}

void consumer_report_cb(uint8_t *report, uint16_t len) {
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, report, DYNGL_CONSUMER_REPORT_LEN, ESP_LOG_DEBUG);
    dyngl_usb_send_consumer_report(report, len);
}
