/************************************************************************************************* 
  OBD-II_PIDs TEST CODE
  LOOVEE @ JUN24, 2017
  
  Query
  send id: 0x7df
      dta: 0x02, 0x01, PID_CODE, 0, 0, 0, 0, 0

  Response
  From id: 0x7E9 or 0x7EA or 0x7EB
      dta: len, 0x41, PID_CODE, byte0, byte1(option), byte2(option), byte3(option), byte4(option)
      
  https://en.wikipedia.org/wiki/OBD-II_PIDs
  
  Input a PID, then you will get reponse from vehicle, the input should be end with '\n'
***************************************************************************************************/
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

#define PID_ENGIN_PRM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05

#define CAN_ID_PID          0x7DF

unsigned char PID_INPUT;
unsigned char getPid    = 0;

void set_mask_filt()
{
    /*
     * set mask, set both the mask to 0x3ff
     */
    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, 0, 0x7E8);                 
    CAN.init_Filt(1, 0, 0x7E8);

    CAN.init_Filt(2, 0, 0x7E8);
    CAN.init_Filt(3, 0, 0x7E8);
    CAN.init_Filt(4, 0, 0x7E8); 
    CAN.init_Filt(5, 0, 0x7E8);
}

void sendPid(unsigned char __pid)
{
    unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};
    SERIAL.print("SEND PID: 0x");
    SERIAL.println(__pid, HEX);
    CAN.sendMsgBuf(CAN_ID_PID, 0, 8, tmp);
}

void setup()
{
    SERIAL.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))    // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
    set_mask_filt();
}


void loop()
{
    taskCanRecv();
    taskDbg();
    
    if(getPid)          // GET A PID
    {
        getPid = 0;
        sendPid(PID_INPUT);
        PID_INPUT = 0;
    }
}

void taskCanRecv()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())                   // check if get data
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        SERIAL.println("\r\n------------------------------------------------------------------");
        SERIAL.print("Get Data From id: 0x");
        SERIAL.println(CAN.getCanId(), HEX);
        for(int i = 0; i<len; i++)    // print the data
        {
            SERIAL.print("0x");
            SERIAL.print(buf[i], HEX);
            SERIAL.print("\t");
        }
        SERIAL.println();
    }
}

void taskDbg()
{
    while(SERIAL.available())
    {
        char c = SERIAL.read();
        
        if(c>='0' && c<='9')
        {
            PID_INPUT *= 0x10;
            PID_INPUT += c-'0';
            
        }
        else if(c>='A' && c<='F')
        {
            PID_INPUT *= 0x10;
            PID_INPUT += 10+c-'A';
        }
        else if(c>='a' && c<='f')
        {
            PID_INPUT *= 0x10;
            PID_INPUT += 10+c-'a';
        }
        else if(c == '\n')      // END
        {
            getPid = 1;
        }
    }
}
// END FILE