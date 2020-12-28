/*
    This is an example of sending and receiving (FD) on its own, connect two channels to each other using jumpers.
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

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while(!Serial); // wait for Serial
    CAN_SEND.setMode(0);
    CAN_RECEIVE.setMode(0);
    
    if (CAN_SEND.begin((byte)CAN_500K_1M) != 0 || CAN_RECEIVE.begin((byte)CAN_500K_1M) != 0) {
      Serial.println("CAN-BUS initiliased error!");
      while(1);
    }
    byte send_mode = CAN_SEND.getMode();
    byte receive_mode = CAN_RECEIVE.getMode();
    SERIAL_PORT_MONITOR.print("CAN BUS Send Mode = "); SERIAL_PORT_MONITOR.println(send_mode);
    SERIAL_PORT_MONITOR.print("CAN BUS Receive Mode = "); SERIAL_PORT_MONITOR.println(receive_mode);
    SERIAL_PORT_MONITOR.println("CAN BUS Shield init ok!");
}

unsigned char stmp[64] = {0};
unsigned char len = 0;
unsigned char buf[64];

void loop() {
    stmp[63] = stmp[63] + 1;
    if (stmp[63] == 100) {
        stmp[63] = 0;
        stmp[62] = stmp[62] + 1;

        if (stmp[62] == 100) {
            stmp[62] = 0;
            stmp[61] = stmp[61] + 1;
        }
    }

    CAN_SEND.sendMsgBuf(0x00, 0, 15, stmp);
    delay(100);                       // send data per 100ms
    SERIAL_PORT_MONITOR.println("CAN BUS sendMsgBuf ok!");

    // ---------------------
        
    if (CAN_MSGAVAIL == CAN_RECEIVE.checkReceive()) {
    // read data,  len: data length, buf: data buf
      SERIAL_PORT_MONITOR.println("checkReceive");
      CAN_RECEIVE.readMsgBuf(&len, buf);
    // print the data
    for (int i = 0; i < len; i++) {
        SERIAL_PORT_MONITOR.print(buf[i]); SERIAL_PORT_MONITOR.print("");
    } 
    SERIAL_PORT_MONITOR.println();
    }
    SERIAL_PORT_MONITOR.println("---LOOP END---");
}

// END FILE
