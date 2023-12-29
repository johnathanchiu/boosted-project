#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"

#include "example_ble_sec_gatts_demo.h"

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t raw_adv_data[] = {
    0x02, 0x01, 0x06,                                                                                           // Length 3, Data Type 1 (Flags), Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
    0x11, 0x06, 0x18, 0x3C, 0x48, 0xE0, 0x0B, 0xBA, 0x12, 0x99, 0xE5, 0x11, 0x1F, 0xC6, 0x86, 0x5A, 0xC5, 0x7D, // Length 18, DATA_TYPE_SERVICE_UUIDS_128_BIT_PARTIAL
    0x09, 0xFF, 0x02, 0x00, 0x03, 0x03, 0x02, 0xFF, 0xFF, 0xFF,                                                 // Length 10, DATA_TYPE_MANUFACTURER_SPECIFIC_DATA
};
static uint8_t raw_scan_rsp_data[] = {
    0x08, 0xFF, 0x00, 0xC6, 0x11, 0xAF, 0x99, 0xFF, 0xFF,                                                                   // Length 9, DATA_TYPE_MANUFACTURER_SPECIFIC_DATA
    0x13, 0x09, 0x42, 0x6F, 0x6F, 0x73, 0x74, 0x65, 0x64, 0x52, 0x6D, 0x74, 0x39, 0x39, 0x41, 0x46, 0x31, 0x31, 0x43, 0x36, // Length 20, DATA_TYPE_LOCAL_NAME_COMPLETE
    0x00, 0x00,                                                                                                             // Length 2, Padding
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

// Define controls char values
static uint8_t controls_throttle_val[] = {0x00, 0x00, 0x00, 0x00};
static esp_attr_value_t controls_throttle_char_profile = {
    .attr_max_len = 0x04,
    .attr_len = sizeof(controls_throttle_val),
    .attr_value = controls_throttle_val,
};
static uint8_t controls_trigger_val[] = {0x00, 0x00};
static esp_attr_value_t controls_trigger_char_profile = {
    .attr_max_len = 0x02,
    .attr_len = sizeof(controls_trigger_val),
    .attr_value = controls_trigger_val,
};
static uint8_t controls_char2_val[] = {0xF5, 0x02, 0x01, 0x00, 0x00, 0x00, 0xC0, 0x2F, 0x00, 0x00, 0xE3, 0x30, 0x00, 0x00};
static esp_attr_value_t controls_char2_profile = {
    .attr_max_len = 0x0E,
    .attr_len = sizeof(controls_char2_val),
    .attr_value = controls_char2_val,
};

// Define device info char values
static uint8_t devinfo_model_num_val[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
static esp_attr_value_t devinfo_model_num_char_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(devinfo_model_num_val),
    .attr_value = devinfo_model_num_val,
};
static uint8_t devinfo_manufacturer_name_val[] = {0x42, 0x6F, 0x6F, 0x73, 0x74, 0x65, 0x64, 0x2C, 0x20, 0x49, 0x6E, 0x63, 0x2E};
static esp_attr_value_t devinfo_manufacturer_name_char_profile = {
    .attr_max_len = 0x0D,
    .attr_len = sizeof(devinfo_manufacturer_name_val),
    .attr_value = devinfo_manufacturer_name_val,
};
static uint8_t devinfo_serial_num_val[] = {0x39, 0x39, 0x41, 0x46, 0x31, 0x31, 0x43, 0x36};
static esp_attr_value_t devinfo_serial_num_char_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(devinfo_serial_num_val),
    .attr_value = devinfo_serial_num_val,
};
static uint8_t devinfo_hw_revision_val[] = {0x30, 0x31, 0x38, 0x39, 0x43, 0x37, 0x34, 0x31};
static esp_attr_value_t devinfo_hw_revision_char_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(devinfo_hw_revision_val),
    .attr_value = devinfo_hw_revision_val,
};
static uint8_t devinfo_fw_revision_val[] = {0x76, 0x32, 0x2E, 0x33, 0x2E, 0x33};
static esp_attr_value_t devinfo_fw_revision_char_profile = {
    .attr_max_len = 0x06,
    .attr_len = sizeof(devinfo_fw_revision_val),
    .attr_value = devinfo_fw_revision_val,
};
static uint8_t devinfo_pnp_val[] = {0x01, 0x00, 0x0A, 0x00, 0x4C, 0x01, 0x00, 0x01};
static esp_attr_value_t devinfo_pnp_char_profile = {
    .attr_max_len = 0x08,
    .attr_len = sizeof(devinfo_pnp_val),
    .attr_value = devinfo_pnp_val,
};

