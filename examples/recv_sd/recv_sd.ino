// demo: CAN-BUS Shield, receive data and save to sd card, can.csv
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// NOTE: Only CAN Bus Shield V2.0 has SD slot, for other versions, you need an SD card Shield as well
// loovee, Jun 12, 2017

#include <SPI.h>
#include <SD.h>

File myFile;

#define CAN_2515
// #define CAN_2518FD

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
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING); // start interrupt
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
    
    if (!SD.begin(4)) {
        SERIAL_PORT_MONITOR.println("SD init fail!");
        while (1);
    }
    SERIAL_PORT_MONITOR.println("SD init OK.");
}

void MCP2515_ISR() {
    flagRecv = 1;
}

void loop() {
    if (flagRecv) {
        // check if get data

        flagRecv = 0;                   // clear flag
        unsigned long id = 0;

        myFile = SD.open("can.csv", FILE_WRITE);

        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (CAN_MSGAVAIL == CAN.checkReceive()) {
            // read data,  len: data length, buf: data buf
            CAN.readMsgBufID(&id, &len, buf);

            SERIAL_PORT_MONITOR.print(id);
            SERIAL_PORT_MONITOR.print(",");
            myFile.print(id);
            myFile.print(",");

            for (int i = 0; i < len; i++) {
                SERIAL_PORT_MONITOR.print(buf[i]);
                SERIAL_PORT_MONITOR.print(",");

                myFile.print(buf[i]);
                myFile.print(",");
            }
            SERIAL_PORT_MONITOR.println();
            myFile.println();
        }

        myFile.close();
    }
}

// END FILE
