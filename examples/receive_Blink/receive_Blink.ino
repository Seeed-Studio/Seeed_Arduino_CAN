// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp2518fd_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = BCM8;
mcp2518fd* controller;

const int LED        = LED_BUILTIN;
boolean ledON        = 1;
                                  // Set CS pin
void setup() {
    SERIAL.begin(115200);
    while(!Serial){};
    controller = new mcp2518fd();
    controller->mcp_canbus(SPI_CS_PIN);
    pinMode(LED, OUTPUT);

    while (CAN_OK != controller->begin((byte)CAN_500K_1M)) {            // init can bus : baudrate = 500k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
}


void loop() {
    unsigned char len = 0;
    unsigned char buf[8];

    if (1 == controller->checkReceive()) {         // check if data coming
        controller->readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = controller->getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.println("get data from ID: 0x");
        SERIAL.println(canId, HEX);

        for (int i = 0; i < len; i++) { // print the data
            SERIAL.print(buf[i]);
            SERIAL.print("\t");
            if (ledON && i == 0) {

                digitalWrite(LED, buf[i]);
                ledON = 0;
                delay(500);
            } else if ((!(ledON)) && i == 4) {

                digitalWrite(LED, buf[i]);
                ledON = 1;
            }
        }
        SERIAL.println();
    }
}

//END FILE