// Define connectivity char values
static uint8_t connectivity_char2_val[] = {0x42, 0x6F, 0x6F, 0x73, 0x74, 0x65, 0x64, 0x52, 0x6D, 0x74, 0x39, 0x39, 0x41, 0x46, 0x31, 0x31, 0x43, 0x36, 0x00};
static esp_attr_value_t connectivity_char2_profile = {
    .attr_max_len = 0x98,
    .attr_len = sizeof(connectivity_char2_val),
    .attr_value = connectivity_char2_val,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Define all the services
static struct gatts_profile_inst gl_profile_tab[NUM_PROFILES] = {
    [PROFILE_CONNECTIVITY_APP_ID] = {
        .gatts_cb = gatts_profile_connectivity_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x2C, 0x77, 0xC4, 0xF4},
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
    [PROFILE_CONTROLS_APP_ID] = {
        .gatts_cb = gatts_profile_controls_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .service_id = {
            .is_primary = true,
            .id = {
                .inst_id = 0x00,
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = {0x78, 0xFE, 0xDE, 0x05, 0x1D, 0x3E, 0x48, 0xA1, 0xE6, 0x11, 0xD4, 0x0C, 0xA0, 0x5D, 0xC0, 0xAF},
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
};

// Define otau char profiles
static struct gatts_char_profile gl_otau_svc_char_profile_tab[OTAU_PROFILE_NUM_CHARS] = {
    [PROFILE_OTAU_CHAR1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23, 0x9B, 0xE1, 0x11, 0x02, 0xD1, 0x13, 0x10, 0x00, 0x00},
            },
        },
        .char_val = &otau_char1_profile,
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ,
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

// Define controls char profiles
static struct gatts_char_profile gl_controls_svc_char_profile_tab[CONTROLS_PROFILE_NUM_CHARS] = {
    [PROFILE_CONTROLS_CHAR1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x78, 0xFE, 0xDE, 0x05, 0x1D, 0x3E, 0x48, 0xA1, 0xE6, 0x11, 0xD4, 0x0C, 0x3F, 0x65, 0xC0, 0xAF},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONTROLS_CHAR2_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x78, 0xFE, 0xDE, 0x05, 0x1D, 0x3E, 0x48, 0xA1, 0xE6, 0x11, 0xD4, 0x0C, 0x40, 0x65, 0xC0, 0xAF},
            },
        },
        .char_val = &controls_char2_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_CONTROLS_TRIGGER_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x78, 0xFE, 0xDE, 0x05, 0x1D, 0x3E, 0x48, 0xA1, 0xE6, 0x11, 0xD4, 0x0C, 0xF4, 0x63, 0xC0, 0xAF},
            },
        },
        .char_val = &controls_trigger_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
    },
    [PROFILE_CONTROLS_THROTTLE_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x78, 0xFE, 0xDE, 0x05, 0x1D, 0x3E, 0x48, 0xA1, 0xE6, 0x11, 0xD4, 0x0C, 0x3E, 0x65, 0xC0, 0xAF},
            },
        },
        .char_val = &controls_throttle_char_profile,
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
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
    [PROFILE_DEVICE_INFO_CHAR_SERIAL_NUM_STR_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0x2A25,
            },
        },
        .char_val = &devinfo_serial_num_char_profile,
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

