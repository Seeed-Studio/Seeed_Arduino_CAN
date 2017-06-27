// demo: CAN-BUS Shield, receive data and save to sd card, can.csv
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// NOTE: Only CAN Bus Shield V2.0 has SD slot, for other versions, you need an SD card Shield as well
// loovee, Jun 12, 2017

#include <SPI.h>
#include "mcp_can.h"
#include <SD.h>

File myFile;

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_CAN    = 9;
const int SPI_CS_SD     = 4;

MCP_CAN CAN(SPI_CS_CAN);                                    // Set CS pin

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
    
    if (!SD.begin(4)) {
        Serial.println("SD initialization failed!");
        while(1);
    }
    Serial.println("SD initialization done.");
}

void MCP2515_ISR()
{
    flagRecv = 1;
}

void loop()
{
    if(flagRecv) 
    {                                   // check if get data

        flagRecv = 0;                   // clear flag
        unsigned long id= 0;
        
        myFile = SD.open("can.csv", FILE_WRITE);

        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (CAN_MSGAVAIL == CAN.checkReceive()) 
        {
            // read data,  len: data length, buf: data buf
            CAN.readMsgBufID(&id, &len, buf);
  
            Serial.print(id);
            Serial.print(",");
            myFile.print(id);
            myFile.print(",");
            
            for(int i = 0; i<len; i++)
            {
                Serial.print(buf[i]);
                Serial.print(",");
                
                myFile.print(buf[i]);
                myFile.print(",");
            }
            Serial.println();
            myFile.println();
        }
        
        myFile.close();
    }
}

// END FILE