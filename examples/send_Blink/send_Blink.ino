// demo: CAN-BUS Shield, send data
#include "mcp2518fd_can.h"
#include <SPI.h>

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
const int ledHIGH    = 1;
const int ledLOW     = 0;
                 
void setup() {
    SERIAL.begin(115200);
    while(!Serial){};
    controller = new mcp2518fd();
    controller->mcp_canbus(SPI_CS_PIN);
    while (CAN_OK != controller->begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};

void loop() {
    SERIAL.println("In loop");
    // send data:  id = 0x70, standard frame, data len = 8, stmp: data buf
    controller->sendMsgBuf(0x70, 0, 8, stmp);
    delay(1000);                       // send data once per second
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
