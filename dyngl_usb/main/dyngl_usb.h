#ifndef DYNGL_USB_H
#define DYNGL_USB_H

#include <stdint.h>
void dyngl_usb_init();
void dyngl_usb_send_kb_report(uint8_t modifier_keys, uint8_t *report);
void dyngl_usb_send_consumer_report(const uint8_t *report, uint16_t len);

#endif // DYGNL_USB_H
