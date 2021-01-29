/*
 * This is an example of sending and receiving (FD) on its own,
 * connect two channels to each other using jumpers.
 */

#include <SPI.h>
#include "mcp2518fd_can.h"

// Set SPI CS Pin according to your hardware
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
// *****************************************
// For Arduino MCP2515 Hat:
// SPI_CS Pin: D9

const int SPI_CS_PIN_SEND = 9;
const int SPI_CS_PIN_RECEIVE = 2;

mcp2518fd CAN_SEND(SPI_CS_PIN_SEND);
mcp2518fd CAN_RECEIVE(SPI_CS_PIN_RECEIVE);

// CANFD could carry data up to 64 bytes
#define MAX_DATA_SIZE 64

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while (!SERIAL_PORT_MONITOR) {} // wait for Serial

    /*
     * To compatible with MCP2515 API,
     * default mode is CAN_CLASSIC_MODE
     * Now set to CANFD mode.
     */
    CAN_SEND.setMode(CAN_NORMAL_MODE);
    CAN_RECEIVE.setMode(CAN_NORMAL_MODE);
    
    if (CAN_SEND.begin(CAN_500K_1M) != 0 || CAN_RECEIVE.begin(CAN_500K_1M) != 0) {
        SERIAL_PORT_MONITOR.println("CAN-BUS initiliased error!");
        while (1);
    }
    byte send_mode = CAN_SEND.getMode();
    byte receive_mode = CAN_RECEIVE.getMode();
    SERIAL_PORT_MONITOR.print("CAN BUS Send Mode = "); SERIAL_PORT_MONITOR.println(send_mode);
    SERIAL_PORT_MONITOR.print("CAN BUS Receive Mode = "); SERIAL_PORT_MONITOR.println(receive_mode);
    SERIAL_PORT_MONITOR.println("CAN BUS Shield init ok!");
}

unsigned char stmp[MAX_DATA_SIZE] = {0};
unsigned char len = 0;
unsigned char buf[MAX_DATA_SIZE];

void loop() {
    // send data:  id = 0x00, standrad frame, data len = 64, stmp: data buf
    stmp[MAX_DATA_SIZE - 1] = stmp[MAX_DATA_SIZE - 1] + 1;
    if (stmp[MAX_DATA_SIZE - 1] == 100) {
        stmp[MAX_DATA_SIZE - 1] = 0;

        stmp[MAX_DATA_SIZE - 2] = stmp[MAX_DATA_SIZE - 2] + 1;
        if (stmp[MAX_DATA_SIZE - 2] == 100) {
            stmp[MAX_DATA_SIZE - 2] = 0;

            stmp[MAX_DATA_SIZE - 3] = stmp[MAX_DATA_SIZE - 3] + 1;
        }
    }

    CAN_SEND.sendMsgBuf(0x00, 0, CANFD::len2dlc(MAX_DATA_SIZE), stmp);
    delay(100);                       // send data per 100ms
    SERIAL_PORT_MONITOR.println("CAN BUS sendMsgBuf ok!");

    // ---------------------
    if (CAN_MSGAVAIL == CAN_RECEIVE.checkReceive()) {
        // read data,  len: data length, buf: data buf
        SERIAL_PORT_MONITOR.println("checkReceive");
        CAN_RECEIVE.readMsgBuf(&len, buf);
        // print the data
        for (int i = 0; i < len; i++) {
            SERIAL_PORT_MONITOR.print(buf[i]);
            SERIAL_PORT_MONITOR.print(' ');
        }
        SERIAL_PORT_MONITOR.println();
    }
    SERIAL_PORT_MONITOR.println("---LOOP END---");
}

// END FILE
