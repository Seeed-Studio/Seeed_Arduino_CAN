// This is an example of receiving both standard frames and extended frames simultaneously. 
// I have used two arrays, STANDARD_CAN_IDS and EXTENDED_CAN_IDS, to receive CAN messages with specific IDs. 
// You need to set filters and masks for these IDs, 
// and make sure to refer to and coordinate with the example for sending extended frames


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

  while (CAN_OK != CAN.begin(CAN_250KBPS))
  {             // init can bus : baudrate = 250k
    SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
    delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");

  // Configure the standard frame filters
  for (int i = 0; i < STANDARD_CAN_ID_COUNT; i++) 
  {
    CAN.init_Mask(i, 0, 0x7FF); // Check all 11 bits of the standard frame
    CAN.init_Filt(i, 0, STANDARD_CAN_IDS[i]);
  }

  // Configure the extended frame filters
  for (int i = 0; i < EXTENDED_CAN_ID_COUNT; i++) 
  {
    CAN.init_Mask(i + STANDARD_CAN_ID_COUNT, 1, 0x1FFFFFFF); // Check all 29 bits of the extended frame
    CAN.init_Filt(i + STANDARD_CAN_ID_COUNT, 1, EXTENDED_CAN_IDS[i]);
  }
}

void loop()
{
  unsigned char len = 0;
  unsigned char buf[8];
  if (CAN_MSGAVAIL == CAN.checkReceive())
  {         // check if data coming
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

    unsigned long canId = CAN.getCanId();

    // Check if it is a standard CAN ID
    for (int i = 0; i < STANDARD_CAN_ID_COUNT; i++) 
    {
        if (canId == STANDARD_CAN_IDS[i]) 
        {
            SERIAL_PORT_MONITOR.println("-----------------------------");
            SERIAL_PORT_MONITOR.print("Get standard data from ID: 0x");
            SERIAL_PORT_MONITOR.println(canId, HEX);

            for (int j = 0; j < len; j++) 
            { // print the data
                SERIAL_PORT_MONITOR.print(buf[j], HEX);
                SERIAL_PORT_MONITOR.print("\t");
            }
            SERIAL_PORT_MONITOR.println();
            break;
        }
    }

    // Check if it is an extended CAN ID
    for (int i = 0; i < EXTENDED_CAN_ID_COUNT; i++) 
    {
        if (canId == EXTENDED_CAN_IDS[i]) 
        {
            SERIAL_PORT_MONITOR.println("-----------------------------");
            SERIAL_PORT_MONITOR.print("Get extended data from ID: 0x");
            SERIAL_PORT_MONITOR.println(canId, HEX);

            for (int j = 0; j < len; j++) 
            { // print the data
                SERIAL_PORT_MONITOR.print(buf[j], HEX);
                SERIAL_PORT_MONITOR.print("\t");
            }
            SERIAL_PORT_MONITOR.println();
            break;
        }
    }
  }
}