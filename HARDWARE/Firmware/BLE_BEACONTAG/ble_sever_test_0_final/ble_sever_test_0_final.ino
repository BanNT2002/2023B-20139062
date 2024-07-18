
//////////////////////////////////////////////////////////////////
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <unordered_map>
#include <Arduino_JSON.h>
#define BLE_server "ESP32_Server0"
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID "cba1d466-344c-4be3-ab3f-189f80dd7518"
BLEScan* pBLEScan;
BLEServer *pServer;
///////////////////////////////////////////////////////////////////////////////////////
int array_size =0;
int butt=1;
static String jsonString;
static String jsonString_sub="";
static boolean con_ = false;
std::string _value;
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP1  4        /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_SLEEP2  180        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////TIME///////////////////////////////////////////
typedef struct {
  boolean t10     =   0; 
  boolean t20     =   0; 
} flagType;
flagType _Flag;

typedef struct {
   unsigned long t10  =   0;
   unsigned long t20  =   0;
} startType;
startType _StartTimer;

void TurnONFlags() {

  
  if ((millis() - _StartTimer.t10) >= 11000) {
     _Flag.t10  = true;
     _StartTimer.t10 = millis();
  }
  if ((millis() - _StartTimer.t20) >= 20000) {
     _Flag.t20  = true;
     _StartTimer.t20 = millis();
  }
  
  
}

void TurnOFFFlags() {
    _Flag.t10      =   false;
    _Flag.t20      =   false;     
}


///////////////////////////////////////////////////////////////////////////////////
std::vector<std::pair<uint16_t, int8_t>> array_ble; 
std::unordered_map<int, std::pair<int, int>> average_rssi;
int scanTime = 1;
int scan_save_power =0;
int8_t rssi=0;//In seconds
uint16_t fillter_beacon= 1234;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    std::string manufacturerData = advertisedDevice.getManufacturerData();
      uint16_t major = (uint16_t(manufacturerData[20]) << 8) | uint16_t(manufacturerData[21]);
      uint16_t minor = (uint16_t(manufacturerData[22]) << 8) | uint16_t(manufacturerData[23]);
    if(major==fillter_beacon){
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      Serial.print("MAC Address: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());

      if (advertisedDevice.haveName()) {
        Serial.print("Name: ");
        Serial.println(advertisedDevice.getName().c_str());
      }

      if (advertisedDevice.haveRSSI()) {
        Serial.print("RSSI: ");
        Serial.println(advertisedDevice.getRSSI());
        rssi = advertisedDevice.getRSSI();
      }
      Serial.print("Major: ");
      Serial.println(major);
      Serial.print("Minor: ");
      Serial.println(minor);
      Serial.println("------------------------------------");
      array_ble.emplace_back(minor, rssi);
    }
  }
  
};
/////////////////////////////////////////////////////////////////////////////////////
BLECharacteristic *characteristic;
bool device_connected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    device_connected = true;
    Serial.println("connected");
  }
  
  void onDisconnect(BLEServer* pServer) {
    device_connected = false;
    Serial.println("disconnected");
  }
};


class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    _value = pCharacteristic->getValue();

    if (_value == "W") {
    characteristic->setValue(jsonString.c_str());
    characteristic->notify();
      Serial.println("Received data from client: " + String(_value.c_str()));
    }
    else{
      Serial.println("Received data from client: " + String(_value.c_str()));
    }
  }
};
/////////////////////////////////////////////////////////////////////////////
void setupserver(){
  BLEDevice::init(BLE_server);
  pServer = BLEDevice::createServer();
  BLEService *service = pServer->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );
  characteristic->addDescriptor(new BLE2902());
  characteristic->setCallbacks(new MyCallbacks());
  service->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pServer->getAdvertising()->start();
  pServer->setCallbacks(new MyServerCallbacks());
}


void setupscaner(){
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false);
  pBLEScan->setInterval(400);
  pBLEScan->setWindow(320);
}
////////////////////////////////////////////////////////////////////////////
void creation_object(){
  Serial.println("creation");
  Serial.println("========");
  jsonString = "";
  JSONVar myObject = "{}";
 std::unordered_map<int, std::pair<int, int>> average_rssi; // (minor, (tổng rssi, số lần xuất hiện))
for (const auto& pair : array_ble) {
    int minor = pair.first;
    int rssi = pair.second;
    average_rssi[minor].first += rssi;
    average_rssi[minor].second++;
}

// Gán lại vector với các giá trị trung bình
array_ble.clear();
for (const auto& pair : average_rssi) {
    int minor = pair.first;
    int totalRssi = pair.second.first;
    int count = pair.second.second;
    int averageValue = totalRssi / count;
    array_ble.emplace_back(minor, averageValue);
    Serial.print("Minor: ");
    Serial.print(minor);
    Serial.print(", Average RSSI: ");
    Serial.println(averageValue);
}
    array_size=array_ble.size();
    if(array_size>=3){
    for (int i = 0; i < array_ble.size(); i++) {
  uint16_t minor = array_ble[i].first;
  int8_t rssi = array_ble[i].second;
  String minorStr = String(minor);
  const char* minorChar = minorStr.c_str();
  myObject["ble"]=0;
  myObject[minorStr]= rssi;
}
   jsonString = JSON.stringify(myObject);
  Serial.println(jsonString);
}
  std::vector<std::pair<uint16_t, int8_t>>().swap(array_ble);
}
////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
   setupserver();
   setupscaner();
   ///////////////////////////////////////////////////////////////////////////
   esp_err_t errRc1 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P7);
   esp_err_t errRc3 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P7);
   esp_err_t errRc2 = ::esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL8, ESP_PWR_LVL_P7);
   //////////////////////////////////////////////////////////////////////////
  Serial.println("Waiting for ESP32 client connection...");
  ///////////////////////////////////////////////////////////
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();
  ///////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
void loop() {
  TurnONFlags();
//  int value_33=1;
int   value_33 = analogRead(33);
  if(value_33==0){
   if (_Flag.t10){
  for(int i =0; i<8 ;i++){
  con_ = false;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();
  }
  creation_object();
  Serial.println("off can ble");
   }
   
  if (device_connected) {
    if(con_ == false){
    if(jsonString !="" && jsonString_sub != jsonString){
      characteristic->setValue(jsonString.c_str());
      characteristic->notify();
      Serial.println("Sending notification: " + jsonString);
      jsonString_sub=jsonString;
      array_ble.clear();
      con_ = true;
    } 
    }
  }
  else{
//  ////////////////////////////////////////////////////////////////////
    array_ble.clear();
  }
  }
  else{
  ///////////////////////////////////////////////////////////////
  if (scan_save_power != 1 ){
  for(int i =0; i<10 ;i++){
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();
  }
  creation_object();
  scan_save_power = 1;
  Serial.println("off can ble");
  }
    /////////////////////////////////////////////////////////
     if (device_connected) {
    if(jsonString !="" && jsonString_sub != jsonString){
      characteristic->setValue(jsonString.c_str());
      characteristic->notify();
      Serial.println("Sending notification: " + jsonString);
      jsonString_sub=jsonString;
      array_ble.clear();

    }
  }
  ///////////////////////////////////////////////////////////
  if (_Flag.t20){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP2 * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
  }
  //////////////////////////////////////////////////////////////
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
  }
TurnOFFFlags();
  }
///////////////////////////////////////////////////////////////////////////////
