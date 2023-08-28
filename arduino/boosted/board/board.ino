/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEUUID.h>
#include <BLESecurity.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define BOARD_INFO_SERVICE_UUID "7DC55A86-C61F-11E5-9912-BA0BE0483C18"
#define BOARD_VALUE_CHARACTERISTIC_UUID "7DC59643-C61F-11E5-9912-BA0BE0483C18"
#define BOARD_ID_CHARACTERISTIC_UUID "7DC5BB39-C61F-11E5-9912-BA0BE0483C18"
#define BOARD_ODOMETRY_CHARACTERISTIC_UUID "7DC56594-C61F-11E5-9912-BA0BE0483C18"
#define BOARD_RIDE_MODES_CHARACTERISTIC_UUID "7DC55DEC-C61F-11E5-9912-BA0BE0483C18"
#define BOARD_CURRENT_RIDE_MODE_CHARACTERISTIC_UUID "7DC55F22-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BOARD_INFO_1_CHARACTERISTIC_UUID "7DC56666-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BOARD_INFO_2_CHARACTERISTIC_UUID "7DC56986-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BOARD_INFO_3_CHARACTERISTIC_UUID "7DC56B34-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BOARD_INFO_4_CHARACTERISTIC_UUID "7DC56BFC-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BOARD_INFO_5_CHARACTERISTIC_UUID "7DC573EC-C61F-11E5-9912-BA0BE0483C18"

#define BATTERY_INFO_SERVICE_UUID "65A8EAA8-C61F-11E5-9912-BA0BE0483C18"
#define BATTERY_SOC_CHARACTERISTIC_UUID "65A8EEAE-C61F-11E5-9912-BA0BE0483C18"
#define BATTERY_CAPACITY_CHARACTERISTIC_UUID "65A8F3C2-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BATTERY_INFO_1_CHARACTERISTIC_UUID "65A8F5D4-C61F-11E5-9912-BA0BE0483C18"
#define UNKNOWN_BATTERY_INFO_2_CHARACTERISTIC_UUID "65A8F832-C61F-11E5-9912-BA0BE0483C18"

#define UNKNOWN_A_SERVICE_UUID "588560E2-0065-11E6-8D22-5E5517507C66"
#define UNKNOWN_A_1_CHARACTERISTIC_UUID "58856524-0065-11E6-8D22-5E5517507C66"

#define UNKNOWN_B_SERVICE_UUID "00001016-D102-11E1-9B23-00025B00A5A5"
#define UNKNOWN_B_1_CHARACTERISTIC_UUID "00001013-D102-11E1-9B23-00025B00A5A5"
#define UNKNOWN_B_2_CHARACTERISTIC_UUID "00001018-D102-11E1-9B23-00025B00A5A5"
#define UNKNOWN_B_3_CHARACTERISTIC_UUID "00001014-D102-11E1-9B23-00025B00A5A5"
#define UNKNOWN_B_4_CHARACTERISTIC_UUID "00001011-D102-11E1-9B23-00025B00A5A5"

BLEUUID DEVICE_INFORMATION_SERVICE_UUID = BLEUUID((uint16_t)0x180A);
BLEUUID MODEL_NUMBER_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A24);
BLEUUID MANUFACTURER_NAME_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A29);
BLEUUID SERIAL_NUMBER_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A25);
BLEUUID HARDWARE_REVISION_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A27);
BLEUUID FIRMWARE_REVISION_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A26);
BLEUUID PNP_ID_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A50);

uint8_t pnpIDArr[] = { 0x01, 0x00, 0x0A, 0x00, 0x4C, 0x01, 0x00, 0x01 };

uint8_t boardRideModesArr[] = { 0x04 };
uint8_t boardCurrentRideModeArr[] = { 0x03 };
uint8_t boardValueArr[] = { 0x02 };
uint8_t boardOdometryArr[] = { 0xF6, 0xCF, 0x5C, 0x00 };
uint8_t boardInfoUnknown1Arr[] = { 0x4E, 0x6F, 0x6E, 0x65 };
uint8_t boardInfoUnknown2Arr[] = { 0x4E, 0x6F, 0x6E, 0x65 };
uint8_t boardInfoUnknown3Arr[] = { 0x00, 0x00 };
uint8_t boardInfoUnknown4Arr[] = { 0x00, 0x00, 0x00, 0x00 };

uint8_t boardBatteryUnknown1Arr[] = { 0x00 };
uint8_t boardBatteryUnknown2Arr[] = { 0x00 };

uint8_t unknownB1Arr[] = { 0x01 };
uint8_t unknownB4Arr[] = { 0x07 };

