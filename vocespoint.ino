#include <BLEDescriptor.h>
#include<BLECharacteristic.h>
#include<BLE2902.h>
#include<BLEAdvertisedDevice.h>
#include<BLEDevice.h>
#include<BLEUtils.h>
#include<BLEBeacon.h>
#include<BLEUUID.h>
#include<BLEServer.h>
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "beep.h"
#include "AudioConfigLocal.h"
using namespace audio_tools;
AudioInfo info(48000, 1, 16);
void sunet();
void chemare();
void stopchemare();
void trimitereMesaj(String mesajNotify);
//AnalogAudioStream out;
AudioKitStream out;
MemoryStream music(Beep_array, BeepArray_len);
StreamCopy copier(out, music);

BLEAdvertising *pAdvertising;
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool notificare = false;
bool StatusBeep =false;
String mesagiValue = "salut";
int perioada = 10, contor =0;
#define BUUID "A134D0B2-1DA2-1BA7-C94C-E8E00C9F7A2D"
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer(){
  sunet();
  contor++;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      trimitereMesaj("ion dominiuc alexei mateievici");
      Serial.print("este o conectare noua");
      };
        
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
Serial.print("clientul sa deconectat");      
pServer->startAdvertising();
        Serial.println("start advertising");
        if (StatusBeep)
        stopchemare();
        }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      //std::string rxValue = pCharacteristic->getValue();
      String rxValue = pCharacteristic->getValue();
Serial.print(rxValue.c_str());

if (rxValue == "1") {
chemare();
trimitereMesaj("Se redă semnalu sonor");
}
if(rxValue == "0") {
stopchemare();
Serial.println("sa livrat 0");
}
    }
};

class MyDescriptorCallbacks: public BLEDescriptorCallbacks {
  void onWrite(BLEDescriptor* pDescriptor){
uint16_t descriptorValue = pDescriptor ->getValue()[0]|(pDescriptor ->getValue()[1] << 8);
Serial.println(descriptorValue);
if (descriptorValue == 1) {
//pTxCharacteristic ->setNotifyProperty(true);

} else if (descriptorValue == 0) {
//pTxCharacteristic ->setNotifyProperty(false);
//pDescriptor -> setValue(descriptorValue);
}
  };
  void onRead(BLEDescriptor* pDescriptor){
pDescriptor -> getValue();
}
};

void personalserver(){
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor *pBleDescriptor = new BLE2902();
  pBleDescriptor -> setCallbacks(new MyDescriptorCallbacks());
                        pTxCharacteristic->addDescriptor(pBleDescriptor);
pTxCharacteristic->setNotifyProperty(true);
BLECharacteristic * pRx = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
pRx->setCallbacks(new MyCallbacks());
pRx->setWriteProperty(true);
  pService->start();
//pAdvertising = pServer->getAdvertising();  
//pAdvertising->start();
}

void personalBeacon()
{
pAdvertising = pServer->getAdvertising();
    BLEBeacon myBeacon;
  myBeacon.setManufacturerId(0X4C00);
  myBeacon.setMajor(1);
  myBeacon.setMinor(12);
  myBeacon.setSignalPower(0xc5);
  BLEUUID bleUUID = BLEUUID(BUUID);
bleUUID = bleUUID.to128();
  myBeacon.setProximityUUID(BLEUUID(bleUUID.getNative()->uuid.uuid128, 16, true));
  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x1A);
  advertisementData.setManufacturerData(myBeacon.getData());
  pAdvertising->setAdvertisementData(advertisementData);
//  pAdvertising->setAdvertisementType(ADV_TYPE_IND);

    pAdvertising->start();
}
void setup() {
  //My_timer = timerBegin(0, 80, true);
  My_timer = timerBegin(1);
timerAttachInterrupt(My_timer, &onTimer);
timerAlarm(My_timer, 1000000, true, 0);
Serial.begin(115200);
BLEDevice::init("punct de emiter");
personalserver();
personalBeacon();
  Serial.println("asteptam o conectare ...");
      auto config = out.defaultConfig(TX_MODE);
  config.copyFrom(info);  
//config.auto_clear = true;
config.clear();
config.use_apll = true;
out.begin(config);
}
void chemare(){
  sunet();
  StatusBeep = true;
timerStart(My_timer);
  }
void stopchemare(){
  StatusBeep = false;
  timerEnd(My_timer);
  contor = 0;
}
void sunet(){
    music.begin();
do {
  if (music.begin())
        copier.copy();
}while (!copier.copy());
if (!copier.copy());
music.end();
}
void trimitereMesaj(String mesajNotify){
  mesagiValue =mesajNotify;
  pTxCharacteristic->setValue(mesajNotify.c_str());
pTxCharacteristic->notify();
//mesajNotify ++;
}

void loop() {
          
    if (contor >= perioada)
  stopchemare();
}
