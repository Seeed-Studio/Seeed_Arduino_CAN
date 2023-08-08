/*****************************************************************************************
* This is implementation of CAN BUS ASCII protocol based on LAWICEL v1.3 serial protocol
*  of CAN232/CANUSB device (http://www.can232.com/docs/can232_v3.pdf)
*
* Made for Arduino with Seeduino/ElecFreaks CAN BUS Shield based on MCP2515
*
* Copyright (C) 2015 Anton Viktorov <latonita@yandex.ru>
*                                    https://github.com/latonita/can-ascii
*
* This library is free software. You may use/redistribute it under The MIT License terms. 
*
*****************************************************************************************/


#ifndef _CAN_SERIAL_H_
#define _CAN_SERIAL_H_

//#if defined(ARDUINO) && ARDUINO >= 100
//    #include "arduino.h"
//#else
//    #include "WProgram.h"
//#endif

#include "mcp_can.h"


#define LW232_LAWICEL_VERSION_STR     "V1013"
#define LW232_LAWICEL_SERIAL_NUM      "NA123"
#define LW232_CAN_BUS_SHIELD_CS_PIN   10


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Commands not supported/not implemented:
//     s, W, M, m, U
//      
//   Commands modified:
//     S - supports not declared 83.3 rate straight away (S9)     
//         refer to https://github.com/latonita/CAN_BUS_Shield fork with 83.3 support, easy to add.
//     F - returns MCP2515 error flags
//     Z - extra Z2 option enables 4 byte timestamp vs standard 2 byte (60000ms max)
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   
//                          CODE   SUPPORTED   SYNTAX               DESCRIPTION     
//
#define LW232_CMD_SETUP     'S' //   YES+      Sn[CR]               Setup with standard CAN bit-rates where n is 0-8.
                                //                                  S0 10Kbit          S4 125Kbit         S8 1Mbit
                                //                                  S1 20Kbit          S5 250Kbit         S9 83.3Kbit
                                //                                  S2 50Kbit          S6 500Kbit
                                //                                  S3 100Kbit         S7 800Kbit
#define LW232_CMD_SETUP_BTR 's' //    -        sxxyy[CR]            Setup with BTR0/BTR1 CAN bit-rates where xx and yy is a hex value.
#define LW232_CMD_OPEN      'O' //   YES       O[CR]                Open the CAN channel in normal mode (sending & receiving).
#define LW232_CMD_LISTEN    'L' //   YES       L[CR]                Open the CAN channel in listen only mode (receiving).
#define LW232_CMD_CLOSE     'C' //   YES       C[CR]                Close the CAN channel.
#define LW232_CMD_TX11      't' //   YES       tiiildd...[CR]       Transmit a standard (11bit) CAN frame.
#define LW232_CMD_TX29      'T' //   YES       Tiiiiiiiildd...[CR]  Transmit an extended (29bit) CAN frame
#define LW232_CMD_RTR11     'r' //   YES       riiil[CR]            Transmit an standard RTR (11bit) CAN frame.
#define LW232_CMD_RTR29     'R' //   YES       Riiiiiiiil[CR]       Transmit an extended RTR (29bit) CAN frame.
#define LW232_CMD_POLL_ONE  'P' //   YES       P[CR]                Poll incomming FIFO for CAN frames (single poll)
#define LW232_CMD_POLL_MANY 'A' //   YES       A[CR]                Polls incomming FIFO for CAN frames (all pending frames)
#define LW232_CMD_FLAGS     'F' //   YES+      F[CR]                Read Status Flags.
#define LW232_CMD_AUTOPOLL  'X' //   YES       Xn[CR]               Sets Auto Poll/Send ON/OFF for received frames.
#define LW232_CMD_FILTER    'W' //    -        Wn[CR]               Filter mode setting. By default CAN232 works in dual filter mode (0) and is backwards compatible with previous CAN232 versions.
#define LW232_CMD_ACC_CODE  'M' //    -        Mxxxxxxxx[CR]        Sets Acceptance Code Register (ACn Register of SJA1000). // we use MCP2515, not supported
#define LW232_CMD_ACC_MASK  'm' //    -        mxxxxxxxx[CR]        Sets Acceptance Mask Register (AMn Register of SJA1000). // we use MCP2515, not supported
#define LW232_CMD_UART      'U' //   YES       Un[CR]               Setup UART with a new baud rate where n is 0-6.
#define LW232_CMD_VERSION1  'V' //   YES       v[CR]                Get Version number of both CAN232 hardware and software
#define LW232_CMD_VERSION2  'v' //   YES       V[CR]                Get Version number of both CAN232 hardware and software
#define LW232_CMD_SERIAL    'N' //   YES       N[CR]                Get Serial number of the CAN232.
#define LW232_CMD_TIMESTAMP 'Z' //   YES+      Zn[CR]               Sets Time Stamp ON/OFF for received frames only. EXTENSION to LAWICEL: Z2 - millis() timestamp w/o standard 60000ms cycle
#define LW232_CMD_AUTOSTART 'Q' //   YES  todo     Qn[CR]               Auto Startup feature (from power on). 

#define LOW_BYTE(x)     ((unsigned char)((x)&0xFF))
#define HIGH_BYTE(x)    ((unsigned char)(((x)>>8)&0xFF))
#define LOW_WORD(x)     ((unsigned short)((x)&0xFFFF))
#define HIGH_WORD(x)    ((unsigned short)(((x)>>16)&0xFFFF))

#ifndef INT32U
#define INT32U unsigned long
#endif

#ifndef INT16U
#define INT16U word
#endif

#ifndef INT8U
#define INT8U byte
#endif

