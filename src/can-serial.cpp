/*****************************************************************************************
* This is implementation of CAN BUS ASCII protocol based on LAWICEL v1.3 serial protocol
*  of CanSerial/CANUSB device (http://www.CanSerial.com/docs/CanSerial_v3.pdf)
*
* Made for Arduino with Seeduino/ElecFreaks CAN BUS Shield based on MCP2515
*
* Copyright (C) 2015 Anton Viktorov <latonita@yandex.ru>
*                                    https://github.com/latonita/can-ascii
*
* This library is free software. You may use/redistribute it under The MIT License terms. 
*
*****************************************************************************************/

#include <SPI.h>
#include "mcp_can.h"
#include "can-serial.h"

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#define dbg_begin(x) debug.begin(x)
#define dbg0(x)   debug.print(x)
#define dbg1(x)   debug.println(x)
#define dbg2(x,y) debug.print(x); debug.println(y)
#define dbgH(x)   debug.print(x,HEX)
#define DEBUG_RX_PIN 8
#define DEBUG_TX_PIN 9
#else
#define dbg_begin(x)
#define dbg0(x) 
#define dbg1(x) 
#define dbg2(x,y)
#define dbgH(x)
#endif

#ifdef LOGGING_ENABLED
    // software serial #2: TX = digital pin 8, RX = digital pin 9
    // on the Mega, use other pins instead, since 8 and 9 don't work on the Mega
  
      SoftwareSerial debug(DEBUG_RX_PIN, DEBUG_TX_PIN);
  //#define debug Serial
#endif

CanSerial* CanSerial::_instance = 0;

CanSerial* CanSerial::instance() {
    if (_instance == 0)
        _instance = new CanSerial();
    return _instance;
}

void CanSerial::init(INT8U defaultCanSpeed, const INT8U clock) {
    dbg_begin(LWUART_DEFAULT_BAUD_RATE); // logging through software serial 
    dbg1("CAN ASCII. Welcome to debug");

    instance()->LWUARTCanSpeedSelection = defaultCanSpeed;
    instance()->LWUARTMcpModuleClock = clock;
    instance()->initFunc();
}


void CanSerial::init(MCP_CAN *CAN) {
    LWUARTCAN = CAN;
}


void CanSerial::setFilter(INT8U (*userFunc)(INT32U)) {
    instance()->setFilterFunc(userFunc);
}

void CanSerial::loop() {
    instance()->loopFunc();
}

void CanSerial::serialEvent() {
    instance()->serialEventFunc();
}

void CanSerial::initFunc() {
    if (!inputString.reserve(LWUART_INPUT_STRING_BUFFER_SIZE)) {
        dbg0("inputString.reserve failed in initFunc. less optimal String work is expected");
    }
    // LWUARTAutoStart = true; //todo: read from eeprom
    // LWUARTAutoPoll = false; //todo: read from eeprom
    //  LWUARTTimeStamp = //read from eeprom
    //    LWUARTMessage[0] = 'Z';    LWUARTMessage[1] = '1'; exec();
    //if (LWUARTAutoStart) {
        inputString = "O\0x0D";
        stringComplete = true;
        loopFunc();
    //}
}

void CanSerial::setFilterFunc(INT8U (*userFunc)(INT32U)) {
    instance()->userAddressFilterFunc = userFunc;
}

void CanSerial::loopFunc() {
    if (stringComplete) {
        int len = inputString.length();
        if (len > 0 && len < LWUART_FRAME_MAX_SIZE) {
            strcpy((char*)LWUARTMessage, inputString.c_str());
            exec();
        }
        // clear the string:
        inputString = "";
        stringComplete = false;
    }
    if (LWUARTCanChannelMode != LWUART_STATUS_CAN_CLOSED) {
        int recv = 0;
        while (CAN_MSGAVAIL == checkReceive() && recv++<5) {
            dbg0('+');
            if (CAN_OK == receiveSingleFrame()) {
                Serial.write(LWUART_CR);
            }
        }
        Serial.flush();
    }
}
void CanSerial::serialEventFunc() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        if (inChar == LWUART_CR) {
            stringComplete = true;
        }
    }
}

