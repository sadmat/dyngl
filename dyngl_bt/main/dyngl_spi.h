#ifndef DYNGL_SPI_H
#define DYNGL_SPI_H

#include <stdint.h>
#include "dyngl_common.h"

void dyngl_spi_init();
void dyngl_spi_send_kb_report(const uint8_t *report, uint16_t len);
void dyngl_spi_send_consumer_report(const uint8_t *report, uint16_t len);
void dyngl_spi_send_state_chg(dyngl_state_t new_state);

#endif // DYNGL_SPI_H