#define LW232_OK                      0x00
#define LW232_OK_SMALL                0x01
#define LW232_OK_BIG                  0x02
#define LW232_ERR                     0x03
#define LW232_ERR_NOT_IMPLEMENTED     0x04
#define LW232_ERR_UNKNOWN_CMD         0x05

#define LW232_FILTER_SKIP			  0x00
#define LW232_FILTER_PROCESS	      0x01

//#define LW232_IS_OK(x) ((x)==LW232_OK ||(x)==LW232_OK_NEW ? TRUE : FALSE)

#define LW232_CR    '\r'
#define LW232_ALL   'A'
#define LW232_FLAG  'F'
#define LW232_TR11  't'
#define LW232_TR29  'T'

#define LW232_RET_ASCII_OK             0x0D
#define LW232_RET_ASCII_ERROR          0x07
#define LW232_RET_ASCII_OK_SMALL       'z'
#define LW232_RET_ASCII_OK_BIG         'Z'

#define LW232_STATUS_CAN_CLOSED        0x00
#define LW232_STATUS_CAN_OPEN_NORMAL   0x01
#define LW232_STATUS_CAN_OPEN_LISTEN   0x01

#define LW232_FRAME_MAX_LENGTH         0x08
#define LW232_FRAME_MAX_SIZE           (sizeof("Tiiiiiiiildddddddddddddddd\r")+1)

#define LW232_INPUT_STRING_BUFFER_SIZE 200

#define LW232_OFF                      '0'
#define LW232_ON_ONE                   '1'
#define LW232_ON_TWO                   '2'

#define LW232_AUTOPOLL_OFF             0x00
#define LW232_AUTOPOLL_ON              0x01

#define LW232_AUTOSTART_OFF            0x00
#define LW232_AUTOSTART_ON_NORMAL      0x01
#define LW232_AUTOSTART_ON_LISTEN      0x02

#define LW232_TIMESTAMP_OFF            0x00
#define LW232_TIMESTAMP_ON_NORMAL      0x01
#define LW232_TIMESTAMP_ON_EXTENDED    0x02

#define LW232_OFFSET_STD_PKT_LEN       0x04
#define LW232_OFFSET_STD_PKT_DATA      0x05
#define LW232_OFFSET_EXT_PKT_LEN       0x09
#define LW232_OFFSET_EXT_PKT_DATA      0x0A


#define LW232_DEFAULT_BAUD_RATE        115200
#define LW232_DEFAULT_CAN_RATE         CAN_500KBPS
#define LW232_DEFAULT_CLOCK_FREQ       MCP_16MHz

#define LW232_CAN_BAUD_NUM             0x0a
#define LW232_UART_BAUD_NUM            0x07



const INT32U lw232SerialBaudRates[] //PROGMEM
= { 230400, 115200, 57600, 38400, 19200, 9600, 2400 };

const INT32U lw232CanBaudRates[] //PROGMEM
 = { CAN_10KBPS, CAN_20KBPS, CAN_50KBPS, CAN_100KBPS, CAN_125KBPS, CAN_250KBPS, CAN_500KBPS, CAN_800KBPS, CAN_1000KBPS, CAN_83K3BPS };

class Can232
{
public:
    static void init(INT8U defaultCanSpeed = LW232_DEFAULT_CAN_RATE, const INT8U clock = LW232_DEFAULT_CLOCK_FREQ);
    static void attach(MCP_CAN *can);
    static void setFilter(INT8U (*userFunc)(INT32U));
    static void loop();
    static void serialEvent();

private:
    static MCP_CAN *MCP_OBJECT;
    static Can232* _instance;
    static Can232* instance();

    void initFunc();
    void setFilterFunc(INT8U (*userFunc)(INT32U));
    void loopFunc();
    void serialEventFunc();

    INT8U (*userAddressFilterFunc)(INT32U addr) = 0;

    /*
    MCP_CAN lw232CAN = MCP_CAN(LW232_CAN_BUS_SHIELD_CS_PIN);
    */

    INT8U lw232CanSpeedSelection = CAN_83K3BPS;
    INT8U lw232McpModuleClock = MCP_16MHz;
    INT8U lw232CanChannelMode = LW232_STATUS_CAN_CLOSED;
    INT8U lw232LastErr = LW232_OK;

    INT8U lw232AutoStart = LW232_AUTOSTART_OFF;
    INT8U lw232AutoPoll  = LW232_AUTOPOLL_OFF;
    INT8U lw232TimeStamp = LW232_TIMESTAMP_OFF;

    INT32U lw232CanId = 0;

    INT8U lw232Buffer[8];
    INT8U lw232PacketLen = 0;

    INT8U lw232Message[LW232_FRAME_MAX_SIZE];

    String inputString = "";         // a string to hold incoming data
    boolean stringComplete = false;  // whether the string is complete

    INT8U parseAndRunCommand();
    INT8U exec();

    INT8U checkReceive();
    INT8U readMsgBufID(INT32U *ID, INT8U *len, INT8U buf[]);
    INT8U receiveSingleFrame();
    INT8U isExtendedFrame();
    INT8U checkPassFilter(INT32U addr);
    INT8U openCanBus();
    
    INT8U sendMsgBuf(INT32U id, INT8U ext, INT8U rtr, INT8U len, INT8U *buf);
    
    void  parseCanStdId();
    void  parseCanExtId();
};

class HexHelper {
public:
    static void printFullByte(INT8U b);
    static void printNibble(INT8U b);

    static INT8U parseNibble(INT8U hex);
    static INT8U parseFullByte(INT8U H, INT8U L);
    static INT8U parseNibbleWithLimit(INT8U hex, INT8U limit);
};

class Can232Fake : Can232 {

};


#endif
