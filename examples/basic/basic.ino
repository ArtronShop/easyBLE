// Coding By IOXhop : www.ioxhop.com
// This version 0.2

#include "BLE.h"

BLE ble("BLE Basic");

void setup() {
  Serial.begin(115200);

  ble.begin();
  
  ble.on(READ, [](int service_uuid, int char_uuid) {
    Serial.println("Event: READ");
    Serial.print("Service UUID: 0x");
    Serial.println(service_uuid, HEX);
    Serial.print("Characteristic UUID: 0x");
    Serial.println(char_uuid, HEX);
	
    ble.reply("OK !");
  });
  
  ble.on(WRITE, [](int service_uuid, int char_uuid) {
    Serial.println("Event: WRITE");
    Serial.print("Service UUID: 0x");
    Serial.println(service_uuid, HEX);
    Serial.print("Characteristic UUID: 0x");
    Serial.println(char_uuid, HEX);
    Serial.print("Data: ");
    char *data = ble.data();
    for (int i=0;data[i]!=0;i++) {
        Serial.print("0x");
        if (data[i] < 0x10) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
  });
  
  BLEService *service = new BLEService(0xFF);
  service->addCharacteristic(0xFF01);
  service->addCharacteristic(0xFF02);
  ble.addService(service);
}

void loop() {
  
}