// Define connectivity char profiles
static struct gatts_char_profile gl_conn_svc_char_profile_tab[CONNECTIVITY_PROFILE_NUM_CHARS] = {
    [PROFILE_CONNECTIVITY_CHAR1_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x4C, 0x7A, 0xC4, 0xF4},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONNECTIVITY_CHAR2_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x8A, 0x7D, 0xC4, 0xF4},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONNECTIVITY_CHAR3_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x66, 0x7E, 0xC4, 0xF4},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONNECTIVITY_CHAR4_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0xF3, 0x29, 0xC4, 0xF4},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONNECTIVITY_CHAR5_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x32, 0x80, 0xC4, 0xF4},
            },
        },
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
    },
    [PROFILE_CONNECTIVITY_CHAR6_ID] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = {0x66, 0x7C, 0x50, 0x17, 0x55, 0x5E, 0x22, 0x8D, 0xE6, 0x11, 0x56, 0x00, 0x3F, 0x29, 0xC4, 0xF4},
            },
        },
        .char_val = &connectivity_char2_profile,
        .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
    },

};

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
            // print_char_profile(&gl_dinfo_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_OTAU_APP_ID].service_handle,
                                                            &gl_otau_svc_char_profile_tab[idx].char_uuid,
                                                            gl_otau_svc_char_profile_tab[idx].perm,
                                                            gl_otau_svc_char_profile_tab[idx].property,
                                                            gl_otau_svc_char_profile_tab[idx].char_val,
                                                            (gl_otau_svc_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
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

static void gatts_profile_controls_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_CONTROLS_APP_ID].service_id, GATTS_CONTROLS_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_CONTROLS_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_CONTROLS_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < CONTROLS_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_dinfo_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_CONTROLS_APP_ID].service_handle,
                                                            &gl_controls_svc_char_profile_tab[idx].char_uuid,
                                                            gl_controls_svc_char_profile_tab[idx].perm,
                                                            gl_controls_svc_char_profile_tab[idx].property,
                                                            gl_controls_svc_char_profile_tab[idx].char_val,
                                                            (gl_controls_svc_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
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

static void gatts_profile_connectivity_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
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
        esp_err_t raw_svc_ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_CONNECTIVITY_APP_ID].service_id, GATTS_CONNECTIVITY_NUM_HANDLE);
        if (raw_svc_ret)
        {
            ESP_LOGE(GATTS_TAG, "create service failed, error code = %x", raw_svc_ret);
        }
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);

        gl_profile_tab[PROFILE_CONNECTIVITY_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_CONNECTIVITY_APP_ID].service_handle);

        int idx;
        for (idx = 0; idx < CONNECTIVITY_PROFILE_NUM_CHARS; idx++)
        {
            // print_char_profile(&gl_conn_svc_char_profile_tab[idx]);
            esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_CONNECTIVITY_APP_ID].service_handle,
                                                            &gl_conn_svc_char_profile_tab[idx].char_uuid,
                                                            gl_conn_svc_char_profile_tab[idx].perm,
                                                            gl_conn_svc_char_profile_tab[idx].property,
                                                            gl_conn_svc_char_profile_tab[idx].char_val,
                                                            (gl_conn_svc_char_profile_tab[idx].char_val != NULL) ? &(esp_attr_control_t){
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
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising start failed");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed");
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
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

    ret = esp_ble_gatts_app_register(PROFILE_OTAU_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(PROFILE_CONTROLS_APP_ID);
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

    ret = esp_ble_gatts_app_register(PROFILE_CONNECTIVITY_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(512);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    return;
}

// void print_char_profile(const struct gatts_char_profile *char_profile)
// {
//     if (char_profile->char_val != NULL)
//     {
//         printf("char_val: %p, len: %d, content: ", (void *)char_profile->char_val, char_profile->char_val->attr_len);
//         for (int i = 0; i < char_profile->char_val->attr_len; i++)
//         {
//             printf("%02X ", char_profile->char_val->attr_value[i]);
//         }
//         printf("\n");
//     }
//     else
//     {
//         printf("char_val is NULL\n");
//     }
// }