INT8U CanSerial::exec() {
    dbg2("Command received:", inputString);
    LWUARTLastErr = parseAndRunCommand();
    switch (LWUARTLastErr) {
    case LWUART_OK:
        Serial.write(LWUART_RET_ASCII_OK);
        break;
    case LWUART_OK_SMALL:
        Serial.write(LWUART_RET_ASCII_OK_SMALL);
        Serial.write(LWUART_RET_ASCII_OK);
        break;
    case LWUART_OK_BIG:
        Serial.write(LWUART_RET_ASCII_OK_BIG);
        Serial.write(LWUART_RET_ASCII_OK);
        break;
    case LWUART_ERR_NOT_IMPLEMENTED:
        // Choose behavior: will it fail or not when not implemented command comes in. Some can monitors might be affected by this selection.
        Serial.write(LWUART_RET_ASCII_ERROR);
        //Serial.write(LWUART_RET_ASCII_OK); 
        break;
    default:
        Serial.write(LWUART_RET_ASCII_ERROR);
    }
    return 0;
}

INT8U CanSerial::parseAndRunCommand() {
    INT8U ret = LWUART_OK;
    INT8U idx = 0;
    INT8U err = 0;

    LWUARTLastErr = LWUART_OK;

    switch (LWUARTMessage[0]) {
        case LWUART_CMD_SETUP:
        // Sn[CR] Setup with standard CAN bit-rates where n is 0-9.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_CLOSED) {
            idx = HexHelper::parseNibbleWithLimit(LWUARTMessage[1], LWUART_CAN_BAUD_NUM);
			      LWUARTCanSpeedSelection = idx; //todo
        }
        else {
            ret = LWUART_ERR;
        }
        break;
        case LWUART_CMD_SETUP_BTR:
        // sxxyy[CR] Setup with BTR0/BTR1 CAN bit-rates where xx and yy is a hex value.
        ret = LWUART_ERR; break;
        case LWUART_CMD_OPEN:
        // O[CR] Open the CAN channel in normal mode (sending & receiving).
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_CLOSED) {
            ret = openCanBus();
            if (ret == LWUART_OK) {
              LWUARTCanChannelMode = LWUART_STATUS_CAN_OPEN_NORMAL;
            }
        }
        else {
            ret = LWUART_ERR;
        }
        break;
        case LWUART_CMD_LISTEN:
        // L[CR] Open the CAN channel in listen only mode (receiving).
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_CLOSED) {
            ret = openCanBus();
            if (ret == LWUART_OK) {
              LWUARTCanChannelMode = LWUART_STATUS_CAN_OPEN_LISTEN;
            }
        }
        else {
            ret = LWUART_ERR;
        }
        break;
        case LWUART_CMD_CLOSE:
        // C[CR] Close the CAN channel.
        if (LWUARTCanChannelMode != LWUART_STATUS_CAN_CLOSED) {
            LWUARTCanChannelMode = LWUART_STATUS_CAN_CLOSED;
        }
        else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_TX11:
        // tiiildd...[CR] Transmit a standard (11bit) CAN frame.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_OPEN_NORMAL) {
            parseCanStdId();
            LWUARTPacketLen = HexHelper::parseNibbleWithLimit(LWUARTMessage[LWUART_OFFSET_STD_PKT_LEN], LWUART_FRAME_MAX_LENGTH + 1);
            for (; idx < LWUARTPacketLen; idx++) {
                LWUARTBuffer[idx] = HexHelper::parseFullByte(LWUARTMessage[LWUART_OFFSET_STD_PKT_DATA + idx * 2], LWUARTMessage[LWUART_OFFSET_STD_PKT_DATA + idx * 2 + 1]);
            }
            INT8U mcpErr = sendMsgBuf(LWUARTCanId, 0, 0, LWUARTPacketLen, LWUARTBuffer);
            if (mcpErr != CAN_OK) {
                ret = LWUART_ERR;
            } else if (LWUARTAutoPoll) {
                ret = LWUART_OK_SMALL;
            } 
        }
        else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_TX29:
        // Tiiiiiiiildd...[CR] Transmit an extended (29bit) CAN frame
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_OPEN_NORMAL) {
            parseCanExtId();
            LWUARTPacketLen = HexHelper::parseNibbleWithLimit(LWUARTMessage[LWUART_OFFSET_EXT_PKT_LEN], LWUART_FRAME_MAX_LENGTH + 1);
            for (; idx < LWUARTPacketLen; idx++) {
                LWUARTBuffer[idx] = HexHelper::parseFullByte(LWUARTMessage[LWUART_OFFSET_EXT_PKT_DATA + idx * 2], LWUARTMessage[LWUART_OFFSET_EXT_PKT_DATA + idx * 2 + 1]);
            }
            if (CAN_OK != sendMsgBuf(LWUARTCanId, 1, 0, LWUARTPacketLen, LWUARTBuffer)) {
                ret = LWUART_ERR;
            } else if (LWUARTAutoPoll) {
                ret = LWUART_OK_BIG;
            } else {
              ret = LWUART_OK;
            }
        }
        break;
    case LWUART_CMD_RTR11:
        // riiil[CR] Transmit an standard RTR (11bit) CAN frame.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_OPEN_NORMAL) {
            parseCanStdId();
            LWUARTPacketLen = HexHelper::parseNibbleWithLimit(LWUARTMessage[LWUART_OFFSET_STD_PKT_LEN], LWUART_FRAME_MAX_LENGTH + 1);
            if (CAN_OK != sendMsgBuf(LWUARTCanId, 0, 1, LWUARTPacketLen, LWUARTBuffer)) {
                ret = LWUART_ERR;
            }
            else if (LWUARTAutoPoll) {
                ret = LWUART_OK_SMALL;
            }
        }
        else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_RTR29:
        // Riiiiiiiil[CR] Transmit an extended RTR (29bit) CAN frame.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_OPEN_NORMAL) {
            parseCanExtId();
            LWUARTPacketLen = HexHelper::parseNibbleWithLimit(LWUARTMessage[LWUART_OFFSET_EXT_PKT_LEN], LWUART_FRAME_MAX_LENGTH + 1);
            if (CAN_OK != sendMsgBuf(LWUARTCanId, 1, 1, LWUARTPacketLen, LWUARTBuffer)) {
                ret = LWUART_ERR;
            }
            else if (LWUARTAutoPoll) {
                ret = LWUART_OK_SMALL; // not a typo. strangely CanSerial_v3.pdf tells to return "z[CR]", not "Z[CR]" as in 29bit. todo: check if it is error in pdf???
            }
        } else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_POLL_ONE:
        // P[CR] Poll incomming FIFO for CAN frames (single poll)
        if (LWUARTCanChannelMode != LWUART_STATUS_CAN_CLOSED && LWUARTAutoPoll == LWUART_AUTOPOLL_OFF) {
            if (CAN_MSGAVAIL == checkReceive()) {
                ret = receiveSingleFrame();
            }
        } else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_POLL_MANY:
        // A[CR] Polls incomming FIFO for CAN frames (all pending frames)
        if (LWUARTCanChannelMode != LWUART_STATUS_CAN_CLOSED && LWUARTAutoPoll == LWUART_AUTOPOLL_OFF) {
            while (CAN_MSGAVAIL == checkReceive()) {
                ret = ret ^ receiveSingleFrame();
                if (ret != CAN_OK)
                    break;
                Serial.write(LWUART_CR);
            }
            if (ret == CAN_OK)
                Serial.print(LWUART_ALL);
        } else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_FLAGS:
        // F[CR] Read Status Flags.
        // LAWICEL CanSerial and CANUSB have some specific errors which differ from MCP2515/MCP2551 errors. We just return MCP2515 error.
        Serial.print(LWUART_FLAG);
        if (LWUARTCAN->checkError() == CAN_OK) 
            err = 0;
        HexHelper::printFullByte(err);
        break;
    case LWUART_CMD_AUTOPOLL:
        // Xn[CR] Sets Auto Poll/Send ON/OFF for received frames.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_CLOSED) {
            LWUARTAutoPoll = (LWUARTMessage[1] == LWUART_ON_ONE) ? LWUART_AUTOPOLL_ON : LWUART_AUTOPOLL_OFF;
            //todo: save to eeprom
        } else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_FILTER:
        // Wn[CR] Filter mode setting. By default CanSerial works in dual filter mode (0) and is backwards compatible with previous CanSerial versions.
        ret = LWUART_ERR_NOT_IMPLEMENTED; break;
    case LWUART_CMD_ACC_CODE:
        // Mxxxxxxxx[CR] Sets Acceptance Code Register (ACn Register of SJA1000). // we use MCP2515,
        ret = LWUART_ERR_NOT_IMPLEMENTED; break;
    case LWUART_CMD_ACC_MASK:
        // mxxxxxxxx[CR] Sets Acceptance Mask Register (AMn Register of SJA1000).
        ret = LWUART_ERR_NOT_IMPLEMENTED; break;
    case LWUART_CMD_UART:
        // Un[CR] Setup UART with a new baud rate where n is 0-6.
        idx = HexHelper::parseNibbleWithLimit(LWUARTMessage[1], LWUART_UART_BAUD_NUM);
        Serial.begin(LWUARTSerialBaudRates[idx]);
        break;
    case LWUART_CMD_VERSION1:
    case LWUART_CMD_VERSION2:
        // V[CR] Get Version number of both CanSerial hardware and software
        Serial.print(LWUART_LAWICEL_VERSION_STR);
        break;
    case LWUART_CMD_SERIAL:
        // N[CR] Get Serial number of the CanSerial.
        Serial.print(LWUART_LAWICEL_SERIAL_NUM);
        break;
    case LWUART_CMD_TIMESTAMP:
        // Zn[CR] Sets Time Stamp ON/OFF for received frames only. Z0 - OFF, Z1 - Lawicel's timestamp 2 bytes, Z2 - arduino timestamp 4 bytes.
        if (LWUARTCanChannelMode == LWUART_STATUS_CAN_CLOSED) {
            // LWUARTTimeStamp = (LWUARTMessage[1] == LWUART_ON_ONE); 
            if (LWUARTMessage[1] == LWUART_ON_ONE) {
                LWUARTTimeStamp = LWUART_TIMESTAMP_ON_NORMAL;
            }
            else if (LWUARTMessage[1] == LWUART_ON_TWO) {
                LWUARTTimeStamp = LWUART_TIMESTAMP_ON_EXTENDED;
            }
            else {
                LWUARTTimeStamp = LWUART_TIMESTAMP_OFF;
            }
        }
        else {
            ret = LWUART_ERR;
        }
        break;
    case LWUART_CMD_AUTOSTART:
        // Qn[CR] Auto Startup feature (from power on).
        if (LWUARTCanChannelMode != LWUART_STATUS_CAN_CLOSED) {
            if (LWUARTMessage[1] == LWUART_ON_ONE) {
                LWUARTAutoStart = LWUART_AUTOSTART_ON_NORMAL;
            }
            else if (LWUARTMessage[1] == LWUART_ON_TWO) {
                LWUARTAutoStart = LWUART_AUTOSTART_ON_LISTEN;
            }
            else {
                LWUARTAutoStart = LWUART_AUTOSTART_OFF;
            }
            //todo: save to eeprom
        }
        else {
            ret = LWUART_ERR;
        }
        break;
    default:
        ret = LWUART_ERR_UNKNOWN_CMD;
    }

    return ret;
}

