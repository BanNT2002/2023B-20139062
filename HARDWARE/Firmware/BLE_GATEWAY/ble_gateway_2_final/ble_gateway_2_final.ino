#include <HardwareSerial.h>
#include <Arduino_JSON.h>     // Thu vien ho tro chuoi Json
#include <WiFi.h>             // Thu vien ho tro ket noi wifi
#include <FirebaseESP32.h>    // Thu vien ho tro firebase
 #include <math.h>
////////////////////////
////////////////////////////////////////////////////
#define RX_PIN 16
#define TX_PIN 17
#define BAUD_RATE 115200
HardwareSerial SerialPort(2); // Sử dụng UART2
///////////////////////////////////////////////////
String* objectNames_bletag = nullptr;
String* objectNames_ible = nullptr;
String ble_name ="";
int count_ible;
float Px = 0;
float Py = 0;
String receivedData = ""; 
float max_distance = 14.2;
//////////////////////////////////////////////////
static boolean reciever_uart =false;
///////////////////////////////////////////////////
const char* ssid = "ANH";       // Tên mạng Wifi bạn muốn kết nối
const char* password = "03012002";  // Mật khẩu mạng Wifi
JSONVar _DuLieuIBeacon;
JSONVar _DuLieuBeacontag;
void POSTDuLieuBeacontag();// Ham gui du lieu len firebase
JSONVar GETDuLieuIBeacon();// Ham lay du lieu tu firebase
JSONVar DuLieuBeacontagMacDinh();
void caculate_location(JSONVar _Data);
void Gopdulieu();
float findMax(float a1, float a2, float a3, float a4);
float findMin(float a1, float a2, float a3) ;
void main_function();
////////////////////////////////////////////////////////////
// Dinh nghia cac API de lien ket voi Firebase
#define FIREBASE_HOST ("https://position-ble-default-rtdb.firebaseio.com/")
#define FIREBASE_SECRET ("LhCkSkwf43KczQU0w1eTpGZ8Hk1TNU6T0D9ah7sw")
FirebaseData firebaseData;
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
  boolean t4s     =   0;
  boolean t10s  =   0; 
  boolean t20s     =   0; 
  boolean t30s     =   0;
} flagType;
flagType _Flag;

typedef struct {
   unsigned long t500ms  =   0;
   unsigned long t2s  =   0;
   unsigned long t4s  =   0;
   unsigned long t10s  =   0;
   unsigned long t20s  =   0;
   unsigned long t30s  =   0;
 
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
    
  if ((millis() - _StartTimer.t4s) >= 4000) {
     _Flag.t4s  = true;
     _StartTimer.t4s = millis();
  }
    if ((millis() - _StartTimer.t10s) >= 10000) {
     _Flag.t10s  = true;
     _StartTimer.t10s = millis();
  }
  if ((millis() - _StartTimer.t20s) >= 20000) {
     _Flag.t20s  = true;
     _StartTimer.t20s = millis();
  }
  
  if ((millis() - _StartTimer.t30s) >= 30000) {
     _Flag.t30s  = true;
     _StartTimer.t30s = millis();
  }
  
}

void TurnOFFFlags() {
     _Flag.t500ms      =   false;
     _Flag.t2s      =   false;
     _Flag.t4s      =   false;
     _Flag.t10s      =   false;
     _Flag.t20s      =   false;
     _Flag.t30s      =   false;     
}
//////////////////////////////////////////////////////
int A = -55;
int n = 2.7;
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  SerialPort.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  WiFi.mode(WIFI_STA);   // Kich hoat che do station
  WiFi.begin(ssid, password); // Kich hoat wifi 
  Firebase.begin(FIREBASE_HOST, FIREBASE_SECRET);
  Firebase.reconnectWiFi(true);
  Serial.println("Connecting to WiFi...");
  int i=0;
  for(i;i<=5;i++){
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  else{
    break;
  }
  }
  Serial.println(firebaseData.errorReason());
  Serial.println(WiFi.localIP()); // Xuat dia chi IP Den1111112 mang wifi ma esp ket noi
  _DuLieuIBeacon= GETDuLieuIBeacon();
  Serial.println(_DuLieuIBeacon); 
  Serial.println("//----------------------------------------------------//");
  Serial.println("Starting Arduino BLE Client application...");
  /////////////////////////////////////////////////////////
}

