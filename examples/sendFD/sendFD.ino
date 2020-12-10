// demo: CAN-BUS Shield, send data
// loovee@seeed.cc


#include <SPI.h>
#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#define CAN_2518FD
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

#ifdef CAN_2518FD
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

void setup() {
    SERIAL.begin(115200);
    while(!Serial){};
    CAN.setMode(0); // Set FD Mode
    while (0 != CAN.begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k     
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    byte mode = CAN.getMode();
    SERIAL.print("CAN BUS get mode = ");
    SERIAL.println(mode);
    SERIAL.println("CAN BUS Shield init ok!");
}

unsigned char stmp[64] = {0};
void loop() {
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    stmp[63] = stmp[63] + 1;
    if (stmp[63] == 100) {
        stmp[63] = 0;
        stmp[62] = stmp[62] + 1;

        if (stmp[62] == 100) {
            stmp[62] = 0;
            stmp[61] = stmp[62] + 1;
        }
    }

    CAN.sendMsgBuf(0x00, 0, 15, stmp);
    delay(100);                       // send data per 100ms
    SERIAL.println("CAN BUS sendMsgBuf ok!");
}

// END FILE
