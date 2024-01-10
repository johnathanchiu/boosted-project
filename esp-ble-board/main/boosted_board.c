#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"

#include "boosted_board.h"

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t controller_adv_uuid[] = {0x66, 0x7c, 0x50, 0x17, 0x55, 0x5e, 0x22, 0x8d, 0xe6, 0x11, 0x56, 0x00, 0x2c, 0x77, 0xc4, 0xf4};
static uint8_t raw_adv_data[] = {
    0x02, 0x01, 0x06,                                                                                           // Length 3, Data Type 1 (Flags), Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
    0x11, 0x06, 0x18, 0x3c, 0x48, 0xe0, 0x0b, 0xba, 0x12, 0x99, 0xe5, 0x11, 0x1f, 0xc6, 0x86, 0x5a, 0xc5, 0x7d, // Length 18, DATA_TYPE_SERVICE_UUIDS_128_BIT_PARTIAL
    0x09, 0xFF, 0x02, 0x00, 0x02, 0x05, 0x01, 0xFF, 0xFF, 0xFF,                                                 // Length 10, DATA_TYPE_MANUFACTURER_SPECIFIC_DATA
};
static uint8_t raw_scan_rsp_data[] = {
    0x08, 0xFF, 0x01, 0x2A, 0x0A, 0xA1, 0x69, 0xFF, 0xFF,                                           // Length 9, DATA_TYPE_MANUFACTURER_SPECIFIC_DATA
    0x0F, 0x09, 0x44, 0x65, 0x6C, 0x74, 0x61, 0x20, 0x46, 0x6C, 0x79, 0x65, 0x72, 0x20, 0x49, 0x49, // Length 16, DATA_TYPE_LOCAL_NAME_COMPLETE
};

// Define board info char values
static uint8_t board_ride_modes_val[] = {0x04};
static esp_attr_value_t board_ride_modes_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(board_ride_modes_val),
    .attr_value = board_ride_modes_val,
};
static uint8_t board_current_ride_mode_val[] = {0x03};
static esp_attr_value_t board_current_ride_mode_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(board_current_ride_mode_val),
    .attr_value = board_current_ride_mode_val,
};
static uint8_t board_value_val[] = {0x02};
static esp_attr_value_t board_value_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(board_value_val),
    .attr_value = board_value_val,
};
static uint8_t board_odometry_val[] = {0xF6, 0xCF, 0x5C, 0x00};
static esp_attr_value_t board_odometry_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(board_odometry_val),
    .attr_value = board_odometry_val,
};
static uint8_t board_id_val[] = {0x42, 0x6F, 0x6F, 0x73, 0x74, 0x65, 0x64, 0x42, 0x6F, 0x61, 0x72, 0x64, 0x36, 0x39, 0x41, 0x31, 0x30, 0x41, 0x32, 0x41, 0x00};
static esp_attr_value_t board_id_profile = {
    .attr_max_len = 0x98,
    .attr_len = sizeof(board_id_val),
    .attr_value = board_id_val,
};
static uint8_t board_info_char_1_val[] = {0x4E, 0x6F, 0x6E, 0x65};
static esp_attr_value_t board_info_char_1_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(board_info_char_1_val),
    .attr_value = board_info_char_1_val,
};
static uint8_t board_info_char_2_val[] = {0x4E, 0x6F, 0x6E, 0x65};
static esp_attr_value_t board_info_char_2_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(board_info_char_2_val),
    .attr_value = board_info_char_2_val,
};
static uint8_t board_info_char_3_val[] = {0x00, 0x00};
static esp_attr_value_t board_info_char_3_profile = {
    .attr_max_len = 0x10,
    .attr_len = sizeof(board_info_char_3_val),
    .attr_value = board_info_char_3_val,
};
static uint8_t board_info_char_4_val[] = {0x00, 0x00, 0x00, 0x00};
static esp_attr_value_t board_info_char_4_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(board_info_char_4_val),
    .attr_value = board_info_char_4_val,
};