INT8U CanSerial::checkReceive() {
#ifndef _MCP_FAKE_MODE_
    return LWUARTCAN->checkReceive();
#else
    return CAN_MSGAVAIL;
#endif
}

INT8U CanSerial::readMsgBufID(INT32U *ID, INT8U *len, INT8U buf[]) {
#ifndef _MCP_FAKE_MODE_
    return LWUARTCAN->readMsgBufID(ID, len, buf);
#else
    *ID = random(0x100, 0x110);
    *len = 4;
    buf[0] = random(0x01, 0x10);
    buf[1] = random(0xa1, 0xf0);
    buf[2] = 0x00;
    buf[3] = 0x00;
    return CAN_OK;
#endif
}



INT8U CanSerial::receiveSingleFrame() {
    INT8U ret = LWUART_OK;
    INT8U idx = 0;
    if (CAN_OK == readMsgBufID(&LWUARTCanId, &LWUARTPacketLen, LWUARTBuffer)) {
        if (LWUARTCanId > 0x1FFFFFFF) {
            ret = LWUART_ERR; // address if totally wrong
        }
        else if (checkPassFilter(LWUARTCanId)) {// do we want to skip some addresses?
            if (isExtendedFrame()) {
                Serial.print(LWUART_TR29);
                HexHelper::printFullByte(HIGH_BYTE(HIGH_WORD(LWUARTCanId)));
                HexHelper::printFullByte(LOW_BYTE(HIGH_WORD(LWUARTCanId)));
                HexHelper::printFullByte(HIGH_BYTE(LOW_WORD(LWUARTCanId)));
                HexHelper::printFullByte(LOW_BYTE(LOW_WORD(LWUARTCanId)));
            }
            else {
                Serial.print(LWUART_TR11);
                HexHelper::printNibble(HIGH_BYTE(LOW_WORD(LWUARTCanId)));
                HexHelper::printFullByte(LOW_BYTE(LOW_WORD(LWUARTCanId)));
            }
            //write data len
            HexHelper::printNibble(LWUARTPacketLen);
            //write data
            for (idx = 0; idx < LWUARTPacketLen; idx++) {
                HexHelper::printFullByte(LWUARTBuffer[idx]);
            }
            //write timestamp if needed
            if (LWUARTTimeStamp != LWUART_TIMESTAMP_OFF) {
                INT32U time = millis();
                if (LWUARTTimeStamp == LWUART_TIMESTAMP_ON_NORMAL) { 
                    // standard LAWICEL protocol. two bytes.
                    time %= 60000;  
                } else {
                    // non standard protocol - 4 bytes timestamp
                    HexHelper::printFullByte(HIGH_BYTE(HIGH_WORD(time)));
                    HexHelper::printFullByte(LOW_BYTE(HIGH_WORD(time)));
                }
                HexHelper::printFullByte(HIGH_BYTE(LOW_WORD(time)));
                HexHelper::printFullByte(LOW_BYTE(LOW_WORD(time)));
            }
        }
    }
    else {
        ret = LWUART_ERR;
    }
    return ret;
}


