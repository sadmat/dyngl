#include "dyngl_bt.h"
#include "dyngl_common.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_gap_bt_api.h"
#include "esp_hid_common.h"
#include "esp_hidh.h"
#include "esp_hidh_bluedroid.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "DYNGL"
#define ADDR_STR_LEN (ESP_BD_ADDR_LEN * 2 + 5)

void start_scan_if_needed();
esp_err_t str_to_addr(const char *str, uint8_t *addr);
void esp_hidh_callback(void *arg, esp_event_base_t event_base, int32_t event_id,
                       void *event_data);
void esp_bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
const char *gap_bt_evt_name(esp_bt_gap_cb_event_t event);

static esp_bd_addr_t keyboard_addr = { 0x00 };
static dyngl_bt_kb_report_callback kb_report_cb = NULL;
static dyngl_bt_consumer_report_callback consumer_report_cb = NULL;
static dyngl_bt_state_chg_callback state_chg_cb = NULL;

void dyngl_bt_init(
    dyngl_bt_kb_report_callback kb_report_callback,
    dyngl_bt_consumer_report_callback consumer_report_callback,
    dyngl_bt_state_chg_callback state_chg_callback,
    const char *kb_addr
) {
    esp_bt_controller_config_t bt_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bluedroid_config_t bluedroid_config = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    esp_hidh_config_t config = {
        .callback = esp_hidh_callback,
        .event_stack_size = 4096,
        .callback_arg = NULL,
    };

    kb_report_cb = kb_report_callback;
    consumer_report_cb = consumer_report_callback;
    state_chg_cb = state_chg_callback;

    ESP_ERROR_CHECK(str_to_addr(kb_addr, (uint8_t *)&keyboard_addr));
    ESP_LOGI(TAG, "Initializing BT module with kb addr: " ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(keyboard_addr));

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_config));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(esp_bluedroid_init_with_cfg(&bluedroid_config));
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_bt_gap_register_callback(esp_bt_gap_callback));
    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE));
    ESP_ERROR_CHECK(esp_hidh_init(&config));
}

void dyngl_bt_pairing_mode() {
    int dev_num = esp_bt_gap_get_bond_device_num();
    esp_bd_addr_t *addrs = malloc(sizeof(esp_bd_addr_t) * dev_num);
    esp_bt_gap_get_bond_device_list(&dev_num, addrs);
    for (int i = 0; i < dev_num; ++i) {
        esp_bt_gap_remove_bond_device(addrs[i]);
    }
    free(addrs);

    ESP_LOGI(TAG, "Paired devices removed. Scanning...");
    ESP_ERROR_CHECK(esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0));
    state_chg_cb(DYNGL_STATE_PAIRING);
}

void start_scan_if_needed() {
    bool kb_found = false;
    int dev_num = esp_bt_gap_get_bond_device_num();
    esp_bd_addr_t *addrs = malloc(sizeof(esp_bd_addr_t) * dev_num);
    esp_bt_gap_get_bond_device_list(&dev_num, addrs);
    for (int i = 0; i < dev_num; ++i) {
        if (!memcmp(&addrs[i], &keyboard_addr, ESP_BD_ADDR_LEN)) {
            kb_found = true;
            break;
        }
    }
    free(addrs);

    if (!kb_found) {
        ESP_LOGI(TAG, "Keyboard not yet paired. Scanning...");
        ESP_ERROR_CHECK(esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0));
        state_chg_cb(DYNGL_STATE_PAIRING);
    } else {
        ESP_LOGI(TAG, "Keyboard already paired. Waiting for connection...");
        state_chg_cb(DYNGL_STATE_IDLE);
    }
}

esp_err_t str_to_addr(const char *str, uint8_t *addr) {
    if (!str || !addr) {
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(str) != ADDR_STR_LEN) {
        return ESP_ERR_INVALID_MAC;
    }

    for (int i = 0; i < ESP_BD_ADDR_LEN; ++i) {
        if (!isxdigit((unsigned char)str[i * 3]) || !isxdigit((unsigned char)str[i * 3 + 1])) {
            return ESP_ERR_INVALID_MAC;
        }

        char part[3] = { str[i * 3], str[i * 3 + 1], 0 };
        addr[i] = (uint8_t)strtol(part, NULL, 16);

        if (i < ESP_BD_ADDR_LEN - 1 && str[i * 3 + 2] != ':') {
            return ESP_ERR_INVALID_MAC;
        }
    }

    return ESP_OK;
}

void esp_hidh_callback(void *arg, esp_event_base_t event_base, int32_t event_id,
                       void *event_data) {
    esp_hidh_event_t event = (esp_hidh_event_t)(event_id);
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)(event_data);

    switch (event) {
        case ESP_HIDH_OPEN_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_OPEN_EVENT");
            state_chg_cb(DYNGL_STATE_CONNECTED);
            break;
        case ESP_HIDH_BATTERY_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_BATTERY_EVENT");
            break;
        case ESP_HIDH_INPUT_EVENT: {
            ESP_LOGD(TAG, "ESP_HIDH_INPUT_EVENT");
            if (param->input.usage == ESP_HID_USAGE_KEYBOARD) {
                if (kb_report_cb) {
                    kb_report_cb(param->input.data, param->input.length);
                }
            }
            else if (param->input.usage == ESP_HID_USAGE_CCONTROL) {
                if (consumer_report_cb) {
                    consumer_report_cb(param->input.data, param->input.length);
                }
            } else {
                ESP_LOGD(TAG, "Unsupported report received: %d", param->input.usage);
            }
            break;
        }
        case ESP_HIDH_FEATURE_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_FEATURE_EVENT");
            break;
        case ESP_HIDH_CLOSE_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_CLOSE_EVENT");
            state_chg_cb(DYNGL_STATE_IDLE);
            break;
        case ESP_HIDH_START_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_START_EVENT, status: %d", param->start.status);
            start_scan_if_needed();
            break;
        case ESP_HIDH_STOP_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_STOP_EVENT");
            break;
        case ESP_HIDH_CONN_REQUEST_EVENT:
            ESP_LOGD(TAG, "ESP_HIDH_CONN_REQUEST_EVENT");
            break;
        default:
            break;
    }
}