// Define device info char values
static uint8_t devinfo_model_num_val[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
static esp_attr_value_t devinfo_model_num_char_profile = {
    .attr_max_len = 0x40,
    .attr_len = sizeof(devinfo_model_num_val),
    .attr_value = devinfo_model_num_val,
};
static uint8_t devinfo_manufacturer_name_val[] = {0x42, 0x6F, 0x6F, 0x73, 0x74, 0x65, 0x64, 0x2C, 0x20, 0x49, 0x6E, 0x63, 0x2E};
static esp_attr_value_t devinfo_manufacturer_name_char_profile = {
    .attr_max_len = 0x98,
    .attr_len = sizeof(devinfo_manufacturer_name_val),
    .attr_value = devinfo_manufacturer_name_val,
};
static uint8_t devinfo_hw_revision_val[] = {0x30, 0x30, 0x30, 0x30};
static esp_attr_value_t devinfo_hw_revision_char_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(devinfo_hw_revision_val),
    .attr_value = devinfo_hw_revision_val,
};
static uint8_t devinfo_fw_revision_val[] = {0x76, 0x31, 0x2E, 0x35, 0x2E, 0x32};
static esp_attr_value_t devinfo_fw_revision_char_profile = {
    .attr_max_len = 0x30,
    .attr_len = sizeof(devinfo_fw_revision_val),
    .attr_value = devinfo_fw_revision_val,
};
static uint8_t devinfo_pnp_val[] = {0x01, 0x00, 0x0A, 0x00, 0x4C, 0x01, 0x00, 0x01};
static esp_attr_value_t devinfo_pnp_char_profile = {
    .attr_max_len = 0x40,
    .attr_len = sizeof(devinfo_pnp_val),
    .attr_value = devinfo_pnp_val,
};

// Define battery info char values
static uint8_t battery_soc_val[] = {0x5B};
static esp_attr_value_t battery_soc_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(battery_soc_val),
    .attr_value = battery_soc_val,
};
static uint8_t battery_capacity_val[] = {0x00, 0x9F, 0x24, 0x00};
static esp_attr_value_t battery_capacity_profile = {
    .attr_max_len = 0x20,
    .attr_len = sizeof(battery_capacity_val),
    .attr_value = battery_capacity_val,
};
static uint8_t battery_info_char_1_val[] = {0x00};
static esp_attr_value_t battery_info_char_1_profile = {
    .attr_max_len = 0x01,
    .attr_len = sizeof(battery_info_char_1_val),
    .attr_value = battery_info_char_1_val,
};
static uint8_t battery_info_char_2_val[] = {0x00};
static esp_attr_value_t battery_info_char_2_profile = {
    .attr_max_len = 0x01,
    .attr_len = sizeof(battery_info_char_2_val),
    .attr_value = battery_info_char_2_val,
};

// Define otau char values
static uint8_t otau_char1_val[] = {0x01};
static esp_attr_value_t otau_char1_profile = {
    .attr_max_len = 0x01,
    .attr_len = sizeof(otau_char1_val),
    .attr_value = otau_char1_val,
};
static uint8_t otau_char3_val[] = {0x00};
static esp_attr_value_t otau_char3_profile = {
    .attr_max_len = 0x01,
    .attr_len = sizeof(otau_char3_val),
    .attr_value = otau_char3_val,
};
static uint8_t otau_char4_val[] = {0x07};
static esp_attr_value_t otau_char4_profile = {
    .attr_max_len = 0x01,
    .attr_len = sizeof(otau_char4_val),
    .attr_value = otau_char4_val,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_RPA_RANDOM,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_RPA_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
};

