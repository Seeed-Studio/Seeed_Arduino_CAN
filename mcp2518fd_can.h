#include "mcp_can.h"
#include "mcp2518fd_canfdspi_api.h"
#include "mcp2518fd_can_dfs.h"
#include "mcp2518fd_can_def.h"


typedef struct _CAN_CONFIG {
    uint32_t DNetFilterCount : 5;
    uint32_t IsoCrcEnable : 1;
    uint32_t ProtocolExpectionEventDisable : 1;
    uint32_t WakeUpFilterEnable : 1;
    uint32_t WakeUpFilterTime : 2;
    uint32_t BitRateSwitchDisable : 1;
    uint32_t RestrictReTxAttempts : 1;
    uint32_t EsiInGatewayMode : 1;
    uint32_t SystemErrorToListenOnly : 1;
    uint32_t StoreInTEF : 1;
    uint32_t TXQEnable : 1;
    uint32_t TxBandWidthSharing : 4;
} CAN_CONFIG;

typedef uint8_t CANFDSPI_MODULE_ID;


class mcp2518fd : public MCP_CAN {
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


    void mcp2518fd_reset(void);                                   // reset mcp2515
    byte mcp2518fd_init(const byte canSpeed, const byte clock);   // mcp2515init
  
    byte mcp2518fd_readRegister(const byte address);
    void mcp2518fd_readRegisterS(const byte address,
                               byte values[],
                               const byte n);
    void mcp2518fd_setRegister(const byte address,                // set mcp2515's register
                             const byte value);
            
    void mcp2518fd_setRegisterS(const byte address,               // set mcp2515's registers
                              const byte values[],
                              const byte n);

                            
};