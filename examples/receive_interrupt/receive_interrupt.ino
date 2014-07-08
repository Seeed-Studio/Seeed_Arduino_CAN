// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"

MCP_CAN CAN(10);                                            // Set CS to pin 10


unsigned char Flag_Recv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

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
    
    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
}

void MCP2515_ISR()
{
     Flag_Recv = 1;
}

void loop()
{
    if(Flag_Recv)                   // check if get data
    {
    
        Flag_Recv = 0;                // clear flag
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);Serial.print("\t");
        }
        Serial.println();
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/