INT8U CanSerial::isExtendedFrame() {
#ifndef _MCP_FAKE_MODE_
    return LWUARTCAN->isExtendedFrame();
#else
    return LWUARTCanId > 0x7FF ? 1 : 0; //simple hack for fake mode
#endif
}


INT8U CanSerial::checkPassFilter(INT32U addr) {
	if (userAddressFilterFunc == 0) 
		return LWUART_FILTER_PROCESS;
	
	return (*userAddressFilterFunc)(addr);
}

INT8U CanSerial::openCanBus() {
    INT8U ret = LWUART_OK;
#ifndef _MCP_FAKE_MODE_
    if (CAN_OK != LWUARTCAN->begin(LWUARTCanSpeedSelection, LWUARTMcpModuleClock))
        ret = LWUART_ERR;
#endif
    return ret;
}


INT8U CanSerial::sendMsgBuf(INT32U id, INT8U ext, INT8U rtr, INT8U len, INT8U *buf) {
#ifndef _MCP_FAKE_MODE_
    return LWUARTCAN->sendMsgBuf(id, ext, rtr, len, buf);
#else
    Serial.print("<sending:");
    Serial.print(id, HEX);
    Serial.print(',');
    if (ext) Serial.print('+');
    else Serial.print('-');
    if (rtr) Serial.print('+');
    else Serial.print('-');
    Serial.print(',');
    Serial.print(len, DEC);
    Serial.print(',');
    int i;
    for (i = 0; i < len; i++) printFullByte(buf[i]);
    return CAN_OK;
#endif
}



