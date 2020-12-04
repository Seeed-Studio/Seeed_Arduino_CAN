// demo: Use RX0BF and RX1BF as digital outputs
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
    while (CAN_OK != controller->begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
        SERIAL.println("CAN init failed, retry");
        delay(100);
    }
    SERIAL.println("CAN init ok");

    if (controller->mcpPinMode(GPIO_PIN_0, GPIO_MODE_GPIO)) {
        SERIAL.println("GPIO_PIN_0 is now an output");
    } else {
        SERIAL.println("GPIO_PIN_0 switch RX0BF");
    }

    if (controller->mcpPinMode(GPIO_PIN_1, GPIO_MODE_GPIO)) {
        SERIAL.println("GPIO_PIN_1 is now an output");
    } else {
        SERIAL.println("Could not switch GPIO_PIN_1");
    }
}

void loop() {
    SERIAL.println("10");
    controller->mcpDigitalWrite(GPIO_PIN_0, HIGH);
    controller->mcpDigitalWrite(GPIO_PIN_1, LOW);
    delay(500);
    SERIAL.println("01");
    controller->mcpDigitalWrite(GPIO_PIN_0, GPIO_LOW);
    controller->mcpDigitalWrite(GPIO_PIN_1, GPIO_HIGH);
    delay(500);
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
