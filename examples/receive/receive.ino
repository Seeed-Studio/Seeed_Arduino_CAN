// demo: CAN-BUS Shield, send data

#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = CANBUS_PIN_SS;
const int CAN_INT_PIN = CANBUS_PIN_INT;

mcp2518fd* controller;

//unsigned char flagRecv = 0;

void setup() {
   SERIAL.begin(115200);
   while(!Serial){};
   controller = new mcp2518fd();
   controller->mcp_canbus(SPI_CS_PIN);
   while (0 != controller->begin()) {            // init can bus : baudrate = 500k
       SERIAL.println("CAN BUS Shield init fail");
       SERIAL.println(" Init CAN BUS Shield again");
       delay(100);
   }
   SERIAL.println("CAN BUS Shield init ok!");
//   attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2518fd_ISR, FALLING); // start interrupt
}

void MCP2518fd_ISR() {
	flagRecv = 1;
}
 
void loop() {
    controller->mcp2518fd_receiveMsg();
    delay(100);
                            // send data per 100ms
}

// END FILE
