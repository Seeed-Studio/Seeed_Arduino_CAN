// demo: set_mask_filter_send
// this demo will show you how to use mask and filter

#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN(10);                                      // Set CS to pin 10

void setup()
{
    Serial.begin(115200);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}

unsigned char stmp[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void loop()
{
    for(int id=0; id<10; id++)
    {
        memset(stmp, id, sizeof(stmp));                 // set id to send data buff
        CAN.sendMsgBuf(id, 0, sizeof(stmp), stmp);
        delay(100);
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
