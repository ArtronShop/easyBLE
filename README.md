# easyBLE

ใช้งานส่วน BLE บน ESP32 ได้อย่างง่าย โดยเรียกใช้ฟังก์ชั่นเพียงไม่กี่ชุดก็สามารถใช้งาน BLE ได้แล้ว ไลบารี่ใช้งานได้กับ Arduino IDE

**หมายเหตุ** นี่ไม่ใช่ไลบารี่จากผู้พัฒนาหลัก

## คลาส BLEService

ใช้สำหรับสร้างเซอร์วิส GATTS และ Characteristic

### BLEService::BLEService(int service_uuid);

เมื่อมีการสร้างออปเจ็คขึ้นมาใหม่ จะต้องกำหนดค่าดังนี้

- (int) service_uuid - กำหนด UUID ของเซอร์วิสที่ต้องการสร้าง มักอยู่ในรูปของเลขฐาน 16 แบบ 8 บิต

### void BLEService::addCharacteristic(int char_uuid) ;

ใช้เพิ่ม Characteristic ลงในเซอร์วิสนั้น ๆ จะต้องกำหนดค่าดังนี้

- (int) char_uuid - กำหนด UUID ของ Characteristic ที่ต้องการสร้าง

## คลาส BLE

ใช้กำหนดเริ่มต้นทำงาน และเพิ่มเซอร์วิสของ GATTS

### BLE::BLE(String name);

เมื่อสร้างออปเจคขึ้นมาใหม่ จะต้องกำหนดค่าดังนี้

- (String) name - ชื่อของบลูทูธที่แสดงให้เห็น

### BLE::BLE(String name, byte *service_uuid);

เมื่อสร้างออปเจคขึ้นมาใหม่ อาจใช้รูปแบบการสร้างโดยกำหนด service_uuid เพิ่มเติมได้

- (byte*) service_uuid

### bool BLE::begin()

ใช้เปิดใช้งานบลูทูธ และเริ่มต้นการทำงานต่าง ๆ เกี่ยวกับบลูทูธ

**ค่าที่ส่งกลับ** เป็นผลการเปิดใช้งานโมดูลเกี่ยวกับบลูทูธ

### void BLE::on(event_t event, eCallback_fn callback);

ใช้กำหนดฟังก์ชั่นที่จะถูกเรียกขึ้นมาเมื่อมีการรับ - ส่งข้อมูลเกิดขึ้น

- (event_t) event - กำหนดเหตุการณ์ที่จะไปเรียกฟังก์ชั่นที่กำหนด โดยเลือกกรอกได้เป็น READ หรือ WRITE
- (eCallback_fn) callback - กำหนดชื่อฟังก์ชั่นที่จะไปเรียกเมื่อเกิดเหตุการณ์ขึ้น

### void BLE::addService(BLEService *service);

ใช้เพิ่มเซอร์วิสที่สร้างด้วยคลาส BLEService

- (BLEService*) service - คลาสของเซอร์วิสที่ต้องการเพิ่ม

### void BLE::reply(char *data, int len);
### void BLE::reply(String data);
### void BLE::reply(int data);
### void BLE::reply(char data);

ใช้ตอบกลับข้อมูลเมื่อมีเหตุการณ์ READ เข้ามา

- (char*) data - ข้อมูลที่ต้องการตอบกลับ
- (String) data  - ข้อมูลที่ต้องการตอบกลับ
- (int) data  - ข้อมูลที่ต้องการตอบกลับ
- (char) data  - ข้อมูลที่ต้องการตอบกลับ
- (int) len - ความยาวของข้อมูล

## ลิขสิทธิ์การใช้งาน

ผู้จัดทำอนุญาตให้นำไปใช้งาน และแจกจ่ายได้โดยคงไว้ซึ่งที่มาของเนื้อหา ห้ามมีให้นำไปใช้งานในเชิงพาณีย์โดยตรง เช่น การนำไปจำหน่าย

 - http://www.ioxhop.com/
