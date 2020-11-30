

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

// Payload

typedef struct {
	bool On;
	uint8_t Dlc;
	bool Mode;
	uint8_t Counter;
	uint8_t Delay;
	bool BRS;
} APP_Payload;


class SPIClass;

class mcp2518fd {
public:
    void mcp_canbus(uint8_t _CS);
    void init_CS(byte _CS);
    byte begin();     // init can


    // byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte* buf);
    // byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true); // send buf

    void mcp2518fd_sendMsgBuf(const byte* buf,byte len);             // send buf
    void mcp2518fd_sendMsg(const byte* buf,byte len);

    int8_t mcp2518fd_receiveMsg();
                                 
    uint8_t mcp2518fd_init();                 // mcp2518fdinit
    void   mcp2518fd_reset(void);               //reset mcp2518fd
    int8_t mcp2518fd_EccEnable(void);         //Section: ECC
    int8_t mcp2518fd_RamInit(uint8_t d);
    int8_t mcp2518fd_ConfigureObjectReset(CAN_CONFIG* config);
    int8_t mcp2518fd_Configure(CAN_CONFIG* config);
    int8_t mcp2518fd_TransmitChannelConfigureObjectReset(CAN_TX_FIFO_CONFIG* config);
    int8_t mcp2518fd_TransmitChannelConfigure(CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_CONFIG* config);
    int8_t mcp2518fd_ReceiveChannelConfigureObjectReset(CAN_RX_FIFO_CONFIG* config);
    int8_t mcp2518fd_ReceiveChannelConfigure(CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_CONFIG* config);
    int8_t mcp2518fd_FilterObjectConfigure(CAN_FILTER filter, CAN_FILTEROBJ_ID* id);
    int8_t mcp2518fd_FilterMaskConfigure(CAN_FILTER filter, CAN_MASKOBJ_ID* mask);
    int8_t mcp2518fd_FilterToFifoLink(CAN_FILTER filter, CAN_FIFO_CHANNEL channel, bool enable);
    int8_t mcp2518fd_BitTimeConfigure(CAN_BITTIME_SETUP bitTime, CAN_SSP_MODE sspMode,CAN_SYSCLK_SPEED clk);
    int8_t mcp2518fd_GpioModeConfigure(GPIO_PIN_MODE gpio0, GPIO_PIN_MODE gpio1);
    int8_t mcp2518fd_TransmitChannelEventEnable(CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT flags);
    int8_t mcp2518fd_ReceiveChannelEventEnable(CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_EVENT flags);
    int8_t mcp2518fd_ModuleEventEnable(CAN_MODULE_EVENT flags);
    int8_t mcp2518fd_OperationModeSelect(CAN_OPERATION_MODE opMode);
    void   mcp2518fd_TransmitMessageQueue();
    int8_t mcp2518fd_TransmitChannelEventGet(CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT* flags);
    int8_t mcp2518fd_ErrorCountStateGet(uint8_t* tec, uint8_t* rec, CAN_ERROR_STATE* flags);
    int8_t mcp2518fd_TransmitChannelLoad(CAN_FIFO_CHANNEL channel, CAN_TX_MSGOBJ* txObj,
           uint8_t *txd, uint32_t txdNumBytes, bool flush);
    int8_t mcp2518fd_ReceiveChannelEventGet(CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_EVENT* flags);
    int8_t mcp2518fd_ReceiveMessageGet(CAN_FIFO_CHANNEL channel, CAN_RX_MSGOBJ* rxObj,uint8_t *rxd, uint8_t nBytes);
    int8_t mcp2518fd_ReceiveChannelUpdate(CAN_FIFO_CHANNEL channel);



    int8_t mcp2518fd_ReadByte(uint16_t address, uint8_t *rxd);
    int8_t mcp2518fd_WriteByte(uint16_t address, uint8_t txd);
    int8_t mcp2518fd_ReadWord(uint16_t address,uint32_t *rxd);
    int8_t mcp2518fd_WriteWord(uint16_t address,uint32_t txd);
    int8_t mcp2518fd_ReadHalfWord(uint16_t address,uint16_t *rxd);
    int8_t mcp2518fd_WriteHalfWord(uint16_t address,uint16_t txd);
    int8_t mcp2518fd_ReadByteArray(uint16_t address,uint8_t *rxd, uint16_t nBytes);
    int8_t mcp2518fd_WriteByteArray(uint16_t address,uint8_t *txd, uint16_t nBytes);
    int8_t mcp2518fd_WriteByteSafe(uint16_t address,uint8_t txd);
    int8_t mcp2518fd_WriteWordSafe(uint16_t address,uint32_t txd);
    int8_t mcp2518fd_ReadByteArrayWithCRC(uint16_t address,uint8_t *rxd, uint16_t nBytes, bool fromRam, bool* crcIsCorrect);
    int8_t mcp2518fd_WriteByteArrayWithCRC(uint16_t address,uint8_t *txd, uint16_t nBytes, bool fromRam);
    int8_t mcp2518fd_ReadWordArray(uint16_t address,uint32_t *rxd, uint16_t nWords);
    int8_t mcp2518fd_WriteWordArray(uint16_t address,uint32_t *txd, uint16_t nWords);





private:

    byte   ext_flg;                         // identifier xxxID
    // either extended (the 29 LSB) or standard (the 11 LSB)
    unsigned long  can_id;                  // can id
    byte   rtr;                             // rtr
    byte   nReservedTx;                     // Count of tx buffers for reserved send
    byte   mcpMode;                         // Current controller mode

};
#endif