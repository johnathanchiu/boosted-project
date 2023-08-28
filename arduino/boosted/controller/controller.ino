/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEUUID.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define OTAU_SERVICE_UUID "00001016-D102-11E1-9B23-00025B00A5A5"
#define UNDEFINED_OTAU_1_CHARACTERISTIC_UUID "00001013-D102-11E1-9B23-00025B00A5A5"
#define UNDEFINED_OTAU_2_CHARACTERISTIC_UUID "00001018-D102-11E1-9B23-00025B00A5A5"
#define UNDEFINED_OTAU_3_CHARACTERISTIC_UUID "00001014-D102-11E1-9B23-00025B00A5A5"
#define UNDEFINED_OTAU_4_CHARACTERISTIC_UUID "00001011-D102-11E1-9B23-00025B00A5A5"

#define CONTROLS_SERVICE_UUID "AFC05DA0-0CD4-11E6-A148-3E1D05DEFE78"
#define THROTTLE_CHARACTERISTIC_UUID "AFC0653E-0CD4-11E6-A148-3E1D05DEFE78"
#define TRIGGER_CHARACTERISTIC_UUID "AFC063F4-0CD4-11E6-A148-3E1D05DEFE78"
#define UNDEFINED_CONTROLLER_1_CHARACTERISTIC_UUID "AFC0653F-0CD4-11E6-A148-3E1D05DEFE78"
#define UNDEFINED_CONTROLLER_2_CHARACTERISTIC_UUID "AFC06540-0CD4-11E6-A148-3E1D05DEFE78"

#define CONNECTIVITY_SERVICE_UUID "F4C4772C-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_1_CHARACTERISTIC_UUID "F4C47A4C-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_2_CHARACTERISTIC_UUID "F4C47D8A-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_3_CHARACTERISTIC_UUID "F4C47E66-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_4_CHARACTERISTIC_UUID "F4C429F3-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_5_CHARACTERISTIC_UUID "F4C48032-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_6_CHARACTERISTIC_UUID "F4C48032-0056-11E6-8D22-5E5517507C66"
#define UNDEFINED_CONNECTIVITY_7_CHARACTERISTIC_UUID "F4C4293F-0056-11E6-8D22-5E5517507C66"

BLEUUID DEVICE_INFORMATION_SERVICE_UUID = BLEUUID((uint16_t)0x180A);
BLEUUID MODEL_NUMBER_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A24);
BLEUUID MANUFACTURER_NAME_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A29);
BLEUUID SERIAL_NUMBER_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A25);
BLEUUID HARDWARE_REVISION_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A27);
BLEUUID FIRMWARE_REVISION_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A26);
BLEUUID PNP_ID_CHARACTERISTIC_UUID = BLEUUID((uint16_t)0x2A50);

uint8_t pnpIDArr[] = { 0x01, 0x00, 0x0A, 0x00, 0x4C, 0x01, 0x00, 0x01 };
uint8_t throttleValueArr[] = { 0x00, 0x00, 0x00, 0x00 };
uint8_t triggerValueArr[] = { 0x00, 0x00 };
uint8_t undefinedControllerValueArr[] = { 0xF5, 0x02, 0x01, 0x00, 0x00, 0x00, 0xC0, 0x2F, 0x00, 0x00, 0xE3, 0x30, 0x00, 0x00 };

uint8_t undefinedOtauCharacteristicArr1[] = { 0x01 };
uint8_t undefinedOtauCharacteristicArr4[] = { 0x07 };

// Set advertisement data byte arrays
char defaultModeManufacturerData[] = { 0x00, 0x00, 0x03, 0x03, 0x02, 0xFF, 0xFF, 0xFF };
char pairingModeManufacturerData[] = { 0x02, 0x00, 0x03, 0x03, 0x02, 0xFF, 0xFF, 0xFF };
char additionalManufacturerData[] = { 0x00, 0xE3, 0xF6, 0xBA, 0x99, 0xFF, 0xFF };

bool IS_PAIRING = false;
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

