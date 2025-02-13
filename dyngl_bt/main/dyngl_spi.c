#include "dyngl_spi.h"
#include "dyngl_common.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include <stdint.h>
#include <string.h>

#define TAG "DYNGL"
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static spi_device_handle_t handle;
static spi_transaction_t transaction;

void dyngl_spi_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = CONFIG_DYNGL_SPI_MOSI,
        .miso_io_num = CONFIG_DYNGL_SPI_MISO,
        .sclk_io_num = CONFIG_DYNGL_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 5000000,
        .duty_cycle_pos = 128,
        .mode = 0,
        .spics_io_num = CONFIG_DYNGL_SPI_CS,
        .cs_ena_posttrans = 3,
        .queue_size = 3
    };

    ESP_LOGI(TAG, "Starting SPI");
    ESP_LOGI(TAG, "MISO: %d", CONFIG_DYNGL_SPI_MISO);
    ESP_LOGI(TAG, "MOSI: %d", CONFIG_DYNGL_SPI_MOSI);
    ESP_LOGI(TAG, "SCLK: %d", CONFIG_DYNGL_SPI_SCLK);
    ESP_LOGI(TAG, "CS: %d", CONFIG_DYNGL_SPI_CS);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); 
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &handle));
}

void dyngl_spi_send_kb_report(const uint8_t *report, uint16_t len) {
    dyngl_message_t msg = {
        .report_type = DYNGL_REPORT_KEYBOARD
    };
    msg.report.keyboard.modifier_keys = report[0];
    if (len - 2 > DYNGL_KB_REPORT_LEN) {
        ESP_LOGW(TAG, "Received %dB kb report. Cutting down to %d", len, DYNGL_KB_REPORT_LEN);
    }
    memcpy(&msg.report.keyboard.report, &report[2], MIN(len - 2, DYNGL_KB_REPORT_LEN));

    transaction.tx_buffer = &msg;
    transaction.rx_buffer = NULL;
    transaction.length = sizeof(msg) * 8;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}

void dyngl_spi_send_consumer_report(const uint8_t *report, uint16_t len) {
    dyngl_message_t msg = {
        .report_type = DYNGL_REPORT_CONSUMER
    };

    if (len > DYNGL_CONSUMER_REPORT_LEN) {
        ESP_LOGW(TAG, "Received %dB consumer report. Cutting down to %d", len, DYNGL_CONSUMER_REPORT_LEN);
    }
    memcpy(&msg.report.consumer.report, report, MIN(len, DYNGL_CONSUMER_REPORT_LEN));

    transaction.tx_buffer = &msg;
    transaction.rx_buffer = NULL;
    transaction.length = sizeof(msg) * 8;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}
