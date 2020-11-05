#include "mcp_can.h"


// #define spi_readwrite      pSPI->transfer
// #define spi_read()         spi_readwrite(0x00)
// #define spi_write(spi_val) spi_readwrite(spi_val)
// #define SPI_BEGIN()        pSPI->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0))
// #define SPI_END()          pSPI->endTransaction()


/*********************************************************************************************************
** Function name:           MCP_CAN
** Descriptions:            Constructor
*********************************************************************************************************/
void MCP_CAN::mcp_canbus(byte _CS) {
    pSPI = &SPI; init_CS(_CS);
}   

/*********************************************************************************************************
** Function name:           readMsgBuf
** Descriptions:            read message buf
*********************************************************************************************************/
byte MCP_CAN::readMsgBuf(byte* len, byte buf[]) {
    return readMsgBufID(readRxTxStatus(), &can_id, &ext_flg, &rtr, len, buf);
}

/*********************************************************************************************************
** Function name:           readMsgBufID
** Descriptions:            read message buf and can bus source ID
*********************************************************************************************************/
byte MCP_CAN::readMsgBufID(unsigned long* ID, byte* len, byte buf[]) {
    return readMsgBufID(readRxTxStatus(), ID, &ext_flg, &rtr, len, buf);
}

/*********************************************************************************************************
** Function name:           getCanId
** Descriptions:            when receive something, you can get the can id!!
*********************************************************************************************************/
unsigned long MCP_CAN::getCanId(void) {
    return can_id;
}

/*********************************************************************************************************
** Function name:           isRemoteRequest
** Descriptions:            when receive something, you can check if it was a request
*********************************************************************************************************/
byte MCP_CAN::isRemoteRequest(void) {
    return rtr;
}

/*********************************************************************************************************
** Function name:           isExtendedFrame
** Descriptions:            did we just receive standard 11bit frame or extended 29bit? 0 = std, 1 = ext
*********************************************************************************************************/
byte MCP_CAN::isExtendedFrame(void) {
    return ext_flg;
}

