// demo: Use RX0BF and RX1BF as digital outputs
// adlerweb, 2017-06-24
#include <SPI.h>
#include "mcp_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

#define SPI_CS_PIN 10

MCP_CAN CAN(SPI_CS_PIN); // Set CS pin


void setup()
{
    SERIAL.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN init failed, retry");
        delay(100);
    }
    SERIAL.println("CAN init ok");

    if(CAN.mcpPinMode(MCP_RX0BF, MCP_PIN_OUT))
    {
        SERIAL.println("RX0BF is now an output");
    }
    else
    {
        SERIAL.println("Could not switch RX0BF");
    }

    if(CAN.mcpPinMode(MCP_RX1BF, MCP_PIN_OUT))
    {
        SERIAL.println("RX1BF is now an output");
    }
    else
    {
        SERIAL.println("Could not switch RX1BF");
    }
}

void loop()
{
    SERIAL.println("10");
    CAN.mcpDigitalWrite(MCP_RX0BF, HIGH);
    CAN.mcpDigitalWrite(MCP_RX1BF, LOW);
    delay(500);
    SERIAL.println("01");
    CAN.mcpDigitalWrite(MCP_RX0BF, LOW);
    CAN.mcpDigitalWrite(MCP_RX1BF, HIGH);
    delay(500);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
