// demo: CAN-BUS Shield, send data
// This is an example of sending both standard frames and extended frames simultaneously. 
// I have used two arrays, STANDARD_CAN_IDS and EXTENDED_CAN_IDS, to store the IDs to be sent, and it supports defining custom send IDs


#include <SPI.h>

#define CAN_2515
// #define CAN_2518FD

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif


#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif    

// Define the list of standard CAN IDs to receive
const unsigned long STANDARD_CAN_IDS[] = {0x123, 0x456};
const int STANDARD_CAN_ID_COUNT = sizeof(STANDARD_CAN_IDS) / sizeof(STANDARD_CAN_IDS[0]);

// Define the list of extended CAN IDs to receive
const unsigned long EXTENDED_CAN_IDS[] = {0x1234567, 0x89ABCDE};
const int EXTENDED_CAN_ID_COUNT = sizeof(EXTENDED_CAN_IDS) / sizeof(EXTENDED_CAN_IDS[0]);

void setup() 
{
  SERIAL_PORT_MONITOR.begin(115200);
  while(!Serial){};

  while (CAN_OK != CAN.begin(CAN_500KBPS)) 
  {             // init can bus : baudrate = 500k
    SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
    delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
}

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void loop() 
{
  // send data: id = 0x00, standrad frame, data len = 8, stmp: data buf
  stmp[7] = stmp[7] + 1;
  if (stmp[7] == 100) 
  {
    stmp[7] = 0;
    stmp[6] = stmp[6] + 1;

    if (stmp[6] == 100) 
    {
      stmp[6] = 0;
      stmp[5] = stmp[5] + 1;
    }
  }

  for (int i = 0; i < STANDARD_CAN_ID_COUNT; i++) 
  {
    CAN.sendMsgBuf(STANDARD_CAN_IDS[i], 0, 8, stmp);//using ID List ID 
    delay(100);    
  }

  for (int i = 0; i < EXTENDED_CAN_ID_COUNT; i++) 
  {
    CAN.sendMsgBuf(EXTENDED_CAN_IDS[i], 1, 8, stmp);//using ID List ID 
    delay(100);    
  }
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/