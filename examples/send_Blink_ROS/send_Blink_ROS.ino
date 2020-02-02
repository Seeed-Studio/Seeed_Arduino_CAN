// demo: CAN-BUS Shield, send data
// Hardware: TWO ArduinoMega with CAN-SHIELDS
// Run this send_Blink_ROS.ino file on one of the Arduinos
// Run receive_Blink on the other
// ***ROS commands to be followe***//
// roscore
// rosrun roial_python serial_node.py /dev/ttyACM1 _baud:=57600
// rostopic pub toggle_led std_msgs/Empty -r 100
// Jaghvi: jaghvim@andrew.cmu.edu


#include <mcp_can.h>
#include <SPI.h>

//ROS
#include <ros.h>
#include <std_msgs/Empty.h>

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

ros::NodeHandle  nh;

const int SPI_CS_PIN = 9;
const int ledHIGH = 1;
const int ledLOW = 0;
unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void messageCb(const std_msgs::Empty& toggle_msg) {
    //digitalWrite(13, HIGH-digitalRead(13));   // blink the led
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x70, 0, 8, stmp);
    delay(1000);                       // send data per 100ms
}

ros::Subscriber<std_msgs::Empty> sub("toggle_led", &messageCb);

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10



void setup() {
    SERIAL.begin(115200);
    nh.initNode();
    nh.subscribe(sub);

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {            // init can bus : baudrate = 500k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
}


void loop() {

    nh.spinOnce();
    delay(1);
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
