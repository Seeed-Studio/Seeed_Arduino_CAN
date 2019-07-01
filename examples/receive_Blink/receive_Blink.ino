// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

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
const int LED        = 8;
boolean ledON        = 1;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    SERIAL.begin(115200);
    pinMode(LED,OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.println("get data from ID: 0x");
        SERIAL.println(canId, HEX);

        for(int i = 0; i<len; i++)    // print the data
        {
            SERIAL.print(buf[i]);
            SERIAL.print("\t");
            if(ledON && i==0)
            {

                digitalWrite(LED, buf[i]);
                ledON = 0;
                delay(500);
            }
            else if((!(ledON)) && i==4)
            {

                digitalWrite(LED, buf[i]);
                ledON = 1;
            }
        }
        SERIAL.println();
    }
}

//END FILE