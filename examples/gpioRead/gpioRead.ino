// demo: Use TX2RTS as digital input
// adlerweb, 2017-06-24
#include <SPI.h>
#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

const int SPI_CS_PIN = BCM8;
mcp2518fd* controller;



void setup() {
    SERIAL.begin(115200);
    while(!Serial){};
    controller = new mcp2518fd();
    controller->mcp_canbus(SPI_CS_PIN);
    while (0 != controller->begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
        SERIAL.println("CAN init failed, retry");
        delay(100);
    }
    SERIAL.println("CAN init ok");

    if (controller->mcpPinMode(GPIO_PIN_0, GPIO_MODE_INT)) {
        SERIAL.println("GPIO_PIN_0 is now an input");
    } else {
        SERIAL.println("Could not switch GPIO_PIN_0");
    }
}

void loop() {
    SERIAL.print("GPIO_PIN_0 is currently ");
    SERIAL.println(controller->mcpDigitalRead(GPIO_PIN_0));
    delay(500);
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
