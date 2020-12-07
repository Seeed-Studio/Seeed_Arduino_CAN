// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN =  BCM8;
const int CAN_INT_PIN = BCM25;

mcp2518fd* controller;
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

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

    attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING); // start interrupt
}

void MCP2515_ISR() {
    flagRecv = 1;
    SERIAL.println("into MCP2515_ISR\n\r");
}

void loop() {
    if (flagRecv) {
        // check if get data

        flagRecv = 0;                   // clear flag

        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (1 == controller->checkReceive()) {
            // read data,  len: data length, buf: data buf
            controller->readMsgBuf(&len, buf);

            // print the data
            for (int i = 0; i < len; i++) {
                SERIAL.print(buf[i]); SERIAL.print("\t");
            }
            SERIAL.println();
        }
    }
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
