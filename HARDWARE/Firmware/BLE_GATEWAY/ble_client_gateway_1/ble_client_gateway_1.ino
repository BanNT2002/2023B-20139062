#include <HardwareSerial.h>
#include <esp_task_wdt.h>
#include <Arduino_JSON.h>     // Thu vien ho tro chuoi Json
#include <BLEDevice.h>
////////////////////////
#define RX_PIN 16
#define TX_PIN 17
#define BAUD_RATE 115200
HardwareSerial SerialPort(2);
String receivedData = "";
//////////////////////////
#define WDT_TIMEOUT 5
////////////////////////////////////////////////////
///////////////////////////////////////////////////
JSONVar _DuLieuBeacontag;
//////////////////////////////////////////////////////
//---------------------------------
/* Khai bao cac co thoi gian
 *  de phan luong cho chuong trinh.
 *  Khi phan luong chuong trinh se 
 *  khong bi delay cung
*/
typedef struct {
  boolean t500ms   = 0;
  boolean t2s   = 0;
  boolean t3s   = 0;
  boolean t4s     =   0;
  boolean t10s  =   0; 
  boolean t8s     =   0; 
  boolean t3m     =   0;
} flagType;
flagType _Flag;

typedef struct {
   unsigned long t500ms  =   0;
   unsigned long t3s  =   0;
   unsigned long t2s  =   0;
   unsigned long t4s  =   0;
   unsigned long t10s  =   0;
   unsigned long t8s  =   0;
   unsigned long t3m  =   0;
 
} startType;
startType _StartTimer;

void TurnONFlags() {
   if ((millis() - _StartTimer.t500ms) >= 500) {
     _Flag.t500ms  = true;
     _StartTimer.t500ms = millis();
  }
  if ((millis() - _StartTimer.t2s) >= 2000) {
     _Flag.t2s  = true;
     _StartTimer.t2s = millis();
  }

     if ((millis() - _StartTimer.t3s) >= 3000) {
     _Flag.t3s  = true;
     _StartTimer.t3s = millis();
  }
    
  if ((millis() - _StartTimer.t4s) >= 4000) {
     _Flag.t4s  = true;
     _StartTimer.t4s = millis();
  }
    if ((millis() - _StartTimer.t10s) >= 10000) {
     _Flag.t10s  = true;
     _StartTimer.t10s = millis();
  }
  if ((millis() - _StartTimer.t8s) >= 8000) {
     _Flag.t8s  = true;
     _StartTimer.t8s = millis();
  }
  
  if ((millis() - _StartTimer.t3m) >= 180000) {
     _Flag.t3m  = true;
     _StartTimer.t3m = millis();
  }
  
}

void TurnOFFFlags() {
     _Flag.t500ms      =   false;
      _Flag.t3s      =   false;
     _Flag.t2s      =   false;
     _Flag.t4s      =   false;
     _Flag.t10s      =   false;
     _Flag.t8s      =   false;
     _Flag.t3m      =   false;     
}
//////////////////////////////////////////////////////
#define MAX_SERVERS 3 // Số lượng tối đa các BLE server bạn muốn kết nối
int Scantime = 5;
// Định nghĩa UUID cho các service và characteristic của từng server
static BLEUUID serviceUUIDs[MAX_SERVERS] = {
  BLEUUID("91bad492-b950-4226-aa2b-4ede9fa42f59"),
  BLEUUID("91bad492-b950-4226-aa2b-4ede9fa42f60"),
  // Thêm UUID cho các server khác (nếu có)
};

