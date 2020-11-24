#include "mcp2518fd_can.h"

uint8_t   SPICS = 0;
SPIClass* pSPI;

CAN_CONFIG config;

APP_Payload payload;
// Receive objects
CAN_RX_FIFO_CONFIG rxConfig;
REG_CiFLTOBJ fObj;
REG_CiMASK mObj;
CAN_RX_FIFO_EVENT rxFlags;
CAN_RX_MSGOBJ rxObj;
uint8_t rxd[MAX_DATA_BYTES];


// Transmit objects
CAN_TX_FIFO_CONFIG txConfig;
CAN_TX_FIFO_EVENT txFlags;
CAN_TX_MSGOBJ txObj;
uint8_t txd[MAX_DATA_BYTES];

CAN_TX_QUEUE_CONFIG txqueConfig;

// Message IDs
#define TX_REQUEST_ID       0x300
#define TX_RESPONSE_ID      0x301
#define BUTTON_STATUS_ID    0x201
#define LED_STATUS_ID       0x200
#define PAYLOAD_ID          0x101

#define MAX_TXQUEUE_ATTEMPTS 50

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

// Maximum number of data bytes in message
#define MAX_DATA_BYTES 64

// //! SPI Transmit buffer
// uint8_t spiTransmitBuffer[96];

// //! SPI Receive buffer
// uint8_t spiReceiveBuffer[96];

CAN_BUS_DIAGNOSTIC busDiagnostics;
uint8_t tec;
uint8_t rec;
CAN_ERROR_STATE errorFlags;

#define DRV_CANFDSPI_INDEX_0 0




/*********************************************************************************************************
** Function name:           MCP_CAN
** Descriptions:            Constructor
*********************************************************************************************************/
void mcp2518fd::mcp_canbus(uint8_t _CS)  {
    nReservedTx = 0;
    pSPI = &SPI; 
    init_CS(_CS);
}



/*********************************************************************************************************
** Function name:           init_CS
** Descriptions:            init CS pin and set UNSELECTED
*********************************************************************************************************/
void mcp2518fd::init_CS(byte _CS) {
    if (_CS == 0) {
        return;
    }
    SPICS = _CS;
    pinMode(SPICS, OUTPUT);
    MCP2518fd_UNSELECT();
}



/*********************************************************************************************************
** Function name:           begin
** Descriptions:            init can and set speed
*********************************************************************************************************/
byte mcp2518fd::begin() {
    Serial.println("into mcp25184fd begin\n\r");
    SPI.begin();
    byte res = mcp2518fd_init();

    return ((res == MCP2518fd_OK) ? CAN_OK : CAN_FAILINIT);
}



void mcp2518fd::mcp2518fd_TransmitMessageQueue(void)
{
    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;
    Serial.printf("into mcp2518fd_TransmitMessageQueue\n\r");
    
    // Check if FIFO is not full
    do {
        DRV_CANFDSPI_TransmitChannelEventGet(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            Nop();
            Nop();
            DRV_CANFDSPI_ErrorCountStateGet(DRV_CANFDSPI_INDEX_0, &tec, &rec, &errorFlags);
            return;
        }
        attempts--;
    }
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC)txObj.bF.ctrl.DLC);
   
    DRV_CANFDSPI_TransmitChannelLoad(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txObj, txd, n, true);

}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_sendMsgBuf(const byte* buf,byte len) {
    return mcp2518fd_sendMsg(buf,len);
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_sendMsg(const byte* buf, byte len) {
    Serial.printf("into mcp2518fdsendMsg\n\r");
    uint8_t n;
    int i;

    // Configure message data
    txObj.word[0] = 0;
    txObj.word[1] = 0;
    txObj.bF.id.SID = TX_REQUEST_ID;
    txObj.bF.ctrl.DLC = CAN_DLC_64;
    txObj.bF.ctrl.IDE = 0;
    txObj.bF.ctrl.BRS = true;
    txObj.bF.ctrl.FDF = 0;

    //Prepare data
    for(i = 0; i< len; i++)
    {
        txd[i] = buf[i];
    }

    mcp2518fd_TransmitMessageQueue();

}


int8_t mcp2518fd::mcp2518fd_receiveMsg() {
    DRV_CANFDSPI_ReceiveChannelEventGet(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxFlags);

    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
        DRV_CANFDSPI_ReceiveMessageGet(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxObj, rxd, 8);
        for (int i = 0; i < 8; i++) {
            Serial.println(rxd[i]); Serial.println("\t");
        }
        Serial.println();

    }

    return 0;

}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
*********************************************************************************************************/
uint8_t mcp2518fd::mcp2518fd_init() {
    // Reset device
    DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);

    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(DRV_CANFDSPI_INDEX_0);
   

    DRV_CANFDSPI_RamInit(DRV_CANFDSPI_INDEX_0, 0xff);


    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 1; 
    config.StoreInTEF = 0;
    Serial.println("DRV_CANFDSPI_ConfigureObjectReset-------end\n\r");
    DRV_CANFDSPI_Configure(DRV_CANFDSPI_INDEX_0, &config);
    Serial.println("DRV_CANFDSPI_Configure-------end\n\r");

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    Serial.println("DRV_CANFDSPI_TransmitChannelConfigureObjectReset-------end\n\r");
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_64;
    txConfig.TxPriority = 1;
    DRV_CANFDSPI_TransmitChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txConfig);
    Serial.println("DRV_CANFDSPI_TransmitChannelConfigure-------end\n\r");


    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    Serial.println("DRV_CANFDSPI_ReceiveChannelConfigureObjectReset-------end\n\r");
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_64;
    DRV_CANFDSPI_ReceiveChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxConfig);
 

    // Setup RX Filter
    fObj.word = 0;
    fObj.bF.SID = 0xda;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &fObj.bF);
    Serial.println("DRV_CANFDSPI_FilterObjectConfigure-------end\n\r");
    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0;
    mObj.bF.MIDE = 1; // Only allow standard IDs
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_1M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

    // Setup Transmit and Receive Interrupts
    DRV_CANFDSPI_GpioModeConfigure(DRV_CANFDSPI_INDEX_0, GPIO_MODE_INT, GPIO_MODE_INT);
	#ifdef APP_USE_TX_INT
    DRV_CANFDSPI_TransmitChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
	#endif
    DRV_CANFDSPI_ReceiveChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    DRV_CANFDSPI_ModuleEventEnable(DRV_CANFDSPI_INDEX_0,(CAN_MODULE_EVENT)(CAN_TX_EVENT | CAN_RX_EVENT));

    // Select Normal Mode
    DRV_CANFDSPI_OperationModeSelect(DRV_CANFDSPI_INDEX_0, CAN_CLASSIC_MODE);

    CAN_OPERATION_MODE abc;
    abc  = DRV_CANFDSPI_OperationModeGet(0);
    Serial.printf("DRV_CANFDSPI_OperationModeGet = %d\n\r",abc);   

    return 0;

}