class GeneralCharacteristicCallBack : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.println("Data was received!");
  }

  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.println("Data was received!");
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("BoostedRmt99BAF6E3");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new RemoteServerCallbacks());

  // Setup throttle and button information service
  BLEService *pThrottleButtonService = pServer->createService(CONTROLS_SERVICE_UUID);
  BLECharacteristic *pThrottleCharacteristic = pThrottleButtonService->createCharacteristic(THROTTLE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pTriggerCharacteristic = pThrottleButtonService->createCharacteristic(TRIGGER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pUndefinedControllerCharacteristic1 = pThrottleButtonService->createCharacteristic(TRIGGER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedControllerCharacteristic2 = pThrottleButtonService->createCharacteristic(TRIGGER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  // Set values for throttle and button information characteristics
  pThrottleCharacteristic->setValue(throttleValueArr, 4);
  pTriggerCharacteristic->setValue(triggerValueArr, 2);
  pUndefinedControllerCharacteristic2->setValue(undefinedControllerValueArr, 14);

  pUndefinedControllerCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedControllerCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());
  pTriggerCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pThrottleCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup device information services
  BLEService *pDeviceInformationService = pServer->createService(DEVICE_INFORMATION_SERVICE_UUID);
  BLECharacteristic *pModelNumberCharacteristic = pDeviceInformationService->createCharacteristic(MODEL_NUMBER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pManufacturerNameCharacteristic = pDeviceInformationService->createCharacteristic(MANUFACTURER_NAME_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pSerialNumberCharacteristic = pDeviceInformationService->createCharacteristic(SERIAL_NUMBER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pHardwareRevisionCharacteristic = pDeviceInformationService->createCharacteristic(HARDWARE_REVISION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pFirmwareRevisionCharacteristic = pDeviceInformationService->createCharacteristic(FIRMWARE_REVISION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *pPnpIDCharacteristic = pDeviceInformationService->createCharacteristic(PNP_ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  // Set values for the device information characteristics
  pModelNumberCharacteristic->setValue("00000000");
  pManufacturerNameCharacteristic->setValue("Boosted, Inc.");
  pSerialNumberCharacteristic->setValue("99BAF6E3");
  pHardwareRevisionCharacteristic->setValue("0189C741");
  pFirmwareRevisionCharacteristic->setValue("v2.3.3");
  pPnpIDCharacteristic->setValue(pnpIDArr, 8);

  pModelNumberCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pManufacturerNameCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pSerialNumberCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pHardwareRevisionCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pFirmwareRevisionCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());
  pPnpIDCharacteristic->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup connectivity information service
  BLEService *pConnectivityService = pServer->createService(CONNECTIVITY_SERVICE_UUID);
  BLECharacteristic *pUndefinedConnectivityCharacteristic1 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic2 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_2_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic3 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_3_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic4 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_4_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic5 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_5_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic6 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_6_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedConnectivityCharacteristic7 = pConnectivityService->createCharacteristic(UNDEFINED_CONNECTIVITY_7_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  // Set values for device information characteristics
  pUndefinedConnectivityCharacteristic7->setValue("BoostedRmt99BAF6E3");
  pUndefinedConnectivityCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic3->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic4->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic5->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic6->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedConnectivityCharacteristic7->setCallbacks(new GeneralCharacteristicCallBack());

  // Setup otau information service
  BLEService *pOtauService = pServer->createService(OTAU_SERVICE_UUID);
  BLECharacteristic *pUndefinedOtauCharacteristic1 = pOtauService->createCharacteristic(UNDEFINED_OTAU_1_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedOtauCharacteristic2 = pOtauService->createCharacteristic(UNDEFINED_OTAU_2_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pUndefinedOtauCharacteristic3 = pOtauService->createCharacteristic(UNDEFINED_OTAU_3_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pUndefinedOtauCharacteristic4 = pOtauService->createCharacteristic(UNDEFINED_OTAU_4_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

  // Set values for otau information characteristics
  pUndefinedOtauCharacteristic1->setValue(undefinedOtauCharacteristicArr1, 1);
  pUndefinedOtauCharacteristic4->setValue(undefinedOtauCharacteristicArr4, 1);
  pUndefinedOtauCharacteristic1->setCallbacks(new GeneralCharacteristicCallBack());
  pUndefinedOtauCharacteristic2->setCallbacks(new GeneralCharacteristicCallBack());

  // Start all services
  pThrottleButtonService->start();
  pDeviceInformationService->start();
  pConnectivityService->start();
  pOtauService->start();

  // Advertise services and add settings
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData advertisementData = BLEAdvertisementData();
  advertisementData.setFlags(0x06);
  advertisementData.setPartialServices(BLEUUID(CONNECTIVITY_SERVICE_UUID));
  if (IS_PAIRING) {
    advertisementData.setManufacturerData(std::string(pairingModeManufacturerData, 8));
  } else {
    advertisementData.setManufacturerData(std::string(defaultModeManufacturerData, 8));
  }

  BLEAdvertisementData scanResponseData = BLEAdvertisementData();
  scanResponseData.setManufacturerData(std::string(additionalManufacturerData, 7));
  scanResponseData.setName("BoostedRmt99BAF6E3");

  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->setScanResponseData(scanResponseData);
  pAdvertising->addServiceUUID(CONTROLS_SERVICE_UUID);
  pAdvertising->addServiceUUID(DEVICE_INFORMATION_SERVICE_UUID);
  pAdvertising->addServiceUUID(CONNECTIVITY_SERVICE_UUID);
  pAdvertising->addServiceUUID(OTAU_SERVICE_UUID);

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