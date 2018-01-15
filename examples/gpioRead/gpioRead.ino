// demo: Use TX2RTS as digital input
// adlerweb, 2017-06-24
#include <SPI.h>
#include "mcp_can.h"

#define SPI_CS_PIN 10

MCP_CAN CAN(SPI_CS_PIN); // Set CS pin


void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN init failed, retry");
        delay(100);
    }
    Serial.println("CAN init ok");

    if(CAN.pinMode(MCP_TX2RTS, MCP_PIN_IN))
    {
        Serial.println("TX2RTS is now an input");
    }
    else
    {
        Serial.println("Could not switch TX2RTS");
    }
}

void loop()
{
    Serial.print("TX2RTS is currently ");
    Serial.println(CAN.digitalRead(MCP_TX2RTS));
    delay(500);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
