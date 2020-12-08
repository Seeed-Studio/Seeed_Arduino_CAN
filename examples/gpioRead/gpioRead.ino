// demo: Use TX2RTS as digital input
// adlerweb, 2017-06-24
#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2518fd_can.h"

#define CAN_2518FD
#define SPI_CS_PIN BCM8

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#ifdef CAN_2518FD
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

void setup() {
    SERIAL.begin(115200);
#ifdef CAN_2518FD
    while (0 != CAN.begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
#endif
#ifdef CAN_2515
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
#endif
   
        SERIAL.println("CAN init failed, retry");
        delay(100);
    }
    SERIAL.println("CAN init ok");
#ifdef CAN_2518FD
    if (CAN.mcpPinMode(GPIO_PIN_0, GPIO_MODE_INT)) {
#else
    if (CAN.mcpPinMode(MCP_TX2RTS, MCP_PIN_IN)) {
#endif
        SERIAL.println("TX2RTS is now an input");
    } else {
        SERIAL.println("Could not switch TX2RTS");
    }
}

void loop() {
    SERIAL.print("TX2RTS is currently ");
#ifdef CAN_2518FD
    SERIAL.println(CAN.mcpDigitalRead(GPIO_PIN_0));
#else
    SERIAL.println(CAN.mcpDigitalRead(MCP_TX2RTS));
#endif
    delay(500);
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