// Define all the services
static struct gatts_profile_inst gl_profile_tab[NUM_PROFILES] = {
    [PROFILE_SERVICE_1_APP_ID] = {
        .gatts_cb = gatts_profile_service_1_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0xE5, 0x22, 0x8D, 0xE6, 0x11, 0x65, 0x00, 0xE2, 0x60, 0x85, 0x58},
                    },
                },
            },
        },
    },
    [PROFILE_OTAU_APP_ID] = {
        .gatts_cb = gatts_profile_otau_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */

        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x16, 0x10, 0x00, 0x00},
                    },
                },
            },
        },
    },
    [PROFILE_BATTERY_INFO_APP_ID] = {
        .gatts_cb = gatts_profile_battery_info_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xA8, 0xEA, 0xA8, 0x65},
                    },
                },
            },
        },
    },
    [PROFILE_BOARD_INFO_APP_ID] = {
        .gatts_cb = gatts_profile_board_info_event_handler,
        .gattc_cb = gattc_profile_board_info_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x86, 0x5A, 0xC5, 0x7D},
                    },
                },
            },
        },
    },
    [PROFILE_DEVICE_INFO_APP_ID] = {
        .gatts_cb = gatts_profile_device_info_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {
                        .uuid16 = 0x180A,
                    },
                },
            },
        },
    },
};

// Define battery info char profiles
static struct gatts_char_profile gl_battery_info_svc_char_profile_tab[BATTERY_INFO_PROFILE_NUM_CHARS] = {
    [PROFILE_BATTERY_SOC_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xAE, 0xEE, 0xA8, 0x65},
            },
        },
        .char_val = &battery_soc_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BATTERY_CAPACITY_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xC2, 0xF3, 0xA8, 0x65},
            },
        },
        .char_val = &battery_capacity_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_BATTERY_1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xD4, 0xF5, 0xA8, 0x65},
            },
        },
        .char_val = &battery_info_char_1_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BATTERY_2_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x32, 0xF8, 0xA8, 0x65},
            },
        },
        .char_val = &battery_info_char_2_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
};

// Define board info char profiles
static struct gatts_char_profile gl_board_info_svc_char_profile_tab[BOARD_INFO_PROFILE_NUM_CHARS] = {
    [PROFILE_BOARD_VALUE_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x43, 0x96, 0xC5, 0x7D},
            },
        },
        .char_val = &board_value_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_BOARD_ID_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x39, 0xBB, 0xC5, 0x7D},
            },
        },
        .char_val = &board_id_profile,
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_BOARD_ODOMETRY_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x94, 0x65, 0xC5, 0x7D},
            },
        },
        .char_val = &board_odometry_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BOARD_RIDE_MODES_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xEC, 0x5D, 0xC5, 0x7D},
            },
        },
        .char_val = &board_ride_modes_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_BOARD_CURRENT_RIDE_MODE_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x22, 0x5F, 0xC5, 0x7D},
            },
        },
        .char_val = &board_current_ride_mode_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BOARD_1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x66, 0x66, 0xC5, 0x7D},
            },
        },
        .char_val = &board_info_char_1_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_BOARD_2_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x86, 0x69, 0xC5, 0x7D},
            },
        },
        .char_val = &board_info_char_2_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_BOARD_3_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x34, 0x6B, 0xC5, 0x7D},
            },
        },
        .char_val = &board_info_char_3_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BOARD_4_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xFC, 0x6B, 0xC5, 0x7D},
            },
        },
        .char_val = &board_info_char_4_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_BOARD_5_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0xEC, 0x73, 0xC5, 0x7D},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
};

// Define device info char profiles
static struct gatts_char_profile gl_dinfo_svc_char_profile_tab[DEVICE_INFO_PROFILE_NUM_CHARS] = {
    [PROFILE_DEVICE_INFO_CHAR_MODEL_NUM_STR_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A24,
            },
        },
        .char_val = &devinfo_model_num_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_DEVICE_INFO_CHAR_MANUFACTURER_NAME_STR_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A29,
            },
        },
        .char_val = &devinfo_manufacturer_name_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_DEVICE_INFO_CHAR_HW_REVISION_STR_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A27,
            },
        },
        .char_val = &devinfo_hw_revision_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_DEVICE_INFO_CHAR_FW_REVISION_STR_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A26,
            },
        },
        .char_val = &devinfo_fw_revision_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_DEVICE_INFO_CHAR_PNP_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A50,
            },
        },
        .char_val = &devinfo_pnp_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
};

