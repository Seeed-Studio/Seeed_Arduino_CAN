// demo: sendFD
//   show how to send CANFD frames.
//   only used with MCP2518FD shield.

#include <SPI.h>
#include "mcp2518fd_can.h"

#if defined(SEEED_WIO_TERMINAL)
const int SPI_CS_PIN = BCM8;
#else
const int SPI_CS_PIN = 9;
#endif

// CANFD could carry data up to 64 bytes
#define MAX_DATA_SIZE 64

mcp2518fd CAN(SPI_CS_PIN); // Set CS pin

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while (!SERIAL_PORT_MONITOR) {}

    /*
     * To compatible with MCP2515 API,
     * default mode is CAN_CLASSIC_MODE
     * Now set to CANFD mode.
     */
    CAN.setMode(CAN_NORMAL_MODE);

    // init can bus : arbitration bitrate = 500k, data bitrate = 1M
    while (0 != CAN.begin(CAN_500K_1M)) {
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");

    byte mode = CAN.getMode();
    SERIAL_PORT_MONITOR.print("CAN mode = ");
    SERIAL_PORT_MONITOR.println(mode);
}

unsigned char stmp[MAX_DATA_SIZE] = {0};
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

    CAN.sendMsgBuf(0x00, 0, CANFD::len2dlc(MAX_DATA_SIZE), stmp);
    delay(100);                       // send data per 100ms
    SERIAL_PORT_MONITOR.println("CAN BUS sendMsgBuf ok!");
}

// END FILE
