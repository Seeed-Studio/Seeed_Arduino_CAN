

#ifndef _MCP2518FD_H_
#define _MCP2518FD_H_


#include "mcp2518fd_canfdspi_api.h"
#include "mcp2518fd_can_def.h"
#include "mcp2518fd_can_dfs.h"
#include <Arduino.h>
#include <SPI.h>

#define MCP2518fd_OK         (0)
#define MCP2518fd_FAIL       (1)
#define CAN_OK              (0)
#define CAN_FAILINIT        (1)
#define CAN_FAILTX          (2)

class SPIClass;

class mcp2518fd {
public:
    void mcp_canbus(uint8_t _CS);
    void init_CS(byte _CS);
    byte begin();     // init can


    // byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte* buf);
    // byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true); // send buf

    void mcp2518fd_sendMsgBuf(const byte* buf);             // send buf
    void mcp2518fd_sendMsg(const byte* buf);

    int8_t mcp2518fd_receiveMsg();

    void mcp2518fd_reset(void);                                   // reset mcp2515
    byte mcp2518fd_init();   // mcp2515init

    void mcp2518fd_init_txobj();


private:

    byte   ext_flg;                         // identifier xxxID
    // either extended (the 29 LSB) or standard (the 11 LSB)
    unsigned long  can_id;                  // can id
    byte   rtr;                             // rtr
    byte   nReservedTx;                     // Count of tx buffers for reserved send
    byte   mcpMode;                         // Current controller mode

};
#endif