#include "mcp_can.h"

/*********************************************************************************************************
** Function name:           MCP_CAN
** Descriptions:            Constructor
*********************************************************************************************************/
MCP_CAN::MCP_CAN(byte _CS)
{
    pSPI = &SPI;
    init_CS(_CS);
}

/*********************************************************************************************************
** Function name:           init_CS
** Descriptions:            init CS pin and set UNSELECTED
*********************************************************************************************************/
void MCP_CAN::init_CS(byte _CS)
{
    if (_CS == 0)
    {
        return;
    }
    SPICS = _CS;
    pinMode(SPICS, OUTPUT);
    digitalWrite(SPICS, HIGH);
}

void MCP_CAN::setSPI(SPIClass *_pSPI)
{
    pSPI = _pSPI; // define SPI port to use before begin()
}
