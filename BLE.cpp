// Coding By IOXhop : www.ioxhop.com
// This version 0.2

#ifndef BLE_CPP
#define BLE_CPP

#include "BLE.h"

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
	switch (event) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
		case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
			esp_ble_gap_start_advertising(&test_adv_params);
			break;
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
		default:
			break;
	}
}

static void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param) {
	esp_gatt_status_t status = ESP_GATT_OK;
	if (param->write.need_rsp) {
		if (param->write.is_prep) {
			if (prepare_write_env->prepare_buf == NULL) {
				prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
				prepare_write_env->prepare_len = 0;
				if (prepare_write_env->prepare_buf == NULL) {
					LOG_ERROR("Gatt_server prep no mem\n");
					status = ESP_GATT_NO_RESOURCES;
				}
			} else {
				if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
					status = ESP_GATT_INVALID_OFFSET;
				} else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
					status = ESP_GATT_INVALID_ATTR_LEN;
				}
			}

			esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
			gatt_rsp->attr_value.len = param->write.len;
			gatt_rsp->attr_value.handle = param->write.handle;
			gatt_rsp->attr_value.offset = param->write.offset;
			gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
			memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
			esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
			if (response_err != ESP_OK) {
				LOG_ERROR("Send response error\n");
			}
			free(gatt_rsp);
			if (status != ESP_GATT_OK) {
				return;
			}
			memcpy(prepare_write_env->prepare_buf + param->write.offset,
				param->write.value,
				param->write.len);
			prepare_write_env->prepare_len += param->write.len;

		} else {
			esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
		}
	}
}

static void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param) {
	if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC) {
		esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
	} else {
		ESP_LOGI(GATTS_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
	}
	if (prepare_write_env->prepare_buf) {
		free(prepare_write_env->prepare_buf);
		prepare_write_env->prepare_buf = NULL;
	}
	prepare_write_env->prepare_len = 0;
}


