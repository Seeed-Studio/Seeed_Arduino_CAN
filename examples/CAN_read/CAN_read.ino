#include <SPI.h>
#include <mcp_can.h>

struct can_frame canMsg;
MCP_CAN mcp2551(10, MCP_CAN::MODE_NORMAL);


void setup() {
  Serial.begin(115200);
  mcp2551.begin(CAN_125KBPS);
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
  
}

void loop() {
  
  if (mcp2551.readMessage(&canMsg) == MCP_CAN::ERROR_OK) {
        
    Serial.print(canMsg.can_id); // print ID

    Serial.print(canMsg.can_dlc); // print DLC

    for(int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
        
        Serial.print(canMsg.data[i]);
        Serial.print(" ");

    }


    Serial.println();      
  }

}
