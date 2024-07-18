
#include <BLEPeripheral.h>
#include <iBeacon.h>
#include<delay.h>

#if !defined(NRF51) && !defined(NRF52) && !defined(__RFduino__)
#error "This example only works with nRF51 boards"
#endif

iBeacon beacon;
BLEPeripheral blePeripheral;
  char* uuid                   = "a196c876-de8c-4c47-ab5a-d7afd5ae7127";
  unsigned short major         = 1234;
  unsigned short minor         = 4;
  unsigned short measuredPower = -55;
void setup() {
  beacon.begin(uuid, major, minor, measuredPower);
}

void loop() {
//  beacon.begin(uuid, major, minor, measuredPower);
//  delay(10000);// delay 5s 
////beacon.loop();
//  blePeripheral.end();
//  delay(2000);//delay 3s
beacon.loop();
}