// Define service 1 char profiles
static struct gatts_char_profile gl_svc_1_char_profile_tab[SERVICE_1_PROFILE_NUM_CHARS] = {
    [PROFILE_SERVICE_1_1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0xE5, 0x22, 0x8D, 0xE6, 0x11, 0x65, 0x00, 0x24, 0x65, 0x85, 0x58},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
};

// Define service 2 char profiles
static struct gatts_char_profile gl_svc_2_char_profile_tab[OTAU_PROFILE_NUM_CHARS] = {
    [PROFILE_OTAU_CHAR1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x13, 0x10, 0x00, 0x00},
            },
        },
        .char_val = &otau_char1_profile,
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_OTAU_CHAR2_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x18, 0x10, 0x00, 0x00},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_OTAU_CHAR3_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x14, 0x10, 0x00, 0x00},
            },
        },
        .char_val = &otau_char3_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
    },
    [PROFILE_OTAU_CHAR4_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x11, 0x10, 0x00, 0x00},
            },
        },
        .char_val = &otau_char4_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
};

static const char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
    const char *key_str = NULL;
    switch (key_type)
    {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;
    }
    return key_str;
}

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
    char *auth_str = NULL;
    switch (auth_req)
    {
    case ESP_LE_AUTH_NO_BOND:
        auth_str = "ESP_LE_AUTH_NO_BOND";
        break;
    case ESP_LE_AUTH_BOND:
        auth_str = "ESP_LE_AUTH_BOND";
        break;
    case ESP_LE_AUTH_REQ_MITM:
        auth_str = "ESP_LE_AUTH_REQ_MITM";
        break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
        auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
        auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
        break;
    case ESP_LE_AUTH_REQ_SC_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
        break;
    default:
        auth_str = "INVALID BLE AUTH REQ";
        break;
    }

    return auth_str;
}

static void gatts_profile_battery_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_BATTERY_INFO_APP_ID].service_id, GATTS_BATTERY_INFO_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_BATTERY_INFO_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_BATTERY_INFO_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < BATTERY_INFO_PROFILE_NUM_CHARS; idx++)
        {
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_BATTERY_INFO_APP_ID].service_handle,
                                                            &gl_battery_info_svc_char_profile_tab[idx].char_uuid,
                                                            gl_battery_info_svc_char_profile_tab[idx].perm,
                                                            gl_battery_info_svc_char_profile_tab[idx].property,
                                                            gl_battery_info_svc_char_profile_tab[idx].char_val,
                                                            (gl_battery_info_svc_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
                                                                                                                               .auto_rsp = ESP_GATT_AUTO_RSP,
                                                                                                                           }
                                                                                                                         : NULL);
            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    default:
        break;
    }
}

static void gatts_profile_board_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].service_id, GATTS_BOARD_INFO_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < BOARD_INFO_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_board_info_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].service_handle,
                                                            &gl_board_info_svc_char_profile_tab[idx].char_uuid,
                                                            gl_board_info_svc_char_profile_tab[idx].perm,
                                                            gl_board_info_svc_char_profile_tab[idx].property,
                                                            gl_board_info_svc_char_profile_tab[idx].char_val,
                                                            (gl_board_info_svc_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
                                                                                                                             .auto_rsp = ESP_GATT_AUTO_RSP,
                                                                                                                         }
                                                                                                                       : NULL);
            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    default:
        break;
    }
}

