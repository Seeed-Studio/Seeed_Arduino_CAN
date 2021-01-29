// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp2518fd_can.h"

#define CAN_2518FD

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif

#ifdef CAN_2518FD
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[64];

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while (!SERIAL_PORT_MONITOR) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING); // start interrupt
    CAN.setMode(CAN_NORMAL_MODE);

    // init can bus : arbitration bitrate = 500k, data bitrate = 1M
    while (0 != CAN.begin(CAN_500K_1M)) {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
    byte mode = CAN.getMode();
    SERIAL_PORT_MONITOR.printf("CAN BUS mode = %d\n\r", mode);
}

void MCP2515_ISR() {
    flagRecv = 1;
}

void loop() {
    if (flagRecv) {
        // check if get data

        flagRecv = 0;                   // clear flag
        SERIAL_PORT_MONITOR.println("into loop");
        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (CAN_MSGAVAIL == CAN.checkReceive()) {
            // read data,  len: data length, buf: data buf
            SERIAL_PORT_MONITOR.println("checkReceive");
            CAN.readMsgBuf(&len, buf);
            // print the data
            for (int i = 0; i < len; i++) {
                SERIAL_PORT_MONITOR.print(buf[i]); SERIAL_PORT_MONITOR.print("\t");
            }
            SERIAL_PORT_MONITOR.println();
        }
    }
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
