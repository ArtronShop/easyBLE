// Coding By IOXhop : www.ioxhop.com
// This version 0.2

#ifndef BLE_H
#define BLE_H

#include "Arduino.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "bt.h"
#include "bta_api.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"

#define GATTS_TAG "BLELib"

#define GATTS_NUM_HANDLE     10
#define PROFILE_MAX 10
#define MAX_CHAR_PER_SERVICE 5
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40
#define PREPARE_BUF_MAX_SIZE 1024

typedef enum {
	READ,
	WRITE
} event_t;

typedef void(*eCallback_fn)(int, int);

static eCallback_fn onRead, onWrite;

static String dev_name = "ESP32_BLE";

static uint8_t char1_str[] = {0x11, 0x22, 0x33};
static esp_attr_value_t gatts_demo_char1_val = {
  .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
  .attr_len     = sizeof(char1_str),
  .attr_value   = char1_str,
};

static uint8_t service_uuid128[32] = {
	/* LSB <--------------------------------------------------------------------------------> MSB */
	//first uuid, 16bit, [12],[13] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAB, 0xCD, 0x00, 0x00,
	//second uuid, 32bit, [12], [13], [14], [15] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAB, 0xCD, 0xAB, 0xCD,
};

static esp_ble_adv_data_t adv_data = {
	.set_scan_rsp = false,
	.include_name = true,
	.include_txpower = true,
	.min_interval = 0x20,
	.max_interval = 0x40,
	.appearance = 0x00,
	.manufacturer_len = 0,
	.p_manufacturer_data =  NULL,
	.service_data_len = 0,
	.p_service_data = NULL,
	.service_uuid_len = 32,
	.p_service_uuid = service_uuid128,
	.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t test_adv_params = {
  .adv_int_min        = 0x20,
  .adv_int_max        = 0x40,
  .adv_type           = ADV_TYPE_IND,
  .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
  .peer_addr          = {0},
  .peer_addr_type     = BLE_ADDR_TYPE_PUBLIC,
  .channel_map        = ADV_CHNL_ALL,
  .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static int nowCount_profile = 0;
static esp_ble_gatts_cb_param_t *paramNow;
static char writeData[50];
static int nowIndex = 0;

static struct prepare_type_env_t {
	uint8_t *prepare_buf;
	int      prepare_len;
} a_prepare_write_env;

class BLEService {
	public:
		int _service_uuid = 0;
	
		int _arr_char_uuid[MAX_CHAR_PER_SERVICE];
		int _handle_id[MAX_CHAR_PER_SERVICE];
		int _char_count = 0;
		
		BLEService(int) ;

		void addCharacteristic(int);

};

struct gatts_profile_inst {
	BLEService *service;
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	uint16_t char_handle;
	esp_gatt_perm_t perm;
	esp_gatt_char_prop_t property;
	uint16_t descr_handle;
	esp_bt_uuid_t descr_uuid;
};

static struct gatts_profile_inst gl_profile_tab[PROFILE_MAX];

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
static void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

// Class ----------------
class BLE {
	public:
		BLE(String) ;
		BLE(String, byte*);
		
		bool begin() ;
		void on(event_t, eCallback_fn) ;
		char* data() ;
		void addService(BLEService*) ;
		void reply(char*, int) ;
		void reply(String) ;
		void reply(int) ;
		void reply(char) ;
	
};
// END Class ------------

#endif