#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>


#define MY_DEVICE_UUID "A3EADBE2-F4C9-DDCB-4EBD-C6E07CC2B23C"

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
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