// // Set advertisement data byte arrays
char defaultModeManufacturerData[] = { 0x00, 0x00, 0x02, 0x05, 0x01, 0xFF, 0xFF, 0xFF };
char pairingModeManufacturerData[] = { 0x02, 0x00, 0x02, 0x05, 0x01, 0xFF, 0xFF, 0xFF };
char additionalManufacturerData[] = { 0x01, 0x2A, 0x0A, 0xA1, 0x69, 0xFF, 0xFF };

bool IS_PAIRING = true;
bool deviceConnected = false;

class RemoteServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected!");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected.");
  }
};

class MySecurityCallbacks : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    // Generate and return a passkey for pairing
    uint32_t passKey = random(100000, 1000000);
    Serial.println("Generated Passkey: " + String(passKey));
    return passKey;
  }

  void onPassKeyNotify(uint32_t passKey) {
    Serial.println("Passkey Notify: " + String(passKey));
  }

  bool onSecurityRequest() {
    return true;  // Accept security request
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    Serial.println("Authentication Complete: " + String(cmpl.success));
    if (cmpl.success) {
      Serial.println("Pairing successful!");
    } else {
      Serial.println("Pairing failed!");
    }
  }

  bool onConfirmPIN(uint32_t pin) {
    Serial.println(pin);
  }
};

