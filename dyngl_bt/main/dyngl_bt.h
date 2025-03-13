#ifndef DYNGL_BT_H
#define DYNGL_BT_H

#include <stdint.h>
#include "dyngl_common.h"

typedef void (*dyngl_bt_kb_report_callback)(uint8_t *, uint16_t);
typedef void (*dyngl_bt_consumer_report_callback)(uint8_t *, uint16_t);
typedef void (*dyngl_bt_state_chg_callback)(dyngl_state_t);

void dyngl_bt_init(
    dyngl_bt_kb_report_callback kb_report_callback,
    dyngl_bt_consumer_report_callback consumer_report_callback,
    dyngl_bt_state_chg_callback state_chg_callback,
    const char *kb_addr
);
void dyngl_bt_pairing_mode();


#endif // DYNGL_BT_H
