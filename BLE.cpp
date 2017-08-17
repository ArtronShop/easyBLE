// Coding By IOXhop : www.ioxhop.com
// This version 0.1

#ifndef BLE_CPP
#define BLE_CPP

#include "BLE.h"

BLE::BLE(String name) {
	dev_name = name;
}

bool BLE::begin() {
	esp_err_t ret;

	// Initialize NVS.
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s initialize controller failed\n", __func__);
		return false;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s enable controller failed\n", __func__);
		return false;
	}
	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s init bluetooth failed\n", __func__);
		return false;
	}
	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed\n", __func__);
		return false;
	}

	esp_ble_gatts_register_callback(gatts_event_handler);
	esp_ble_gap_register_callback(gap_event_handler);
}

void BLE::on(event_t event, eCallback_fn callback) {
	if (event == READ) onRead = callback;
	if (event == WRITE) onWrite = callback;
}

char* BLE::data() {
	return writeData;
}

void BLE::addService(BLEService *service) {
	int profileIndex = nowCount_profile++;
			
	// Serial.printf("[addService]: %d\n", service->_char_count);
	gl_profile_tab[profileIndex].service = service;
	gl_profile_tab[profileIndex].gatts_if = ESP_GATT_IF_NONE;
			
	esp_ble_gatts_app_register(profileIndex);
}
		
void BLE::reply(char *data, int len) {
	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = paramNow->read.handle;
	rsp.attr_value.len = len;
	for (int i=0;i<len;i++) rsp.attr_value.value[i] = data[i];
//	strncpy(rsp.attr_value.value, data, len);
	esp_ble_gatts_send_response(gl_profile_tab[nowIndex].gatts_if, paramNow->read.conn_id, paramNow->read.trans_id,
			ESP_GATT_OK, &rsp);
}

BLEService::BLEService(int service_uuid) {
	_service_uuid = service_uuid;
}

void BLEService::addCharacteristic(int char_uuid) {
	_arr_char_uuid[_char_count++] = char_uuid;
}
#endif