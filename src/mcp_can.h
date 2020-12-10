#ifndef _MCP_CAN_H_
#define _MCP_CAN_H_

#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>



#define CAN_OK              (0)
#define CAN_FAILINIT        (1)
#define CAN_FAILTX          (2)
#define CAN_MSGAVAIL        (3)




class MCP_CAN
{
public:
    virtual void enableTxInterrupt(bool enable = true) = 0; // enable transmit interrupt
    virtual void reserveTxBuffers(byte nTxBuf = 0) = 0;
    virtual byte getLastTxBuffer() = 0;
    virtual byte begin(byte speedset, const byte clockset) = 0;                                                                                             // init can
    virtual byte init_Mask(byte num, byte ext, unsigned long ulData) = 0;                                                                                   // init Masks
    virtual byte init_Filt(byte num, byte ext, unsigned long ulData) = 0;                                                                                   // init filters
    virtual void setSleepWakeup(byte enable) = 0;                                                                                                           // Enable or disable the wake up interrupt (If disabled the MCP2515 will not be woken up by CAN bus activity, making it send only)
    virtual byte sleep() = 0;                                                                                                                               // Put the MCP2515 in sleep mode
    virtual byte wake() = 0;                                                                                                                                // Wake MCP2515 manually from sleep
    virtual byte setMode(byte opMode) = 0;                                                                                                                  // Set operational mode
    virtual byte getMode() = 0;                                                                                                                             // Get operational mode
    virtual byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte *buf, bool wait_sent = true) = 0;                                 // send buf
    virtual byte sendMsgBuf(unsigned long id, byte ext, byte len, const byte *buf, bool wait_sent = true) = 0;                                              // send buf
    virtual byte readMsgBuf(byte *len, byte *buf) = 0;                                                                                                      // read buf
    virtual byte readMsgBufID(unsigned long *ID, byte *len, byte *buf) = 0;                                                                                 // read buf with object ID
    virtual byte checkReceive(void) = 0;                                                                                                                    // if something received
    virtual byte checkError(void) = 0;                                                                                                                      // if something error
    virtual unsigned long getCanId(void) = 0;                                                                                                               // get can id when receive
    virtual byte isRemoteRequest(void) = 0;                                                                                                                 // get RR flag when receive
    virtual byte isExtendedFrame(void) = 0;                                                                                                                 // did we recieve 29bit frame?
    virtual byte readMsgBufID(byte status, volatile unsigned long *id, volatile byte *ext, volatile byte *rtr, volatile byte *len, volatile byte *buf) = 0; // read buf with object ID
    virtual byte trySendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte *buf, byte iTxBuf = 0xff) = 0;                                 // as sendMsgBuf, but does not have any wait for free buffer
    virtual byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte *buf) = 0;                                  // send message buf by using parsed buffer status
    virtual void clearBufferTransmitIfFlags(byte flags = 0) = 0;                                                                                            // Clear transmit flags according to status
    virtual byte readRxTxStatus(void) = 0;                                                                                                                  // read has something send or received
    virtual byte checkClearRxStatus(byte *status) = 0;                                                                                                      // read and clear and return first found rx status bit
    virtual byte checkClearTxStatus(byte *status, byte iTxBuf = 0xff) = 0;                                                                                  // read and clear and return first found or buffer specified tx status bit
    virtual bool mcpPinMode(const byte pin, const byte mode) = 0;                                                                                           // switch supported pins between HiZ, interrupt, output or input
    virtual bool mcpDigitalWrite(const byte pin, const byte mode) = 0;                                                                                      // write HIGH or LOW to RX0BF/RX1BF
    virtual byte mcpDigitalRead(const byte pin) = 0;                                                                                                        // read HIGH or LOW from supported pins
public:
    MCP_CAN(byte _CS);
    void init_CS(byte _CS); // define CS after construction before begin()
    void setSPI(SPIClass *_pSPI);
    inline byte trySendExtMsgBuf(unsigned long id, byte len, const byte *buf, byte iTxBuf = 0xff);
    inline byte sendExtMsgBuf(byte status, unsigned long id, byte len, volatile const byte *buf);

protected:
    byte ext_flg; // identifier xxxID
    // either extended (the 29 LSB) or standard (the 11 LSB)
    unsigned long can_id; // can id
    byte SPICS;
    SPIClass *pSPI;
    byte mcpMode;     // Current controller mode
};

#endif