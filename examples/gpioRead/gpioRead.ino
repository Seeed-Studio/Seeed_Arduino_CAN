// demo: Use TX2RTS as digital input
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

    if(CAN.mcpPinMode(MCP_TX2RTS, MCP_PIN_IN))
    {
        SERIAL.println("TX2RTS is now an input");
    }
    else
    {
        SERIAL.println("Could not switch TX2RTS");
    }
}

void loop()
{
    SERIAL.print("TX2RTS is currently ");
    SERIAL.println(CAN.mcpDigitalRead(MCP_TX2RTS));
    delay(500);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
