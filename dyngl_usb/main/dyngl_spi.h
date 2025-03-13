#ifndef DYNGL_SPI_H
#define DYNGL_SPI_H

#include <stdint.h>
#include "dyngl_common.h"

typedef void (*dyngl_spi_kb_report_cb)(uint8_t, const uint8_t *);
typedef void (*dyngl_spi_consumer_report_cb)(uint8_t *, uint16_t);
typedef void (*dyngl_spi_state_report_cb)(dyngl_state_t);

void dyngl_spi_init(
    dyngl_spi_kb_report_cb kb_report_cb,
    dyngl_spi_consumer_report_cb consumer_report_cb,
    dyngl_spi_state_report_cb state_report_cb
);
void dyngl_spi_run_task();


#endif // DYNGL_SPI_H
