
#include "mcp_can.h"

class mcp2515 : public MCP_CAN {
public:
    
    void init_CS(byte _CS);
    void enableTxInterrupt(bool enable = true);  // enable transmit interrupt
    byte begin(byte speedset, const byte clockset = MCP_16MHz);     // init can
    byte init_Mask(byte num, byte ext, unsigned long ulData);
    byte init_Filt(byte num, byte ext, unsigned long ulData);       // init filters
    void setSleepWakeup(const byte enable);
    byte sleep();
    byte wake();
    byte setMode(byte opMode);
    byte getMode();
    byte checkReceive(void);
    byte checkError(void);
    byte readMsgBufID(byte status, volatile unsigned long* id, volatile byte* ext, volatile byte* rtr, volatile byte* len,
                      volatile byte* buf);
    byte trySendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, byte iTxBuf);
    byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte* buf);
    byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true); // send buf
    byte sendMsgBuf(unsigned long id, byte ext, byte len, const byte* buf, bool wait_sent = true);             // send buf
    void clearBufferTransmitIfFlags(byte flags);
    byte readRxTxStatus(void);
    byte checkClearRxStatus(byte* status);
    byte checkClearTxStatus(byte* status, byte iTxBuf);
    bool mcpPinMode(const byte pin, const byte mode);
    bool mcpDigitalWrite(const byte pin, const byte mode);
    byte mcpDigitalRead(const byte pin);



    void mcp2515_reset(void);                                   // reset mcp2515

    byte mcp2515_readRegister(const byte address);              // read mcp2515's register

    void mcp2515_readRegisterS(const byte address,
                               byte values[],
                               const byte n);
    void mcp2515_setRegister(const byte address,                // set mcp2515's register
                             const byte value);

    void mcp2515_setRegisterS(const byte address,               // set mcp2515's registers
                              const byte values[],
                              const byte n);

    void mcp2515_initCANBuffers(void);

    void mcp2515_modifyRegister(const byte address,             // set bit of one register
                                const byte mask,
                                const byte data);

    byte mcp2515_readStatus(void);                              // read mcp2515's Status
    byte mcp2515_setCANCTRL_Mode(const byte newmode);           // set mode
    byte mcp2515_requestNewMode(const byte newmode);                  // Set mode
    byte mcp2515_configRate(const byte canSpeed, const byte clock);  // set baudrate
    byte mcp2515_init(const byte canSpeed, const byte clock);   // mcp2515init

    void mcp2515_write_id(const byte mcp_addr,                  // write can id
                          const byte ext,
                          const unsigned long id);

    void mcp2515_read_id(const byte mcp_addr,                   // read can id
                         byte* ext,
                         unsigned long* id);

    void mcp2515_write_canMsg(const byte buffer_sidh_addr, unsigned long id, byte ext, byte rtr, byte len,
                              volatile const byte* buf);     // read can msg
    void mcp2515_read_canMsg(const byte buffer_load_addr, volatile unsigned long* id, volatile byte* ext,
                             volatile byte* rtr, volatile byte* len, volatile byte* buf);   // write can msg
    void mcp2515_start_transmit(const byte mcp_addr);           // start transmit
    byte mcp2515_getNextFreeTXBuf(byte* txbuf_n);               // get Next free txbuf
    byte mcp2515_isTXBufFree(byte* txbuf_n, byte iBuf);         // is buffer by index free

    /*
        can operator function
    */

    byte sendMsg(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true); // send message

// private:
//     byte   ext_flg;                         // identifier xxxID
//     // either extended (the 29 LSB) or standard (the 11 LSB)
//     unsigned long  can_id;                  // can id
//     byte   rtr;                             // rtr
//     byte   SPICS;
//     SPIClass* pSPI;
//     byte   nReservedTx = 0;                     // Count of tx buffers for reserved send
//     byte   mcpMode;
};