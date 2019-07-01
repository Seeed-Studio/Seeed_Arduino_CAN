// demo: CAN-BUS Shield, receive data with interrupt mode, and set mask and filter
//
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-7-8

#include <SPI.h>
#include "mcp_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup()
{
    SERIAL.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");

    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt


    /*
     * set mask, set both the mask to 0x3ff
     */
    CAN.init_Mask(0, 0, 0x3ff);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x3ff);


    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, 0, 0x04);                          // there are 6 filter in mcp2515
    CAN.init_Filt(1, 0, 0x05);                          // there are 6 filter in mcp2515

    CAN.init_Filt(2, 0, 0x06);                          // there are 6 filter in mcp2515
    CAN.init_Filt(3, 0, 0x07);                          // there are 6 filter in mcp2515
    CAN.init_Filt(4, 0, 0x08);                          // there are 6 filter in mcp2515
    CAN.init_Filt(5, 0, 0x09);                          // there are 6 filter in mcp2515

}

void MCP2515_ISR()
{
    flagRecv = 1;
}

void loop()
{
    if(flagRecv)                   // check if get data
    {

        flagRecv = 0;                // clear flag
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        SERIAL.println("\r\n------------------------------------------------------------------");
        SERIAL.print("Get Data From id: ");
        SERIAL.println(CAN.getCanId());
        for(int i = 0; i<len; i++)    // print the data
        {
            SERIAL.print("0x");
            SERIAL.print(buf[i], HEX);
            SERIAL.print("\t");
        }
        SERIAL.println();

    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/