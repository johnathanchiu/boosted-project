#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>


#define BOARD_ADVERTISED_UUID "7DC55A86-C61F-11E5-9912-BA0BE0483C18"
#define CONTROLLER_ADVERTISED_SERVICE "F4C4772C-0056-11E6-8D22-5E5517507C66"

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    if (advertisedDevice.isAdvertisingService(BLEUUID(CONTROLLER_ADVERTISED_SERVICE)) || advertisedDevice.isAdvertisingService(BLEUUID(BOARD_ADVERTISED_UUID))) {
      Serial.print("Device Name: ");
      Serial.println(advertisedDevice.getName().c_str());
      Serial.print("  Address: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());

      const uint8_t* rawData = advertisedDevice.getPayload();
      size_t rawDataLength = advertisedDevice.getPayloadLength();

      Serial.print("  Raw Data: ");
      for (int i = 0; i < rawDataLength; i++) {
        Serial.print(rawData[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      if (advertisedDevice.haveAppearance()) {
        Serial.print("  Appearance Value: ");
        Serial.println(advertisedDevice.getAppearance());
      }

      if (advertisedDevice.haveRSSI()) {
        Serial.print("  RSSI Value: ");
        Serial.println(advertisedDevice.getRSSI());
      }

      if (advertisedDevice.haveTXPower()) {
        Serial.print("  TX Power Value: ");
        Serial.println(advertisedDevice.getTXPower());
      }

      if (advertisedDevice.haveServiceUUID()) {
        Serial.print("  Service UUID: ");
        Serial.println(advertisedDevice.getServiceUUID().toString().c_str());
      }

      if (advertisedDevice.haveManufacturerData()) {
        Serial.print("  Manufacturer Data: ");
        Serial.println(advertisedDevice.getManufacturerData().c_str());
      }

      Serial.println();
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);
}

void loop() {
  // Do nothing in the loop, scanning is handled asynchronously
}