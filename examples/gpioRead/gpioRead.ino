// demo: Use TX2RTS as digital input
// adlerweb, 2017-06-24
#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2518fd_can.h"

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
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        SERIAL_PORT_MONITOR.println("CAN init failed, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok");
#ifdef CAN_2518FD
    if (CAN.mcpPinMode(GPIO_PIN_0, GPIO_MODE_INT)) {
#else
    if (CAN.mcpPinMode(MCP_TX2RTS, MCP_PIN_IN)) {
#endif
        SERIAL_PORT_MONITOR.println("TX2RTS is now an input");
    } else {
        SERIAL_PORT_MONITOR.println("Could not switch TX2RTS");
    }
}

void loop() {
    SERIAL_PORT_MONITOR.print("TX2RTS is currently ");
#ifdef CAN_2518FD
    SERIAL_PORT_MONITOR.println(CAN.mcpDigitalRead(GPIO_PIN_0));
#else
    SERIAL_PORT_MONITOR.println(CAN.mcpDigitalRead(MCP_TX2RTS));
#endif
    delay(500);
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
