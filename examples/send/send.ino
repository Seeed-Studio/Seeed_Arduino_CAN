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
const int SPI_CS_PIN = BCM8;
mcp2518fd* controller;

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
}
 
unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void loop() {
   // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
   stmp[7] = stmp[7] + 1;
   if (stmp[7] == 100) {
       stmp[7] = 0;
       stmp[6] = stmp[6] + 1;

       if (stmp[6] == 100) {
           stmp[6] = 0;
           stmp[5] = stmp[6] + 1;
       }
   }

   controller->mcp2518fd_sendMsgBuf(stmp,8);
   delay(100);                       // send data per 100ms
}

// END FILE