class GeneralCharacteristicCallBack : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.println("Data was received!");
  }

  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.println("Data was read!");
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("BoostedBoard69A10A2A");
  // BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());

  // BLESecurity *pSecurity = new BLESecurity();
  // pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  // pSecurity->setCapability(ESP_IO_CAP_OUT);
  // pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new RemoteServerCallbacks());

  // Setup unknown B service
  BLEService *pUnknownBService = pServer->createService(UNKNOWN_B_SERVICE_UUID);
  BLECharacteristic *pUnknownBCharacteristic1 = pUnknownBService->createCharacteristic(UNKNOWN_B_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUnknownBCharacteristic2 = pUnknownBService->createCharacteristic(UNKNOWN_B_2_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUnknownBCharacteristic3 = pUnknownBService->createCharacteristic(UNKNOWN_B_3_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUnknownBCharacteristic4 = pUnknownBService->createCharacteristic(UNKNOWN_B_4_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  pUnknownBCharacteristic1->setValue(unknownB1Arr, 1);
  pUnknownBCharacteristic4->setValue(unknownB4Arr, 1);

  pUnknownBCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBCharacteristic3->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBCharacteristic4->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup device information services
  BLEService *pDeviceInformationService = pServer->createService(DEVICE_INFORMATION_SERVICE_UUID);
  BLECharacteristic *pModelNumberCharacteristic = pDeviceInformationService->createCharacteristic(MODEL_NUMBER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pManufacturerNameCharacteristic = pDeviceInformationService->createCharacteristic(MANUFACTURER_NAME_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pHardwareRevisionCharacteristic = pDeviceInformationService->createCharacteristic(HARDWARE_REVISION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pFirmwareRevisionCharacteristic = pDeviceInformationService->createCharacteristic(FIRMWARE_REVISION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pPnpIDCharacteristic = pDeviceInformationService->createCharacteristic(PNP_ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  // Set values for the device information characteristics
  pModelNumberCharacteristic->setValue("00000000");
  pManufacturerNameCharacteristic->setValue("Boosted, Inc.");
  pHardwareRevisionCharacteristic->setValue("0000");
  pFirmwareRevisionCharacteristic->setValue("v1.5.2");
  pPnpIDCharacteristic->setValue(pnpIDArr, 8);

  pModelNumberCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pManufacturerNameCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pHardwareRevisionCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pFirmwareRevisionCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pPnpIDCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup board information service
  BLEService *pBoardInfoService = pServer->createService(BLEUUID(BOARD_INFO_SERVICE_UUID), (uint32_t)21);
  BLECharacteristic *pBoardRideModesCharacteristic = pBoardInfoService->createCharacteristic(BOARD_RIDE_MODES_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pCurrentBoardRideModeCharacteristic = pBoardInfoService->createCharacteristic(BOARD_CURRENT_RIDE_MODE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pBoardValueCharacteristic = pBoardInfoService->createCharacteristic(BOARD_VALUE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pBoardOdometryCharacteristic = pBoardInfoService->createCharacteristic(BOARD_ODOMETRY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUnknownBoardCharacteristic1 = pBoardInfoService->createCharacteristic(UNKNOWN_BOARD_INFO_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pUnknownBoardCharacteristic2 = pBoardInfoService->createCharacteristic(UNKNOWN_BOARD_INFO_2_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pUnknownBoardCharacteristic3 = pBoardInfoService->createCharacteristic(UNKNOWN_BOARD_INFO_3_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUnknownBoardCharacteristic4 = pBoardInfoService->createCharacteristic(UNKNOWN_BOARD_INFO_4_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUnknownBoardCharacteristic5 = pBoardInfoService->createCharacteristic(UNKNOWN_BOARD_INFO_5_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pBoardIDCharacteristic = pBoardInfoService->createCharacteristic(BOARD_ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  // Set values for board information characteristics
  pBoardRideModesCharacteristic->setValue(boardRideModesArr, 1);
  pCurrentBoardRideModeCharacteristic->setValue(boardCurrentRideModeArr, 1);
  pBoardValueCharacteristic->setValue(boardValueArr, 1);
  pBoardOdometryCharacteristic->setValue(boardOdometryArr, 4);
  pUnknownBoardCharacteristic1->setValue(boardInfoUnknown1Arr, 4);
  pUnknownBoardCharacteristic2->setValue(boardInfoUnknown2Arr, 4);
  pUnknownBoardCharacteristic3->setValue(boardInfoUnknown3Arr, 2);
  pUnknownBoardCharacteristic4->setValue(boardInfoUnknown4Arr, 4);
  pBoardIDCharacteristic->setValue("BoostedBoard69A10A2A");

  pUnknownBoardCharacteristic5->setCallbacks(new GeneralCharacteristicCallBack());
  pBoardIDCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pBoardRideModesCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pBoardValueCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pCurrentBoardRideModeCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pBoardOdometryCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBoardCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBoardCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBoardCharacteristic3->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBoardCharacteristic4->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup unknown A service
  BLEService *pUnknownAService = pServer->createService(UNKNOWN_A_SERVICE_UUID);
  BLECharacteristic *pUnknownACharacteristic1 = pUnknownAService->createCharacteristic(UNKNOWN_A_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  pUnknownACharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup battery information services
  BLEService *pBatteryInfoService = pServer->createService(BATTERY_INFO_SERVICE_UUID);
  BLECharacteristic *pBatterySOCCharacteristic = pBatteryInfoService->createCharacteristic(BATTERY_SOC_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pBatteryCapacityCharacteristic = pBatteryInfoService->createCharacteristic(BATTERY_CAPACITY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pUnknownBatteryCharacteristic1 = pBatteryInfoService->createCharacteristic(UNKNOWN_BATTERY_INFO_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUnknownBatteryCharacteristic2 = pBatteryInfoService->createCharacteristic(UNKNOWN_BATTERY_INFO_2_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  // Set values for battery information characteristics
  pBatterySOCCharacteristic->setValue("[");
  pBatteryCapacityCharacteristic->setValue("$");
  pUnknownBatteryCharacteristic1->setValue(boardBatteryUnknown1Arr, 1);
  pUnknownBatteryCharacteristic2->setValue(boardBatteryUnknown2Arr, 1);

  pBatterySOCCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pBatteryCapacityCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBatteryCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUnknownBatteryCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());

  // Start all services
  pDeviceInformationService->start();
  pBoardInfoService->start();
  pBatteryInfoService->start();
  pUnknownAService->start();
  pUnknownBService->start();

  // Advertise services and add settings
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData advertisementData = BLEAdvertisementData();
  advertisementData.setFlags(0x06);
  advertisementData.setPartialServices(BLEUUID(BOARD_INFO_SERVICE_UUID));
  if (IS_PAIRING) {
    advertisementData.setManufacturerData(std::string(pairingModeManufacturerData, 8));
  } else {
    advertisementData.setManufacturerData(std::string(defaultModeManufacturerData, 8));
  }

  BLEAdvertisementData scanResponseData = BLEAdvertisementData();
  scanResponseData.setManufacturerData(std::string(additionalManufacturerData, 7));
  scanResponseData.setName("john's boosted");

  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->setScanResponseData(scanResponseData);

  pAdvertising->addServiceUUID(BOARD_INFO_SERVICE_UUID);
  pAdvertising->addServiceUUID(BATTERY_INFO_SERVICE_UUID);
  pAdvertising->addServiceUUID(UNKNOWN_A_SERVICE_UUID);
  pAdvertising->addServiceUUID(UNKNOWN_B_SERVICE_UUID);
  pAdvertising->addServiceUUID(DEVICE_INFORMATION_SERVICE_UUID);

  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("Device is now advertising!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}