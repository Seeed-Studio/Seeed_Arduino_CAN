/*
 * demo: CAN-BUS Shield, send random id/type/data
 * This code has a little same with linux can-utils 'cangen -v'
 *
 * Copyright (C) 2020 Seeed Technology Co.,Ltd.
 */
#include <SPI.h>

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

// TEST TEST MCP2518FD CAN2.0 data transfer
#define MAX_DATA_SIZE 8
// To TEST MCP2518FD CANFD data transfer, uncomment below lines
// #undef  MAX_DATA_SIZE
// #define MAX_DATA_SIZE 64

#endif//CAN_2518FD

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#define MAX_DATA_SIZE 8
#endif

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while (!SERIAL_PORT_MONITOR) {}

    #if MAX_DATA_SIZE > 8
    /*
     * To compatible with MCP2515 API,
     * default mode is CAN_CLASSIC_MODE
     * Now set to CANFD mode.
     */
    CAN.setMode(CAN_NORMAL_MODE);
    #endif
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
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
    #if MAX_DATA_SIZE > 8
    // pad CANFD extra bytes with 0
    for (i = len; i < MAX_DATA_SIZE; i++) {
        cdata[i] = 0;
    }
    #endif

    CAN.sendMsgBuf(id, bool(type & 0x1),
                       bool(type & 0x2),
                     #if MAX_DATA_SIZE > 8
                       CANFD::len2dlc(len),
                     #else
                       len,
                     #endif
                       cdata);

    char prbuf[32 + MAX_DATA_SIZE * 3];
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