static BLEUUID characteristicUUIDs[MAX_SERVERS] = {
  BLEUUID("cba1d466-344c-4be3-ab3f-189f80dd7518"),
  BLEUUID("ca73b3ba-39f6-4ab3-91ae-186dc9577d98"),
  // Thêm UUID cho các server khác (nếu có)
};
///////////////////////////////////////////////////////
std::string _value[MAX_SERVERS];
static BLEAddress *serverAddresses[MAX_SERVERS];
BLEScan* pBLEScan  = nullptr;
BLEClient*  pClient  = BLEDevice::createClient();
String data_="";
String data_sub="";
////////////////////////////////////////////////////////
//BLEAdvertisedDevice advertisedDevice;
static boolean doConnect[MAX_SERVERS];
static boolean connected[MAX_SERVERS];
static boolean connected_=false;
static boolean finish_update = false;
static boolean caculate = false;
static BLERemoteCharacteristic* characteristics[MAX_SERVERS];


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult( BLEAdvertisedDevice advertisedDevice) {
    Serial.println(advertisedDevice.toString().c_str());
    for (int i = 0; i < MAX_SERVERS; i++) {
if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUIDs[i]))
      {
        Serial.print("RSSI: ");
        BLEDevice::getScan()->stop();
        BLEAddress tempAddress = advertisedDevice.getAddress();
        serverAddresses[i] = new BLEAddress(tempAddress);
        doConnect[i] = true;
        Serial.println("Device (" + String(i) + ") found. Connecting!");
        finish_update=true;
      }
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("connect");
   }

  void onDisconnect(BLEClient* pclient) {
    connected_ = false;
    Serial.println("onDisconnect");
    
    
  }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // Xử lý thông báo
    data_=(char*)pData;
    pClient->disconnect();
    caculate =true;
}

 bool connectToServer(int serverIndex) {
  pClient = BLEDevice::createClient();
  if (!pClient->connect(*serverAddresses[serverIndex])) {
//    Serial.println("Failed to connect to server " + String(serverIndex));
    return false;
  }
  pClient->setClientCallbacks(new MyClientCallback());
  BLERemoteService* pRemoteService = pClient->getService(serviceUUIDs[serverIndex]);

  if (pRemoteService == nullptr) {
//    Serial.print("Failed to find service UUID for server " + String(serverIndex));
    pClient->disconnect();
    return false;
  }

  characteristics[serverIndex] = pRemoteService->getCharacteristic(characteristicUUIDs[serverIndex]);
  if (characteristics[serverIndex] == nullptr) {
//    Serial.print("Failed to find characteristic UUID for server " + String(serverIndex + 1));
    pClient->disconnect();
    return false;
  }

//  Serial.println("Characteristic Found for server " + String(serverIndex));
  if (characteristics[serverIndex]->canRead()) {
    _value[serverIndex] = characteristics[serverIndex]->readValue();
    Serial.print("The characteristic value for server " + String(serverIndex) + " was: ");
    Serial.println(_value[serverIndex].c_str());
  }

  if (characteristics[serverIndex]->canNotify()) {
    characteristics[serverIndex]->registerForNotify(notifyCallback);
  }

  connected[serverIndex] = true;
  connected_=true;
}
////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  SerialPort.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("//----------------------------------------------------//");
  Serial.println("Starting Arduino BLE Client application...");
  /////////////////////////////////////////////////////////
  esp_task_wdt_init(WDT_TIMEOUT, true); // 
  esp_task_wdt_add(NULL);
  ////////////////////////////////////////////////////
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(500);
    pBLEScan->setWindow(300);
    pBLEScan->setActiveScan(true);
    /////////////////////////////////////////////////////
    esp_err_t errRc1 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P7);
    esp_err_t errRc2 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL8, ESP_PWR_LVL_P7);
    esp_err_t errRc3 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P7);
    ////////////////////////////////////////////////////
}

void loop() {
     TurnONFlags(); // Bat co thoi gian
       if (_Flag.t3s){
      esp_task_wdt_reset();
      Serial.println("WDT2");
    }
     if (_Flag.t2s){
    esp_task_wdt_reset();
    Serial.println("WDT4");
    BLEScanResults foundDevices = pBLEScan->start(1, true);
    pBLEScan->clearResults();
    }
////////////////////////////////////////////////////////
      for (int i = 0; i < 3; i++) {
      if (doConnect[i]) {
      esp_task_wdt_reset();
//      Serial.println("WDT_connect");
      connectToServer(i);
      doConnect[i] = false;
      }
     }
 
////////////////////////////////////////////////////////
if(caculate){
  esp_task_wdt_reset();
//  Serial.println("WDT_send");
   if(data_sub != data_){
   Serial.print("Data: ");
     if(finish_update){
       finish_update = false;
      _DuLieuBeacontag=JSON.parse(data_);
      Serial.println(_DuLieuBeacontag);
       Serial.println(_DuLieuBeacontag);
    // Send data uart 
      SerialPort.println(_DuLieuBeacontag);
      finish_update = true;
     }
    data_sub=data_;
   }
   
  }

  if (SerialPort.available()) {
    receivedData = SerialPort.readStringUntil('\n');
//    Serial.println("Received from ESP32_ble_gateway_2: " + receivedData);
  }

   TurnOFFFlags(); // Tat co thoi gian
}