static void gatts_profile_device_info_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_DEVICE_INFO_APP_ID].service_id, GATTS_DEVICE_INFO_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_DEVICE_INFO_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_DEVICE_INFO_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < DEVICE_INFO_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_dinfo_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_DEVICE_INFO_APP_ID].service_handle,
                                                            &gl_dinfo_svc_char_profile_tab[idx].char_uuid,
                                                            gl_dinfo_svc_char_profile_tab[idx].perm,
                                                            gl_dinfo_svc_char_profile_tab[idx].property,
                                                            gl_dinfo_svc_char_profile_tab[idx].char_val,
                                                            &(esp_attr_control_t){
                                                                .auto_rsp = ESP_GATT_AUTO_RSP,
                                                            });

            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    default:
        break;
    }
}

static void gatts_profile_service_1_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
        if (set_dev_name_ret)
        {
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret)
        {
            ESP_LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret)
        {
            ESP_LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_SERVICE_1_APP_ID].service_id, GATTS_SERVICE_1_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_SERVICE_1_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_SERVICE_1_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < SERVICE_1_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_conn_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_SERVICE_1_APP_ID].service_handle,
                                                            &gl_svc_1_char_profile_tab[idx].char_uuid,
                                                            gl_svc_1_char_profile_tab[idx].perm,
                                                            gl_svc_1_char_profile_tab[idx].property,
                                                            gl_svc_1_char_profile_tab[idx].char_val,
                                                            (gl_svc_1_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
                                                                                                                    .auto_rsp = ESP_GATT_AUTO_RSP,
                                                                                                                }
                                                                                                              : NULL);

            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    default:
        break;
    }
}

