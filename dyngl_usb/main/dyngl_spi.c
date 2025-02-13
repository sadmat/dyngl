#include "dyngl_spi.h"
#include "dyngl_common.h"
#include "driver/spi_common.h"
#include "driver/spi_slave.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/spi_types.h"
#include "portmacro.h"

#define TAG "DYNGL"

void post_transaction_cb(spi_slave_transaction_t *transaction);

static dyngl_spi_kb_report_cb kb_callback = NULL;
static dyngl_spi_consumer_report_cb consumer_callback = NULL;
static void *transaction_buffer = NULL;

void dyngl_spi_init(
    dyngl_spi_kb_report_cb kb_report_cb,
    dyngl_spi_consumer_report_cb consumer_report_cb
) {
    kb_callback = kb_report_cb;
    consumer_callback = consumer_report_cb;

    spi_bus_config_t buscfg = {
        .mosi_io_num = 11,
        .miso_io_num = 13,
        .sclk_io_num = 12,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = 10,
        .queue_size = 3,
        .flags = 0,
        .post_setup_cb = NULL,
        .post_trans_cb = NULL
    };

    ESP_ERROR_CHECK(spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));

    transaction_buffer = spi_bus_dma_memory_alloc(SPI2_HOST, sizeof(dyngl_message_t), 0);
    assert(transaction_buffer);
}

void dyngl_spi_run_task() {
    static spi_slave_transaction_t transaction = { 0 };
    transaction.rx_buffer = transaction_buffer;
    transaction.tx_buffer = NULL;
    transaction.length = sizeof(dyngl_message_t) * 8;
    ESP_ERROR_CHECK(spi_slave_transmit(SPI2_HOST, &transaction, portMAX_DELAY));
    dyngl_message_t *msg = (dyngl_message_t *)transaction.rx_buffer;
    if (msg->report_type == DYNGL_REPORT_KEYBOARD) {
        if (kb_callback) {
            kb_callback(msg->report.keyboard.modifier_keys, msg->report.keyboard.report);
        }
    } else if (msg->report_type == DYNGL_REPORT_CONSUMER) {
        if (consumer_callback) {
            consumer_callback(msg->report.consumer.report, DYNGL_CONSUMER_REPORT_LEN);
        }
    } else {
        ESP_LOGW(TAG, "Unknown report received: report type: %d, tx len: %zu", msg->report_type, transaction.trans_len);
    }
}

