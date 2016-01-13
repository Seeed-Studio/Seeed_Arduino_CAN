Arduino MCP2515 CAN interface library
---------------------------------------------------------


<br>
CAN-BUS is a common industrial bus because of its long travel distance, medium communication speed and high reliability. It is commonly found on modern machine tools and as an automotive diagnostic bus. This CAN-BUS Shield adopts MCP2515 CAN Bus controller with SPI interface and MCP2551 CAN transceiver to give your Arduino/Seeeduino CAN-BUS capibility. With an OBD-II converter cable added on and the OBD-II library imported, you are ready to build an onboard diagnostic device or data logger.

- Implements CAN V2.0B at up to 1 Mb/s
- SPI Interface up to 10 MHz
- Standard (11 bit) and extended (29 bit) data and remote frames
- Two receive buffers with prioritized message storage


<br>
# Usage:



## 1. Initialization

To create connection with MCP2515 provide pin number where SPI CS is connected (10 by default) and mode

The available modes are listed as follows:
```C++
MCP_CAN::MODE_NORMAL,
MCP_CAN::MODE_LOOPBACK,
MCP_CAN::MODE_LISTENONLY
```
To work with network set baudrate.
The available baudrates are listed as follows:
```C++
enum CAN_SPEED {
    CAN_5KBPS,
    CAN_10KBPS,
    CAN_20KBPS,
    CAN_31K25BPS,
    CAN_33KBPS,
    CAN_40KBPS,
    CAN_50KBPS,
    CAN_80KBPS,
    CAN_83K3BPS,
    CAN_95KBPS,
    CAN_100KBPS,
    CAN_125KBPS,
    CAN_200KBPS,
    CAN_250KBPS,
    CAN_500KBPS,
    CAN_1000KBPS
};
```

<br>
Example of initialization
```C++
MCP_CAN mcp2515(10, MCP_CAN::MODE_LOOPBACK);
mcp2515.begin(CAN_125KBPS);
```
<br>

Note: To transfer data on high speed of CAN interface via UART dont forget to update UART baudrate as necessary.

##2. Frame data format

Library uses Linux-like structure to store can frames;
```C++
struct can_frame {
    uint32_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    uint8_t  can_dlc;
    uint8_t  data[8];
};
```
For additional information see [SocketCAN](https://www.kernel.org/doc/Documentation/networking/can.txt)

##3. Set Receive Mask and Filter

There are 2 receive mask registers and 5 filter registers on the controller chip that guarantee you get data from the target device. They are useful especially in a large network consisting of numerous nodes.

We provide two functions for you to utilize these mask and filter registers. They are:

```C++
init_Mask(unsigned char num, unsigned char ext, unsigned char ulData);
init_Filt(unsigned char num, unsigned char ext, unsigned char ulData);
```

**num** represents which register to use. You can fill 0 or 1 for mask and 0 to 5 for filter.

**ext** represents the status of the frame. 0 means it's a mask or filter for a standard frame. 1 means it's for a extended frame.

**ulData** represents the content of the mask of filter.



<br>
## 4. Send Data
```C++
MCP_CAN::ERROR sendMessage(const MCP_CAN::TXBn txbn, const struct can_frame *frame);
MCP_CAN::ERROR sendMessage(const struct can_frame *frame);
```

This is a function to send data onto the bus. 

For example, In the 'send' example, we have:

```C++
struct can_frame frame;
frame.can_id = 0x000;
frame.can_dlc = 4;
frame.data[0] = 0xFF;
frame.data[1] = 0xFF;
frame.data[2] = 0xFF;
frame.data[3] = 0xFF;

/* send out the message to the bus and 
tell other devices this is a standard frame from 0x00. */
mcp2515.sendMessage(&frame);
```

```C++
struct can_frame frame;
frame.can_id = 0x12345678 | CAN_EFF_MASK;
frame.can_dlc = 2;
frame.data[0] = 0xFF;
frame.data[1] = 0xFF;

/* send out the message to the bus using second TX buffer and 
tell other devices this is a extended frame from 0x12345678. */
mcp2515.sendMessage(MCP_CAN::TXB1, &frame);
```


<br>
## 5. Receive Data

The following function is used to receive data on the 'receive' node:

```C++
MCP_CAN::ERROR readMessage(const MCP_CAN::RXBn rxbn, struct can_frame *frame);
MCP_CAN::ERROR readMessage(struct can_frame *frame);
```

In conditions that masks and filters have been set. This function can only get frames that meet the requirements of masks and filters.

You can choise one of two method to receive: interrup-based and polling

Example of poll read

```C++
struct can_frame frame;

void loop() {
    if (mcp2515.readMessage(&frame) == MCP_CAN::ERROR_OK) {
        // frame contains received message
    }
}
```

Example of interrupt based read
```C++
bool interrupt = false;
struct can_frame frame;
	
void irqHandler() {
    interrupt = true;
}
	
void setup() {
    ...
    attachInterrupt(0, irqHandler, FALLING);
}
	
void loop() {
    if (interrupt) {
        interrupt = false;
       
        uint8_t irq = mcp2515.getInterrupts();
        
        if (irq & MCP_CAN::CANINTF_RX0IF) {
            if (mcp2515.readMessage(MCP_CAN::RXB0, &frame) == MCP_CAN::ERROR_OK) {
                // frame contains received from RXB0 message
            }
        }
            
        if (irq & MCP_CAN::CANINTF_RX1IF) {
            if (mcp2515.readMessage(MCP_CAN::RXB1, &frame) == MCP_CAN::ERROR_OK) {
                // frame contains received from RXB1 message
            }
        }
    }
}
```
<br>
## 6. Examples

Example implementation of CanHacker (lawicel) protocol based device: [https://github.com/autowp/can-usb](https://github.com/autowp/can-usb)

<br>
For more information, please refer to [wiki page](http://www.seeedstudio.com/wiki/CAN-BUS_Shield).

    
----

This software is written by loovee ([luweicong@seeed.cc](luweicong@seeed.cc "luweicong@seeed.cc")) for seeed studio,<br>
Updated by Dmitry ([https://github.com/autowp](https://github.com/autowp "https://github.com/autowp"))<br>
and is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check [LICENSE.md](LICENSE.md) for more information.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed Studio is an open hardware facilitation company based in Shenzhen, China. <br>
Benefiting from local manufacture power and convenient global logistic system, <br>
we integrate resources to serve new era of innovation. Seeed also works with <br>
global distributors and partners to push open hardware movement.<br>
