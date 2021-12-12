/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
static const int pinS = 18;
Servo mServo;
bool isDeviceConnected = false;
uint16_t isUnlock = false;

void lock(){
  Serial.println("lock");
  mServo.write(175);
  isUnlock = false;
}
void unlock(){
  Serial.println("unlock");
  mServo.write(5); 
  isUnlock = true; 
}
class MyServerCallbacks : public BLEServerCallbacks{
  void onConnect(BLEClient *pClient){
    isDeviceConnected = true;
  }
  void onDisconnect(BLEClient *pClient){
    isDeviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      Serial.println(value.c_str());
      if(!value.compare("lock1")){
        lock();
      }
      else if(!value.compare("unlock1")){
        unlock();
      }
    }
    void onRead(BLECharacteristic *pCharacteristic){
      pCharacteristic->setValue(isUnlock);
    }
};

BLEServer *pServer;
void setup() {
  Serial.begin(115200);
  mServo.attach(pinS, 500, 2400);

  BLEDevice::init("LoESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());

  isUnlock = 3;
  pCharacteristic->setValue(isUnlock);
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(isDeviceConnected == false){
    delay(2000);
    pServer->startAdvertising();
  }
  delay(20);
}
