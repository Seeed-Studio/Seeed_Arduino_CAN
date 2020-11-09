// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

//Mcp_Factory<mcp2518fd> my_Factory;


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 23;
mcp2518fd* controller;

void setup() {
   SERIAL.begin(115200);
   controller = new mcp2518fd();
  controller->mcp_canbus(SPI_CS_PIN);
  while (0 != controller->begin()) {            // init can bus : baudrate = 500k
       SERIAL.println("CAN BUS Shield init fail");
       SERIAL.println(" Init CAN BUS Shield again");
       delay(100);
   }
   SERIAL.println("CAN BUS Shield init ok!");
}
 
void loop() {
   controller->mcp2518fd_receiveMsg();
   delay(100);                       // send data per 100ms
}

// END FILE
