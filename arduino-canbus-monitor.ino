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

#include <SPI.h>
#include "mcp_can.h"
#include "can-232.h"
#include "SoftwareSerial.h"


INT8U myCustomAddressFilter(INT32U addr) {
    INT8U ret = LW232_FILTER_SKIP; //LW232_FILTER_PROCESS or LW232_FILTER_SKIP
    switch(addr) {
    //    case 0x01b: //VIN
    //    case 0x1C8:  //lights
    //    case 0x2C0: // pedals
        case 0x3d0: // sound vol, treb..
			ret = LW232_FILTER_PROCESS;
    //    case 0x000: // ?
    //    case 0x003: //shifter
    //    case 0x015: // dor open close affects this as well
    //    case 0x02c: // ???
    //        ret = 0;
    //        break;
    //    case 0x002:
    //    case 0x1a7: //fuel cons or some other
    //      ret = 1;
    //      break;
    //     default: 
    //       ret = 0;
    }

	return ret;
}

void setup() {
	Serial.begin(LW232_DEFAULT_BAUD_RATE);

    Can232::init();
	//Can232::setFilter(myCustomAddressFilter); // optional
}
void loop() {
    Can232::loop();
}

void serialEvent() {
    Can232::serialEvent();
}