void CanSerial::parseCanStdId() {
    LWUARTCanId = (((INT32U)HexHelper::parseNibble(LWUARTMessage[1])) << 8)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[2])) << 4)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[3])));
    LWUARTCanId &= 0x7FF;
}

void CanSerial::parseCanExtId() {
    LWUARTCanId = (((INT32U)HexHelper::parseNibble(LWUARTMessage[1])) << 28)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[2])) << 24)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[3])) << 20)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[4])) << 16)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[5])) << 12)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[6])) << 8)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[7])) << 4)
        + (((INT32U)HexHelper::parseNibble(LWUARTMessage[8])));
    LWUARTCanId &= 0x1FFFFFFF;
}

void HexHelper::printFullByte(INT8U b) {
    if (b < 0x10) {
        Serial.print('0');
       // dbg0('0');
    }
    Serial.print(b, HEX);
    //dbgH(b);
}

void HexHelper::printNibble(INT8U b) {
    Serial.print(b & 0x0F, HEX);
    //dbgH(b & 0x0F);
}


INT8U HexHelper::parseNibble(INT8U hex) {
    INT8U ret = 0;
    if (hex >= '0' && hex <= '9') {
        ret = hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        ret = hex - 'a' + 10;
    } else if (hex >= 'A' && hex <= 'F') {
        ret = hex - 'A' + 10;
    } // else error, return 0
    return ret;
}

INT8U HexHelper::parseFullByte(INT8U H, INT8U L) {
    return (parseNibble(H) << 4) + parseNibble(L);
}

INT8U HexHelper::parseNibbleWithLimit(INT8U hex, INT8U limit) {
    INT8U ret = parseNibble(hex);
    if (ret < limit)
        return ret;
    else
        return 0;
}
