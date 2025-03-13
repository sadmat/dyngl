#ifndef DYNGL_COMMON_H
#define DYNGL_COMMON_H

#include <stdint.h>

#define DYNGL_KB_REPORT_LEN       6
#define DYNGL_CONSUMER_REPORT_LEN 2

typedef enum {
    DYNGL_REPORT_KEYBOARD,
    DYNGL_REPORT_CONSUMER,
    DYNGL_REPORT_STATE_CHG
} dyngl_report_type_e;

typedef struct {
    uint8_t modifier_keys;
    uint8_t report[DYNGL_KB_REPORT_LEN];
} dyngl_keyboard_report_t;

typedef struct {
    uint8_t report[DYNGL_CONSUMER_REPORT_LEN];
} dyngl_consumer_report_t;

typedef enum {
    DYNGL_STATE_PAIRING,
    DYNGL_STATE_IDLE,
    DYNGL_STATE_CONNECTED
} dyngl_state_t;

typedef struct {
    dyngl_report_type_e report_type;

    union {
        dyngl_keyboard_report_t keyboard;
        dyngl_consumer_report_t consumer;
        dyngl_state_t state;
    } report;
} dyngl_message_t;

#endif // DYNGL_COMMON_H
