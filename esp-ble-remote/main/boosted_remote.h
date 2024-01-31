#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_device.h"
#include "esp_random.h"

typedef struct
{
    esp_attr_value_t *char_val;
    esp_bt_uuid_t descr_uuid;
} gatts_char_descr;

struct gatts_char_profile
{
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    esp_attr_value_t *char_val;
    gatts_char_descr *descr;
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

#define GATTS_TAG "BOOSTED_REMOTE"
#define DEVICE_NAME "BoostedRmt99AF11C6"

// Define all the profiles
#define NUM_PROFILES 4
#define PROFILE_CONTROLS_APP_ID 0
#define PROFILE_CONNECTIVITY_APP_ID 1
#define PROFILE_DEVICE_INFO_APP_ID 2
#define PROFILE_OTAU_APP_ID 3

// Define controls services & characteristics
#define CONTROLS_PROFILE_NUM_CHARS 4
// 1 svc + 4 chars + 4 char vals + 1 desc
#define GATTS_CONTROLS_NUM_HANDLE 10
#define PROFILE_CONTROLS_THROTTLE_ID 0
#define PROFILE_CONTROLS_TRIGGER_ID 1
#define PROFILE_CONTROLS_CHAR1_ID 2
#define PROFILE_CONTROLS_CHAR2_ID 3

// Define device info services & characteristics
#define DEVICE_INFO_PROFILE_NUM_CHARS 6
// 1 svc + 6 chars + 6 char vals + 2 desc
#define GATTS_DEVICE_INFO_NUM_HANDLE 15
#define PROFILE_DEVICE_INFO_CHAR_MODEL_NUM_STR_ID 0
#define PROFILE_DEVICE_INFO_CHAR_SERIAL_NUM_STR_ID 1
#define PROFILE_DEVICE_INFO_CHAR_HW_REVISION_STR_ID 2
#define PROFILE_DEVICE_INFO_CHAR_FW_REVISION_STR_ID 3
#define PROFILE_DEVICE_INFO_CHAR_MANUFACTURER_NAME_STR_ID 4
#define PROFILE_DEVICE_INFO_CHAR_PNP_ID 5

// Define connectivity services & characteristics
#define CONNECTIVITY_PROFILE_NUM_CHARS 6
// 1 svc + 6 chars + 6 char vals + 5 desc
#define GATTS_CONNECTIVITY_NUM_HANDLE 18
#define PROFILE_CONNECTIVITY_CHAR1_ID 0
#define PROFILE_CONNECTIVITY_CHAR2_ID 1
#define PROFILE_CONNECTIVITY_CHAR3_ID 2
#define PROFILE_CONNECTIVITY_CHAR4_ID 3
#define PROFILE_CONNECTIVITY_CHAR5_ID 4
#define PROFILE_CONNECTIVITY_CHAR6_ID 5

// Define over-the-air-updates services & characteristics
#define OTAU_PROFILE_NUM_CHARS 4
// 1 svc + 4 chars + 4 char vals + 1 desc
#define GATTS_OTAU_NUM_HANDLE 10
#define PROFILE_OTAU_CHAR1_ID 0
#define PROFILE_OTAU_CHAR2_ID 1
#define PROFILE_OTAU_CHAR3_ID 2
#define PROFILE_OTAU_CHAR4_ID 3

// Define the callback handler functions
static void gatts_profile_connectivity_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_device_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_controls_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_otau_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void generate_random_address(esp_bd_addr_t rand_addr);
char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req);
