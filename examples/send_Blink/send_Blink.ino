// demo: CAN-BUS Shield, send data
#include <SPI.h>

#define CAN_2515
// #define CAN_2518FD

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10

// Set SPI CS Pin according to your hardware
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
// *****************************************
// For Arduino MCP2515 Hat:
// SPI_CS Pin: D9

#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif                              // Set CS pin

const int ledHIGH    = 1;
const int ledLOW     = 0;
                           

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);

#ifdef CAN_2518FD
    while (0 != CAN.begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
#else
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
#endif
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
}

unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};

void loop() {
    SERIAL_PORT_MONITOR.println("In loop");
    // send data:  id = 0x70, standard frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x70, 0, 8, stmp);
    delay(1000);                       // send data once per second
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
