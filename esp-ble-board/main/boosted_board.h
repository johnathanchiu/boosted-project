#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

struct gatts_char_profile
{
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    esp_attr_value_t *char_val;
};

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;

    // uint16_t conn_id;
    // uint16_t descr_handle;
    // esp_bt_uuid_t descr_uuid;
};

#define GATTS_TAG "BOOSTED_BOARD"
#define DEVICE_NAME "BoostedBoard69A10A2A"

// Define all the profiles
#define NUM_PROFILES 5
#define PROFILE_SERVICE_1_APP_ID 0
#define PROFILE_SERVICE_2_APP_ID 1
#define PROFILE_BATTERY_INFO_APP_ID 2
#define PROFILE_BOARD_INFO_APP_ID 3
#define PROFILE_DEVICE_INFO_APP_ID 4

#define BOARD_INFO_PROFILE_NUM_CHARS 10
#define GATTS_BOARD_INFO_NUM_HANDLE 21
#define PROFILE_BOARD_VALUE_ID 0
#define PROFILE_BOARD_ID_ID 1
#define PROFILE_BOARD_ODOMETRY_ID 2
#define PROFILE_BOARD_RIDE_MODES_ID 3
#define PROFILE_BOARD_CURRENT_RIDE_MODE_ID 4
#define PROFILE_BOARD_1_ID 5
#define PROFILE_BOARD_2_ID 6
#define PROFILE_BOARD_3_ID 7
#define PROFILE_BOARD_4_ID 8
#define PROFILE_BOARD_5_ID 9

#define BATTERY_INFO_PROFILE_NUM_CHARS 4
#define GATTS_BATTERY_INFO_NUM_HANDLE 9
#define PROFILE_BATTERY_SOC_ID 0
#define PROFILE_BATTERY_CAPACITY_ID 1
#define PROFILE_BATTERY_1_ID 2
#define PROFILE_BATTERY_2_ID 3

#define SERVICE_1_PROFILE_NUM_CHARS 1
#define GATTS_SERVICE_1_NUM_HANDLE 3
#define PROFILE_SERVICE_1_1_ID 0

#define SERVICE_2_PROFILE_NUM_CHARS 4
#define GATTS_SERVICE_2_NUM_HANDLE 9
#define PROFILE_SERVICE_2_1_ID 0
#define PROFILE_SERVICE_2_2_ID 1
#define PROFILE_SERVICE_2_3_ID 2
#define PROFILE_SERVICE_2_4_ID 3

#define DEVICE_INFO_PROFILE_NUM_CHARS 5
#define GATTS_DEVICE_INFO_NUM_HANDLE 11
#define PROFILE_DEVICE_INFO_CHAR_MODEL_NUM_STR_ID 0
#define PROFILE_DEVICE_INFO_CHAR_MANUFACTURER_NAME_STR_ID 1
#define PROFILE_DEVICE_INFO_CHAR_HW_REVISION_STR_ID 2
#define PROFILE_DEVICE_INFO_CHAR_FW_REVISION_STR_ID 3
#define PROFILE_DEVICE_INFO_CHAR_PNP_ID 4

// Define the callback handler functions
static void gatts_profile_battery_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_board_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_device_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_service_1_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_service_2_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// void print_char_profile(const struct gatts_char_profile *char_profile);