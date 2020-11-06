#include "mcp_can.h"
#include "mcp2518fd_can_dfs.h"
#include "mcp2518fd_can_def.h"
#include "mcp2518fd_canfdspi_api.h"




#define MAX_TXQUEUE_ATTEMPTS 50

// Code anchor for break points
#define Nop() asm("nop")

extern void _exit(int status)
{
	asm("BKPT #0");
	for (;;);
}

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2


// Transmit objects
CAN_TX_FIFO_CONFIG txConfig;
CAN_TX_FIFO_EVENT txFlags;
CAN_TX_MSGOBJ txObj;
uint8_t txd[MAX_DATA_BYTES];


// Receive objects
CAN_RX_FIFO_CONFIG rxConfig;
REG_CiFLTOBJ fObj;
REG_CiMASK mObj;
CAN_RX_FIFO_EVENT rxFlags;
CAN_RX_MSGOBJ rxObj;
uint8_t rxd[MAX_DATA_BYTES];

REG_t reg;

uint8_t ledCount = 0, ledState = 0;

uint8_t i;

CAN_BUS_DIAGNOSTIC busDiagnostics;
uint8_t tec;
uint8_t rec;
CAN_ERROR_STATE errorFlags;


// Message IDs
#define TX_REQUEST_ID       0x300
#define TX_RESPONSE_ID      0x301
#define BUTTON_STATUS_ID    0x201
#define LED_STATUS_ID       0x200
#define PAYLOAD_ID          0x101

// Payload

typedef struct {
	bool On;
	uint8_t Dlc;
	bool Mode;
	uint8_t Counter;
	uint8_t Delay;
	bool BRS;
} APP_Payload;



class mcp2518fd : public MCP_CAN {
public:
    
    void init_CS(byte _CS);
    byte begin(byte speedset, const byte clockset = MCP_16MHz);     // init can


    // byte sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte* buf);
    // byte sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent = true); // send buf

    void mcp2518fd_sendMsgBuf(const byte* buf);             // send buf
    void mcp2518fd_sendMsg(const byte* buf);

    void mcp2518fd_reset(void);                                   // reset mcp2515
    byte mcp2518fd_init(const byte canSpeed, const byte clock);   // mcp2515init

    void mcp2518fd_init_txobj();
  
    int8_t mcp2518fd_readRegister(const byte address,uint8_t *rxd);
    int8_t mcp2518fd_writeRegister(const byte address,uint8_t txd);

    int8_t mcp2518fd_readRegisterWord(const byte address,uint8_t *rxd);
    int8_t mcp2518fd_writeRegisterWord(const byte address,uint8_t txd);

    int8_t mcp2518fd_ReadRegisterHalfWord(uint16_t address, uint16_t *rxd);
    int8_t mcp2518fd_WriteRegisterHalfWord(uint16_t address,uint16_t txd);

    int8_t mcp2518fd_WriteByteSafe(uint16_t address,uint8_t txd);
    int8_t mcp2518fd_WriteWordSafe(uint16_t address,uint32_t txd);

    int8_t mcp2518fd_ReadByteArray(uint16_t address,uint8_t *rxd, uint16_t nBytes);
    int8_t mcp2518fd_WriteByteArray(uint16_t address,uint8_t *txd, uint16_t nBytes);

    int8_t mcp2518fd_ReadByteArrayWithCRC(uint16_t address,uint8_t *rxd, uint16_t nBytes, bool fromRam, bool* crcIsCorrect);
    int8_t mcp2518fd_WriteByteArrayWithCRC(uint16_t address,uint8_t *txd, uint16_t nBytes, bool fromRam);

    int8_t mcp2518fd_ReadWordArray(uint16_t address,uint32_t *rxd, uint16_t nWords);
    int8_t mcp2518fd_WriteWordArray(uint16_t address,uint32_t *txd, uint16_t nWords);

};