void loop() {
     TurnONFlags(); // Bat co thoi gian
     main_function();
     TurnOFFFlags(); // Tat co thoi gian

}

void Gopdulieu(){
    char jsonString[100];
    count_ible=ble_name.toInt();
    snprintf(jsonString, sizeof(jsonString), "{\"beacontag%d\":{\"Px\":%.2f,\"Py\":%.2f}}", count_ible, Px, Py);
    _DuLieuBeacontag = JSON.parse(jsonString);
    Serial.print("_DuLieuBeacontag: ");
    Serial.println(_DuLieuBeacontag);
    if(Px >= 0 && Py >= 0){
    POSTDuLieuBeacontag();
    }
}

///////////////////////////////////////////////////////

//////////////////////////////////////////////////////
void POSTDuLieuBeacontag() {
    FirebaseJson updateData;
    updateData.setJsonData(JSON.stringify(_DuLieuBeacontag));
    Firebase.RTDB.updateNode(&firebaseData, "/position", &updateData);
}
////////////////////////////////////////////////////////
JSONVar GETDuLieuIBeacon(){
    Firebase.RTDB.getJSON(&firebaseData, "/beacons");
    return JSON.parse(firebaseData.jsonString());
}
//////////////////////////////////////////////////
JSONVar DuLieuBeacontagMacDinh() {
  JSONVar DuLieuBeacontag;
  DuLieuBeacontag["Beacon"] = 0;
  return DuLieuBeacontag;
}
//////////////////////////////////////////

