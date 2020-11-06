#ifndef _MCP_CAN_H_
#define _MCP_CAN_H_

#include "mcp2515_can_dfs.h"

#define MAX_CHAR_IN_MESSAGE 8

#define spi_readwrite      pSPI->transfer
#define spi_read()         spi_readwrite(0x00)
#define spi_write(spi_val) spi_readwrite(spi_val)
#define SPI_BEGIN()        pSPI->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0))
#define SPI_END()          pSPI->endTransaction()

class MCP_CAN {
public:
    void mcp_canbus(byte _CS = 0);
    virtual void init_CS(byte _CS) = 0;                      // define CS after construction before begin()
    void setSPI(SPIClass* _pSPI) {
        pSPI = _pSPI;    // define SPI port to use before begin()
    }
    virtual void enableTxInterrupt(bool enable = true) = 0;  // enable transmit interrupt

    void reserveTxBuffers(byte nTxBuf = 0) {
        nReservedTx = (nTxBuf < MCP_N_TXBUFFERS ? nTxBuf : MCP_N_TXBUFFERS - 1);
    }
    byte getLastTxBuffer() {
        return MCP_N_TXBUFFERS - 1;    // read index of last tx buffer
    }

    virtual byte begin(byte speedset, const byte clockset = MCP_16MHz) = 0;     // init can
    virtual byte init_Mask(byte num, byte ext, unsigned long ulData) = 0;       // init Masks
    virtual byte init_Filt(byte num, byte ext, unsigned long ulData);       // init filters
    virtual void setSleepWakeup(byte enable) = 0;                               // Enable or disable the wake up interrupt (If disabled the MCP2515 will not be woken up by CAN bus activity, making it send only)
    virtual byte sleep() = 0;													// Put the MCP2515 in sleep mode
    virtual byte wake() = 0;													// Wake MCP2515 manually from sleep
    virtual byte setMode(byte opMode) = 0;                                      // Set operational mode
    virtual byte getMode() = 0;				                                    // Get operational mode
    virtual byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true) = 0; // send buf
    virtual byte sendMsgBuf(unsigned long id, byte ext, byte len, const byte* buf, bool wait_sent = true) = 0;             // send buf
    byte readMsgBuf(byte* len, byte* buf);                          // read buf
    byte readMsgBufID(unsigned long* ID, byte* len, byte* buf);     // read buf with object ID
    virtual byte checkReceive(void) = 0;                                        // if something received
    virtual byte checkError(void) = 0;                                          // if something error
    unsigned long getCanId(void);                                   // get can id when receive
    byte isRemoteRequest(void);                                     // get RR flag when receive
    byte isExtendedFrame(void);                                     // did we recieve 29bit frame?

    virtual byte readMsgBufID(byte status, volatile unsigned long* id, volatile byte* ext, volatile byte* rtr, volatile byte* len,
                      volatile byte* buf) = 0; // read buf with object ID
    virtual byte trySendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf,
                       byte iTxBuf = 0xff) = 0; // as sendMsgBuf, but does not have any wait for free buffer
    virtual byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len,
                    volatile const byte* buf) = 0; // send message buf by using parsed buffer status
    inline byte trySendExtMsgBuf(unsigned long id, byte len, const byte* buf,
                                 byte iTxBuf = 0xff) { // as trySendMsgBuf, but set ext=1 and rtr=0
        return trySendMsgBuf(id, 1, 0, len, buf, iTxBuf);
    }
    inline byte sendExtMsgBuf(byte status, unsigned long id, byte len,
                              volatile const byte* buf) { // as sendMsgBuf, but set ext=1 and rtr=0
        return sendMsgBuf(status, id, 1, 0, len, buf);
    }
    virtual void clearBufferTransmitIfFlags(byte flags = 0) = 0;                // Clear transmit flags according to status
    virtual byte readRxTxStatus(void) = 0;                                      // read has something send or received
    virtual byte checkClearRxStatus(byte* status) = 0;                          // read and clear and return first found rx status bit
    virtual byte checkClearTxStatus(byte* status,
                            byte iTxBuf = 0xff) = 0;      // read and clear and return first found or buffer specified tx status bit

    virtual bool mcpPinMode(const byte pin, const byte
                    mode) = 0;                  // switch supported pins between HiZ, interrupt, output or input
    virtual bool mcpDigitalWrite(const byte pin, const byte mode) = 0;             // write HIGH or LOW to RX0BF/RX1BF
    virtual byte mcpDigitalRead(const byte pin) = 0;                               // read HIGH or LOW from supported pins

    byte   ext_flg;                         // identifier xxxID
    // either extended (the 29 LSB) or standard (the 11 LSB)
    unsigned long  can_id;                  // can id
    byte   rtr;                             // rtr
    byte   SPICS;
    SPIClass* pSPI;
    byte   nReservedTx = 0;                     // Count of tx buffers for reserved send
    byte   mcpMode;   
};

template <class T>
class Mcp_Factory {
public:
    // MCP_CAN* getmcpcan(std:string device) {
    //     if(device == nullptr)
    //     {
    //         return nullptr;
    //     }
    //     if(device == "mcp2515") {
    //         return new mcp2515();
    //     }

    //     // if(device == "mcp2518") {
    //     //     return new mcp2518fd();
    //     // }
    // }

    Mcp_Factory() {
      controller = new T();
    }
    T* controller;
};

#endif