void esp_bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    ESP_LOGD(TAG, "bt_gap_callback event: %s", gap_bt_evt_name(event));
    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT: {
            if (memcmp(param->disc_res.bda, &keyboard_addr, ESP_BD_ADDR_LEN) == 0) {
                ESP_LOGI(TAG, "Keyboard found, connecting");
                ESP_ERROR_CHECK(esp_bt_gap_cancel_discovery());
                esp_hidh_dev_open(param->disc_res.bda, ESP_HID_TRANSPORT_BT, 0);
            }
            break;
        }
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
            bool discovery_stopped = param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED;
            ESP_LOGD(TAG, "disovery stopped: %s", discovery_stopped ? "true" : "false");
            // TODO: restart whe not paired?
            break;
        }
        case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT: {
            ESP_LOGD(TAG, "Connection status: %d", param->acl_conn_cmpl_stat.stat);
            break;
        }
        case ESP_BT_GAP_ENC_CHG_EVT: {
            ESP_LOGD(TAG, "Encryption type: %d", param->enc_chg.enc_mode);
            break;
        }
        case ESP_BT_GAP_MODE_CHG_EVT: {
            ESP_LOGD(TAG, "Mode: %d", param->mode_chg.mode);
            break;
        }
        default:
            break;
    }
}

const char *gap_bt_evt_name(esp_bt_gap_cb_event_t event) {
    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT: return "ESP_BT_GAP_DISC_RES_EVT";
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: return "ESP_BT_GAP_DISC_STATE_CHANGED_EVT";
        case ESP_BT_GAP_RMT_SRVCS_EVT: return "ESP_BT_GAP_RMT_SRVCS_EVT";
        case ESP_BT_GAP_RMT_SRVC_REC_EVT: return "ESP_BT_GAP_RMT_SRVC_REC_EVT";
        case ESP_BT_GAP_AUTH_CMPL_EVT: return "ESP_BT_GAP_AUTH_CMPL_EVT";
        case ESP_BT_GAP_PIN_REQ_EVT: return "ESP_BT_GAP_PIN_REQ_EVT";
        case ESP_BT_GAP_CFM_REQ_EVT: return "ESP_BT_GAP_CFM_REQ_EVT";
        case ESP_BT_GAP_KEY_NOTIF_EVT: return "ESP_BT_GAP_KEY_NOTIF_EVT";
        case ESP_BT_GAP_KEY_REQ_EVT: return "ESP_BT_GAP_KEY_REQ_EVT";
        case ESP_BT_GAP_READ_RSSI_DELTA_EVT: return "ESP_BT_GAP_READ_RSSI_DELTA_EVT";
        case ESP_BT_GAP_CONFIG_EIR_DATA_EVT: return "ESP_BT_GAP_CONFIG_EIR_DATA_EVT";
        case ESP_BT_GAP_SET_AFH_CHANNELS_EVT: return "ESP_BT_GAP_SET_AFH_CHANNELS_EVT";
        case ESP_BT_GAP_READ_REMOTE_NAME_EVT: return "ESP_BT_GAP_READ_REMOTE_NAME_EVT";
        case ESP_BT_GAP_MODE_CHG_EVT: return "ESP_BT_GAP_MODE_CHG_EVT";
        case ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT: return "ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT";
        case ESP_BT_GAP_QOS_CMPL_EVT: return "ESP_BT_GAP_QOS_CMPL_EVT";
        case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT: return "ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT";
        case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT: return "ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT";
        case ESP_BT_GAP_SET_PAGE_TO_EVT: return "ESP_BT_GAP_SET_PAGE_TO_EVT";
        case ESP_BT_GAP_GET_PAGE_TO_EVT: return "ESP_BT_GAP_GET_PAGE_TO_EVT";
        case ESP_BT_GAP_ACL_PKT_TYPE_CHANGED_EVT: return "ESP_BT_GAP_ACL_PKT_TYPE_CHANGED_EVT";
        case ESP_BT_GAP_ENC_CHG_EVT: return "ESP_BT_GAP_ENC_CHG_EVT";
        case ESP_BT_GAP_SET_MIN_ENC_KEY_SIZE_EVT: return "ESP_BT_GAP_SET_MIN_ENC_KEY_SIZE_EVT";
        case ESP_BT_GAP_GET_DEV_NAME_CMPL_EVT: return "ESP_BT_GAP_GET_DEV_NAME_CMPL_EVT";
        case ESP_BT_GAP_EVT_MAX: return "ESP_BT_GAP_EVT_MAX";
        default: return "UNKNOWN event";
    }
}