///////////////////////////////////////////////////////       
void caculate_location(JSONVar _Data){
  int MAX_OBJECT_COUNT = 5;
  delete[] objectNames_bletag;
  objectNames_bletag = new String[MAX_OBJECT_COUNT];
  //////////////////////////////////////////////////////////
  int MAX_OBJECT_COUNT_ibe = 20;
  delete[] objectNames_ible;
  objectNames_ible = new String[MAX_OBJECT_COUNT_ibe];
  int rssi1 = 0;
  int rssi2 = 0;
  int rssi3 = 0;
  int rssi4 = 0;
  float d1 =  0;
  float d2 =  0;
  float d3 =  0;
  float d4 =  0;
  ////////////////////////////
  float Px1=0;
  float Px2 =0;
  float Px3 =0;
  float Px4 =0;
  //////////////////////////////
  float Py1 =0;
  float Py2 =0;
  float Py3 =0;
  float Py4 =0;
  /////////////////////////////
  JSONVar keys = _Data.keys();
  ///////////////////////////////
  int ibeacon=0;
  for (int i = 0; i < keys.length(); i++) {
    objectNames_bletag[i] =JSON.stringify( _Data.keys()[i]);
    if(i==0){
      ble_name=JSON.stringify( _Data[keys[i]]);
    }
     String num_beacon_ = objectNames_bletag[i];
     String rssi1_ = "1";
     String rssi4_ = "4";
     num_beacon_.remove(0, 1);
     num_beacon_.remove(1, 1);
     if(i==1){
      rssi1 = (JSON.stringify( _Data[keys[i]])).toInt();
      if(num_beacon_ == rssi1_ ){
        rssi1 -=4;
      } 
      else if (num_beacon_ == rssi4_){
        rssi1 -=4;
      }
    }
    else if(i==2){
      rssi2 = (JSON.stringify( _Data[keys[i]])).toInt();
      if(num_beacon_ == rssi1_ ){
        rssi2 -=4;
      }
      else if (num_beacon_ ==  rssi4_){
        rssi2 -=4;
      }
    }
    else if(i==3){
      rssi3 = (JSON.stringify( _Data[keys[i]])).toInt();
      if(num_beacon_ ==rssi1_ ){
        rssi3 -=4;
      }
      else if (num_beacon_ == rssi4_){
        rssi3 -=4;
      }
    }
    else if(i==4){
      rssi4 = (JSON.stringify( _Data[keys[i]])).toInt();
      if(num_beacon_ ==rssi1_ ){
        rssi4 -=4;
      }
      else if (num_beacon_ == rssi4_){
        rssi4 -=4;
      }
    }
    ibeacon=ibeacon+1;
  }
    Serial.print("ibeacon: ");
    Serial.println(ibeacon);
    Serial.println(rssi1);
    Serial.println(rssi2);
    Serial.println(rssi3);
    Serial.println(rssi4);
    float ratio1 = float((A - rssi1) / (24.0));
    d1 = pow(10, ratio1);
    float ratio2 =float ((A - rssi2) / (24.0));
    d2 = pow(10, ratio2);
    float ratio3 = float((A - rssi3) / (24.0));
    d3 = pow(10, ratio3);
    float ratio4 =float ((A - rssi4) / (24.0));
    d4 = pow(10,ratio4);
   Serial.print("d1: ");
   Serial.println(d1);
   Serial.print("d2: ");
   Serial.println(d2);
   Serial.print("d3: ");
   Serial.println(d3);
   Serial.print("d4: ");
   Serial.println(d4);
/////////////////////////////////////////////////
    delete keys;
   keys = _DuLieuIBeacon.keys();
    for (int i = 0; i <=4; i++) {
      String num_beacon=objectNames_bletag[i];
      num_beacon.remove(0, 1);
      num_beacon.remove(1, 1);
//      Serial.println(num_beacon);
      String compare = "ibeacon"+num_beacon;
//      Serial.println(compare);
    for(int j =0 ; j < keys.length(); j++){
      objectNames_ible[j] =JSON.stringify( _DuLieuIBeacon.keys()[j]);
      String ibe= objectNames_ible[j];
      ibe.remove(0,1);
      ibe.remove(8,1);
      Serial.println(ibe);
        if(compare == ibe){
        JSONVar value = _DuLieuIBeacon[keys[j]];
//        Serial.println(value);
//        Serial.println(_DuLieuIBeacon);
        if(i==1){
          Px1=JSON.stringify(value["Px"]).toFloat();
          Py1=JSON.stringify(value["Py"]).toFloat();
            Serial.println(Px1);
            Serial.println(Py1) ;
        }
        else if (i==2){
          Px2=JSON.stringify(value["Px"]).toFloat();
          Py2=JSON.stringify(value["Py"]).toFloat();
             Serial.println(Px2) ;
            Serial.println(Py2) ;
        }
        else if (i==3){
          Px3=JSON.stringify(value["Px"]).toFloat();
          Py3=JSON.stringify(value["Py"]).toFloat();
          Serial.println(Px3) ;
          Serial.println(Py3) ; 
        }
        else if (i==4){
          Px4=JSON.stringify(value["Px"]).toFloat();
          Py4=JSON.stringify(value["Py"]).toFloat();
          Serial.println(Px4) ;
          Serial.println(Py4) ;
        }
      }
    }
    }

////////////////////////////////////////////
///////////////////////////////////////////
/////////////////////////////// set dont touch/////////////////////////////////////
float max_=0;
if(d4 > 0.5){
 max_= findMax(d1,d2,d3,d4);
}
  if(ibeacon >= 4 ){
  if(max_==d1 ){
//    float tam=d1;
    d1=d4;
//    d4 = tam ;
    Px1=Px4;
    Py1=Py4;

  }
  else if(max_==d2){
    d2 =d4;
    Px2=Px4;
    Py2=Py4;
  }
  else if(max_==d3){
    d3=d4;
    Px3=Px4;
    Py3=Py4;
  }
  }
///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
     bool d1_max = false;
     bool d2_max = false;
     bool d3_max = false;
      if(d1> max_distance){
      d1= max_distance;
      d1_max= true;
    }
     else if(d2> max_distance){
      d2= max_distance;
      d2_max= true;
    }
     else if(d3> max_distance){
      d3= max_distance;
      d3_max = true;
    }
/////////////////////////////////////////////
 bool max_in_d1 = false;
 bool max_in_d2 = false;
 bool max_in_d3 = false;
  max_= findMax(d1,d2,d3,0);
  if(max_==d1 ){
  max_in_d1 = true;
  max_in_d2 = false;
  max_in_d3 = false;
  }
  else if(max_==d2){
  max_in_d1 = false;
  max_in_d2 = true;
  max_in_d3 = false;
  }
  else if(max_==d3){
  max_in_d1 = false;
  max_in_d2 = false;
  max_in_d3 = true;
  }
/////////////////////////////////////////////
float Px_max = findMax(Px1, Px2, Px3,0);
float Px_min= findMin(Px1, Px2, Px3);
float Py_max= findMax(Py1, Py2, Py3,0);
float Py_min= findMin(Py1, Py2, Py3);
float px_max = Px_max + 3;
float px_min = Px_min -3;
float py_max = Py_max + 3;
float py_min = Py_min -3;
if(px_min < 0){
  px_min =0;
}

if(py_min < 0){
  py_min =0;
}

Serial.print("Py_min");
Serial.println(Py_min);
Serial.print("pxmax");
Serial.println(px_max);
Serial.print("px_min");
Serial.println(px_min);
Serial.print("py_max");
Serial.println(py_max);
Serial.print("py_min");
Serial.println(py_min);
/////////////////////////////////////////////
   Serial.println(Px1) ;
   Serial.println(Py1) ;
   Serial.println(Px2) ;
   Serial.println(Py2) ;
   Serial.println(Px3) ;
   Serial.println(Py3) ; 
   Serial.print("d1: ");
   Serial.println(d1);
   Serial.print("d2: ");
   Serial.println(d2);
   Serial.print("d3: ");
   Serial.println(d3);
   Serial.print("d4: ");
   Serial.println(d4);
     float A=0;
   float B=0;
   float C=0;
   float D = 0;
   float E = 0;
   float F = 0;
/////////////////////////////////////////////

  if(d1_max == true && d2_max == true && d3_max == true){
    
  }
  else{
    if (max_in_d1){
       float temp = d1;
      int i =0;
     while(1){
   A= (-2*Px1+ 2*Px2);
   B=(-2*Py1 + 2*Py2);
   C=(d1*d1 - d2*d2 - Px1*Px1 + Px2*Px2 - Py1*Py1 + Py2*Py2);
   D=(-2*Px2 + 2*Px3);
   E=(-2*Py2 + 2*Py3);
   F=(d2*d2 - d3*d3 -Px2*Px2 + Px3*Px3 -Py2*Py2 + Py3*Py3);
   Px=(C*E -F*B)/(E*A -B*D);
   Py=(C*D - A*F)/(B*D - A*E);
   if (Px > px_min && Py > py_min && Px < px_max && Py < py_max && !isnan(Px) && !isnan(Py)){
   Serial.print("x: ");
   Serial.println(Px);
   Serial.print("y: ");
   Serial.println(Py);
   break;
   }
   i = i +1; 
   if(i <= 60){
   d1= d1 - 0.03;
   }
   else if (60<i<=100){
    if(i == 61){
      d1 = temp;
    }
    d1 = d1 + 0.03;
   }
   else if (i > 100){
    break;
   }
     }
    }
    else if (max_in_d2){
      int i =0;
      float temp = d2;
        while(1){
   A= (-2*Px1+ 2*Px2);
   B=(-2*Py1 + 2*Py2);
   C=(d1*d1 - d2*d2 - Px1*Px1 + Px2*Px2 - Py1*Py1 + Py2*Py2);
   D=(-2*Px2 + 2*Px3);
   E=(-2*Py2 + 2*Py3);
   F=(d2*d2 - d3*d3 -Px2*Px2 + Px3*Px3 -Py2*Py2 + Py3*Py3);
   Px=(C*E -F*B)/(E*A -B*D);
   Py=(C*D - A*F)/(B*D - A*E);
   if (Px > px_min && Py > py_min && Px < px_max && Py < py_max && !isnan(Px) && !isnan(Py) ){
   Serial.print("x: ");
   Serial.println(Px);
   Serial.print("y: ");
   Serial.println(Py);
   break;
   }

   i = i +1; 
    if(i <= 60){
   d2= d2 - 0.03;
   }
   else if (60<i<=100){
    if(i == 61){
      d2 = temp;
    }
    d2 = d2 + 0.03;
   }
   else if (i > 100){
    break;
   }
     }
    }
    else if (max_in_d3){
      float temp = d3;
      int i =0;
        while(1){
   A= (-2*Px1+ 2*Px2);
   B=(-2*Py1 + 2*Py2);
   C=(d1*d1 - d2*d2 - Px1*Px1 + Px2*Px2 - Py1*Py1 + Py2*Py2);
   D=(-2*Px2 + 2*Px3);
   E=(-2*Py2 + 2*Py3);
   F=(d2*d2 - d3*d3 -Px2*Px2 + Px3*Px3 -Py2*Py2 + Py3*Py3);
   Px=(C*E -F*B)/(E*A -B*D);
   Py=(C*D - A*F)/(B*D - A*E);
   if (Px > px_min && Py > py_min && Px < px_max && Py < py_max && !isnan(Px) && !isnan(Py) ){
   Serial.print("x: ");
   Serial.println(Px);
   Serial.print("y: ");
   Serial.println(Py);
   break;
   }
   i = i +1; 
    if(i <= 60){
   d3= d3 - 0.03;
   }
   else if (60<i<=100){
    if(i == 61){
      d3 = temp;
    }
    d3 = d3 + 0.03;
   }
   else if (i > 100){
    break;
   }
     }
    }
  }
/////////////////////////////////////////////

//   Serial.println(Px1) ;
//   Serial.println(Py1) ;
//   Serial.println(Px2) ;
//   Serial.println(Py2) ;
//   Serial.println(Px3) ;
//   Serial.println(Py3) ; 
//   Serial.print("d1: ");
//   Serial.println(d1);
//   Serial.print("d2: ");
//   Serial.println(d2);
//   Serial.print("d3: ");
//   Serial.println(d3);
//   Serial.print("d4: ");
//   Serial.println(d4);
//   float A=0;
//   float B=0;
//   float C=0;
//   float D = 0;
//   float E = 0;
//   float F = 0;
//   A= (-2*Px1+ 2*Px2);
//   B=(-2*Py1 + 2*Py2);
//   C=(d1*d1 - d2*d2 - Px1*Px1 + Px2*Px2 - Py1*Py1 + Py2*Py2);
//   D=(-2*Px2 + 2*Px3);
//   E=(-2*Py2 + 2*Py3);
//   F=(d2*d2 - d3*d3 -Px2*Px2 + Px3*Px3 -Py2*Py2 + Py3*Py3);
//   Px=(C*E -F*B)/(E*A -B*D);
//   Py=(C*D - A*F)/(B*D - A*E);
//   Serial.print("x: ");
//   Serial.println(Px);
//   Serial.print("y: ");
//   Serial.println(Py);
}


float findMax(float a1, float a2, float a3, float a4) {
  float maxValue = a1;
  
  if (a2 > maxValue) {
    maxValue = a2;
  }
  
  if (a3 > maxValue) {
    maxValue = a3;
  }
  
  if (a4 > maxValue) {
    maxValue = a4;
  }
  
  return maxValue;
}

float findMin(float a1, float a2, float a3) {
  float minValue = a1;
  
  if (a2 < minValue) {
    minValue = a2;
  }
  
  if (a3 < minValue) {
    minValue = a3;
  }
  
//  if (a4 < maxValue) {
//    minValue = a4;
//  }
  
  return minValue;
}
void main_function(){
  JSONVar reciever;
  if (SerialPort.available()) {
    receivedData = SerialPort.readStringUntil('\n');
    Serial.println("Received from ESP32_ble_gateway_1: " + receivedData);
     reciever = JSON.parse(receivedData);
     reciever_uart= true;
  }

   if(reciever_uart){
    caculate_location(reciever);
    Gopdulieu();
    reciever_uart= false;
    
   }
   if (_Flag.t20s){
    _DuLieuIBeacon= GETDuLieuIBeacon();
   }
   
   if(_Flag.t30s){
  if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password);  
  } 
   }
  
}