static void gatts_profile_otau_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_OTAU_APP_ID].service_id, GATTS_OTAU_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_OTAU_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_OTAU_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < OTAU_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_svc_2_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_OTAU_APP_ID].service_handle,
                                                            &gl_svc_2_char_profile_tab[idx].char_uuid,
                                                            gl_svc_2_char_profile_tab[idx].perm,
                                                            gl_svc_2_char_profile_tab[idx].property,
                                                            gl_svc_2_char_profile_tab[idx].char_val,
                                                            (gl_svc_2_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
                                                                                                                    .auto_rsp = ESP_GATT_AUTO_RSP,
                                                                                                                }
                                                                                                              : NULL);

            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    default:
        break;
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_svc_uuid = NULL;
    uint8_t adv_uuid_len = 0;
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            ESP_LOGI(GATTS_TAG, "advertisting data set!");
            esp_ble_gap_start_advertising(&adv_params);
            ESP_LOGI(GATTS_TAG, "advertising started!");
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "advertisting data set!");
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            ESP_LOGI(GATTS_TAG, "scan response data set!");
            esp_ble_gap_start_advertising(&adv_params);
            ESP_LOGI(GATTS_TAG, "advertising started!");
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "scan response data set!");
        }
        break;
    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "config local privacy failed, error code =%x", param->local_privacy_cmpl.status);
            break;
        }
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret)
        {
            ESP_LOGE(GATTS_TAG, "set scan params error, error code = %x", scan_ret);
        }
        break;
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        // the unit of the duration is second
        uint32_t duration = 30;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "Scan start success");
        break;
    case ESP_GAP_BLE_PASSKEY_REQ_EVT: /* passkey request event */
        /* Call the following function to input the passkey which is displayed on the remote device */
        // esp_ble_passkey_reply(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, true, 0x00);
        ESP_LOGI(GATTS_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT:
    {
        ESP_LOGI(GATTS_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        uint8_t tk[16] = {1}; // If you paired with OOB, both devices need to use the same tk
        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
        break;
    }
    case ESP_GAP_BLE_LOCAL_IR_EVT: /* BLE local IR event */
        ESP_LOGI(GATTS_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_ER_EVT: /* BLE local ER event */
        ESP_LOGI(GATTS_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* send the positive(true) security response to the peer device to accept the security request.
        If not accept the security request, should send the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
        show the passkey number to the user to confirm it with the number displayed by peer device. */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(GATTS_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%" PRIu32, param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: /// the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
        /// show the passkey number to the user to input it in the peer device.
        ESP_LOGI(GATTS_TAG, "The passkey Notify number:%06" PRIu32, param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_KEY_EVT:
        // shows the ble key info share with peer device to the user.
        ESP_LOGI(GATTS_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
    {
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TAG, "remote BD_ADDR: %08x%04x",
                 (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                 (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(GATTS_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(GATTS_TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
        if (!param->ble_security.auth_cmpl.success)
        {
            ESP_LOGI(GATTS_TAG, "fail reason = 0x%x", param->ble_security.auth_cmpl.fail_reason);
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "auth mode = %s", esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            adv_svc_uuid = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                    ESP_BLE_AD_TYPE_128SRV_PART, &adv_uuid_len);
            if (adv_uuid_len == 16)
            {
                // ESP_LOGI(GATTS_TAG, "Found a device with the UUID %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                //          adv_svc_uuid[15],
                //          adv_svc_uuid[14],
                //          adv_svc_uuid[13],
                //          adv_svc_uuid[12],
                //          adv_svc_uuid[11],
                //          adv_svc_uuid[10],
                //          adv_svc_uuid[9],
                //          adv_svc_uuid[8],
                //          adv_svc_uuid[7],
                //          adv_svc_uuid[6],
                //          adv_svc_uuid[5],
                //          adv_svc_uuid[4],
                //          adv_svc_uuid[3],
                //          adv_svc_uuid[2],
                //          adv_svc_uuid[1],
                //          adv_svc_uuid[0]);
                if (memcmp(adv_svc_uuid, controller_adv_uuid, sizeof(controller_adv_uuid)) == 0)
                {
                    ESP_LOGI(GATTS_TAG, "Found the desired service UUID");
                    if (scan_result->scan_rst.ble_adv[23] == 0x02)
                    {
                        ESP_LOGI(GATTS_TAG, "Remote is in pairing mode!");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                    }
                    else
                    {
                        ESP_LOGI(GATTS_TAG, "Remote is not currently in pairing mode.");
                    }
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }
    default:
        ESP_LOGI(GATTS_TAG, "got some potential information %d...", event);
        break;
    }
}

static void gattc_profile_board_info_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event)
    {
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTS_TAG, "open failed, error status = %x", p_data->open.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "open success");
        gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].conn_id = p_data->open.conn_id;
        memcpy(gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].remote_bda, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTS_TAG, gl_profile_tab[PROFILE_BOARD_INFO_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req(gattc_if, p_data->open.conn_id);
        if (mtu_ret)
        {
            ESP_LOGE(GATTS_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    default:
        ESP_LOGI(GATTS_TAG, "GATTC got some potential information %d...", event);
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    int idx;
    for (idx = 0; idx < NUM_PROFILES; idx++)
    {
        if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            gatts_if == gl_profile_tab[idx].gatts_if)
        {
            if (gl_profile_tab[idx].gatts_cb)
            {
                gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
            }
        }
    }
}

static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    int idx;
    for (idx = 0; idx < NUM_PROFILES; idx++)
    {
        if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            gattc_if == gl_profile_tab[idx].gattc_if)
        {
            if (gl_profile_tab[idx].gattc_cb)
            {
                gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
            }
        }
    }
}

void app_main()
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed", __func__);
        return;
    }
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed", __func__);
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed", __func__);
        return;
    }

    ret = esp_ble_gap_config_local_privacy(true);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "privacy set failed, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gattc_register_callback(gattc_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s gattc register error, error code = %x", __func__, ret);
        return;
    }

    ret = esp_ble_gattc_app_register(PROFILE_BOARD_INFO_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_SERVICE_1_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_OTAU_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_BOARD_INFO_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_BATTERY_INFO_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_DEVICE_INFO_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(512);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
        return;
    }

    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND; // bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;       // set the IO capability to No output No input
    uint8_t key_size = 16;                          // the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK | ESP_BLE_LINK_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK | ESP_BLE_LINK_KEY_MASK;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}