static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	paramNow = param;
	switch (event) {
		case ESP_GATTS_REG_EVT:
			// ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);

			esp_gatt_srvc_id_t service_id_tmp;
			service_id_tmp.is_primary = true;
			service_id_tmp.id.inst_id = 0x00;
			service_id_tmp.id.uuid.len = ESP_UUID_LEN_16;
			service_id_tmp.id.uuid.uuid.uuid16 = gl_profile_tab[nowIndex].service->_service_uuid;
			
			esp_ble_gap_set_device_name(dev_name.c_str());
			esp_ble_gap_config_adv_data(&adv_data);
			esp_ble_gatts_create_service(gatts_if, &service_id_tmp, GATTS_NUM_HANDLE);
			break;
		case ESP_GATTS_READ_EVT: {
			// Serial.printf("GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
		
			if (onRead) {
				for (int i=0;i<gl_profile_tab[nowIndex].service->_char_count;i++) {
					if (param->read.handle == gl_profile_tab[nowIndex].service->_handle_id[i]) {
						onRead(gl_profile_tab[nowIndex].service->_service_uuid, gl_profile_tab[nowIndex].service->_arr_char_uuid[i]);
						break;
					}
				}
				
			}
			break;
		}
		case ESP_GATTS_WRITE_EVT: {
			// Serial.printf("GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d\n", param->write.conn_id, param->write.trans_id, param->write.handle);
			// Serial.printf("GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
			write_event_env(gatts_if, &a_prepare_write_env, param);
		
			if (onWrite) {
				memcpy(writeData, param->write.value, param->write.len);
				for (int i=0;i<gl_profile_tab[nowIndex].service->_char_count;i++) {
					if (param->write.handle == gl_profile_tab[nowIndex].service->_handle_id[i]) {
						onWrite(gl_profile_tab[nowIndex].service->_service_uuid, gl_profile_tab[nowIndex].service->_arr_char_uuid[i]);
						break;
					}
				}
			}
			break;
		}
		case ESP_GATTS_EXEC_WRITE_EVT:
			// ESP_LOGI(GATTS_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
			esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
			exec_write_event_env(&a_prepare_write_env, param);
			break;
		case ESP_GATTS_MTU_EVT:
		case ESP_GATTS_CONF_EVT:
		case ESP_GATTS_UNREG_EVT:
			break;
		case ESP_GATTS_CREATE_EVT:
			// ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
			gl_profile_tab[nowIndex].service_handle = param->create.service_handle;
			
			esp_ble_gatts_start_service(gl_profile_tab[nowIndex].service_handle);
			
			// Serial.printf("[ESP_GATTS_CREATE_EVT]: gl_profile_tab[%d].service->_char_count = %d\n", nowIndex, gl_profile_tab[nowIndex].service->_char_count);
			for (int i=0;i<gl_profile_tab[nowIndex].service->_char_count;i++) {
				// Serial.printf("[ESP_GATTS_CREATE_EVT]: 0x%x\n", gl_profile_tab[nowIndex].service->_arr_char_uuid[i]);
				esp_bt_uuid_t *char_uuid = new esp_bt_uuid_t;
				char_uuid->len = ESP_UUID_LEN_16;
				char_uuid->uuid.uuid16 = gl_profile_tab[nowIndex].service->_arr_char_uuid[i];
				
				esp_ble_gatts_add_char(gl_profile_tab[nowIndex].service_handle, char_uuid,
                             ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                             ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                             &gatts_demo_char1_val, NULL);
			}
			break;
		case ESP_GATTS_ADD_INCL_SRVC_EVT:
			break;
		case ESP_GATTS_ADD_CHAR_EVT: {
			uint16_t length = 0;
			const uint8_t *prf_char;

			/* Serial.printf("ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle); */
			
			for (int i=0;i<gl_profile_tab[nowIndex].service->_char_count;i++) {
				if (param->add_char.char_uuid.uuid.uuid16 == gl_profile_tab[nowIndex].service->_arr_char_uuid[i]) {
					gl_profile_tab[nowIndex].service->_handle_id[i] = param->add_char.attr_handle;
					break;
				}
			}
			
			gl_profile_tab[nowIndex].char_handle = param->add_char.attr_handle;
			gl_profile_tab[nowIndex].descr_uuid.len = ESP_UUID_LEN_16;
			gl_profile_tab[nowIndex].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
			esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);

			esp_ble_gatts_add_char_descr(gl_profile_tab[nowIndex].service_handle, &gl_profile_tab[nowIndex].descr_uuid,
				ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
			break;
		}
		case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		case ESP_GATTS_DELETE_EVT:
		case ESP_GATTS_START_EVT:
		case ESP_GATTS_STOP_EVT:
			break;
		case ESP_GATTS_CONNECT_EVT: {
			esp_ble_conn_update_params_t conn_params = {0};
			memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));

			conn_params.latency = 0;
			conn_params.max_int = 0x50;    // max_int = 0x50*1.25ms = 100ms
			conn_params.min_int = 0x30;    // min_int = 0x30*1.25ms = 60ms
			conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms

			gl_profile_tab[nowIndex].conn_id = param->connect.conn_id;

			esp_ble_gap_update_conn_params(&conn_params);
			break;
		}
		case ESP_GATTS_DISCONNECT_EVT:
			esp_ble_gap_start_advertising(&test_adv_params);
			break;
		case ESP_GATTS_OPEN_EVT:
		case ESP_GATTS_CANCEL_OPEN_EVT:
		case ESP_GATTS_CLOSE_EVT:
		case ESP_GATTS_LISTEN_EVT:
		case ESP_GATTS_CONGEST_EVT:
		default:
			break;
  }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status == ESP_GATT_OK) {
			gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
		} else {
			ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
				param->reg.app_id,
				param->reg.status);
			return;
		}
	}

  do {
    for (nowIndex=0;nowIndex<nowCount_profile;nowIndex++) {
		if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[nowIndex].gatts_if) {
			gatts_profile_event_handler(event, gatts_if, param);
		}
    }
  } while (0);
}

BLE::BLE(String name) {
	dev_name = name;
}

BLE::BLE(String name, byte *service_uuid) {
	dev_name = name;
	memcpy(adv_data.p_service_uuid, service_uuid, 32);
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
	memcpy(rsp.attr_value.value, data, len);
	esp_ble_gatts_send_response(gl_profile_tab[nowIndex].gatts_if, paramNow->read.conn_id, paramNow->read.trans_id,
			ESP_GATT_OK, &rsp);
}

void BLE::reply(String data) {
	reply((char*)data.c_str(), data.length());
}

void BLE::reply(int data) {
	char buf[] = { data&0xFF, (data>>8)&0xFF };
	reply(buf, 2);
}

void BLE::reply(char data) {
	char buf[] = { data };
	reply(buf, 1);
}

BLEService::BLEService(int service_uuid) {
	_service_uuid = service_uuid;
}

void BLEService::addCharacteristic(int char_uuid) {
	_arr_char_uuid[_char_count++] = char_uuid;
}
#endif