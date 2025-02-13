#include "dyngl_usb.h"
#include "device/usbd.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include <stdint.h>

#define TAG "DYNGL"

#define RID_KEYBOARD 1
#define RID_CONSUMER 2
#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

// Descriptors ------------------------------------------------------------------------------------
//
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER))
};

const char *hid_string_descriptor[5] = {
    (char[]){0x09, 0x04},        // 0: is supported language is English (0x0409)
    "github.com/sadmat",         // 1: Manufacturer
    "DYNGL",                     // 2: Product
    "idkmanwhatever",            // 3: Serials, should use chip ID
    "Bluetooth to USB HID Proxy",// 4: HID
};

static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

// Callbacks --------------------------------------------------------------------------------------

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize) { }

// Init -------------------------------------------------------------------------------------------

void dyngl_usb_init() {
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

void dyngl_usb_send_kb_report(uint8_t modifier_keys, uint8_t *report) {
    if (!tud_mounted()) {
        ESP_LOGD(TAG, "Device not mounted");
        return;
    }
    tud_hid_keyboard_report(RID_KEYBOARD, modifier_keys, report);
}

void dyngl_usb_send_consumer_report(const uint8_t *report, uint16_t len) {
    if (!tud_mounted()) {
        ESP_LOGD(TAG, "Device not mounted");
        return;
    }
    tud_hid_report(RID_CONSUMER, report, len);
}
