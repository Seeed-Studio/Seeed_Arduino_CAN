/*
 * demo: CAN-BUS Shield, send random id/type/data
 * This code has a little same with linux can-utils 'cangen -mv'
 *
 * Copyright (C) 2020 Seeed Technology Co.,Ltd.
 */
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
#define MAX_DATA_SIZE 64
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#define MAX_DATA_SIZE 8
#endif

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while(!Serial){};

#ifdef CAN_2518FD
    while (0 != CAN.begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
#else
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
#endif
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");

    randomSeed(millis());
}

uint32_t id;
uint8_t  type; // bit0: ext, bit1: rtr
unsigned len;
byte cdata[MAX_DATA_SIZE] = {0};

void loop() {
    type = random(4);
    if (type & 0x1) {
        // total 29 bits
        // Arduino AVR only generate up to 16 bits random number
        id = random(0x1U << 14);
        id |= (uint32_t)random(0x1U << 15) << 14;
    } else {
        id = random(0x1U << 11);
    }
    if (type & 0x2) {
        len = 0;
        // remote frame could also carry data
        // but don't do it.
        // len = random(0, MAX_DATA_SIZE + 1);
    } else {
        len = random(0, MAX_DATA_SIZE + 1);
    }

    int i;
    for (i = 0; i < len; i++) {
        cdata[i] = random(0x100);
    }

    CAN.sendMsgBuf(id, bool(type & 0x1),
                       bool(type & 0x2),
                       len, cdata);
    char prbuf[0x40];
    int n;

    /* Displayed type:
     *
     * 0x00: standard data frame
     * 0x02: extended data frame
     * 0x30: standard remote frame
     * 0x32: extended remote frame
     */
    static const byte type2[] = {0x00, 0x02, 0x30, 0x32};
    n = sprintf(prbuf, "TX: [%08lX](%02X) ", (unsigned long)id, type2[type]);
    // n = sprintf(prbuf, "TX: [%08lX](%02X) ", id, type);

    for (i = 0; i < len; i++) {
        n += sprintf(prbuf + n, "%02X ", cdata[i]);
    }
    SERIAL_PORT_MONITOR.println(prbuf);

    unsigned d = random(30);
    SERIAL_PORT_MONITOR.println(d);
    delay(d);
}
// END FILE
