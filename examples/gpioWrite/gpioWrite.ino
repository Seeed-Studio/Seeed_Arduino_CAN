// demo: Use RX0BF and RX1BF as digital outputs
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

    if(CAN.pinMode(MCP_RX0BF, MCP_PIN_OUT))
    {
        Serial.println("RX0BF is now an output");
    }
    else
    {
        Serial.println("Could not switch RX0BF");
    }

    if(CAN.pinMode(MCP_RX1BF, MCP_PIN_OUT))
    {
        Serial.println("RX1BF is now an output");
    }
    else
    {
        Serial.println("Could not switch RX1BF");
    }
}

void loop()
{
    Serial.println("10");
    CAN.digitalWrite(MCP_RX0BF, HIGH);
    CAN.digitalWrite(MCP_RX1BF, LOW);
    delay(500);
    Serial.println("01");
    CAN.digitalWrite(MCP_RX0BF, LOW);
    CAN.digitalWrite(MCP_RX1BF, HIGH);
    delay(500);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
