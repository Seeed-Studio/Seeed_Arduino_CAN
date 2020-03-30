// demo: CAN Sleep Example - send
// by Kai, based on the send example by loovee and the additions from Zak Kemble (https://github.com/coryjfowler/MCP_CAN_lib/pull/10/files)
//
// See receive_sleep example for additional notes.

#include <mcp_can.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

// To use the sleep mode of the transceiver (MCP2551), it's Rs pin must be connected to either the MCP2515 or
// any free Arduino output.
#define RS_TO_MCP2515 true                                   // Set this to false if Rs is connected to your Arduino
#define RS_OUTPUT MCP_RX0BF                                  // RX0BF is a pin of the MCP2515. You can also define an Arduino pin here

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

// Watchdog interrupt, used to wake the MCU periodically.
ISR(WDT_vect) {
    wdt_disable();  // disable watchdog
}

void sleepMCU()
// Sleep the MCU for one second.
// See http://www.gammon.com.au/power for details.
// (You can make your life a lot easyer by using one of the numeral low power libraries for Arduino.)
{
    // disable ADC
    ADCSRA = 0;

    // clear various "reset" flags
    MCUSR = 0;
    // allow changes, disable reset
    WDTCSR = bit(WDCE) | bit(WDE);
    // set interrupt mode and an interval
    WDTCSR = bit(WDIE) | bit(WDP2) | bit(WDP1);       // set WDIE, and 1 second delay
    wdt_reset();  // pat the dog

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    noInterrupts();            // timed sequence follows
    sleep_enable();

    // turn off brown-out enable in software
    MCUCR = bit(BODS) | bit(BODSE);
    MCUCR = bit(BODS);
    interrupts();              // guarantees next instruction executed
    sleep_cpu();

    // cancel sleep as a precaution
    sleep_disable();
}

void setup() {
    SERIAL.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_16MHz)) {    // init can bus : baudrate = 500k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");

    CAN.setSleepWakeup(0);                                  // the MCP2515 will NOT wake up on incoming messages,
    // making it a 'send only' node

    // Pull the Rs pin of the MCP2551 transceiver low to enable it:
    if (RS_TO_MCP2515) {
        CAN.mcpPinMode(MCP_RX0BF, MCP_PIN_OUT);
        CAN.mcpDigitalWrite(RS_OUTPUT, LOW);
    } else {
        pinMode(RS_OUTPUT, OUTPUT);
        digitalWrite(RS_OUTPUT, LOW);
    }
}

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void loop() {
    SERIAL.println("Sending message");

    CAN.sendMsgBuf(0x00, 0, 0, NULL);     // Send empty wakeup message

    delay(100);                           // give the receiving node some time to wake up


    // send data:  id = 0x00, standard frame, data len = 8, stmp: data buf
    stmp[7] = stmp[7] + 1;
    if (stmp[7] == 100) {
        stmp[7] = 0;
        stmp[6] = stmp[6] + 1;

        if (stmp[6] == 100) {
            stmp[6] = 0;
            stmp[5] = stmp[6] + 1;
        }
    }

    CAN.sendMsgBuf(0x00, 0, 8, stmp);


    // sleep
    SERIAL.println("Sleep");
    SERIAL.flush();

    // Put MCP2515 into sleep mode
    CAN.sleep();

    // Put the transceiver into standby (by pulling Rs high):
    if (RS_TO_MCP2515) {
        CAN.mcpDigitalWrite(RS_OUTPUT, HIGH);
    } else {
        digitalWrite(RS_OUTPUT, HIGH);
    }

    // Put the MCU to sleep for one second
    sleepMCU();

    // wake MCP2515 and transceiver
    CAN.wake();
    if (RS_TO_MCP2515) {
        CAN.mcpDigitalWrite(RS_OUTPUT, LOW);
    } else {
        digitalWrite(RS_OUTPUT, LOW);
    }
}

// END FILE
