#include "mcp2518fd_can.h"

//! SPI Transmit buffer
uint8_t spiTransmitBuffer[96];

//! SPI Receive buffer
uint8_t spiReceiveBuffer[96];


#define DRV_CANFDSPI_INDEX_0 0

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
byte mcp2518fd::begin(byte speedset, const byte clockset) {
    pSPI->begin();
    byte res = mcp2518fd_init(speedset, clockset);

    return ((res == MCP2515_OK) ? CAN_OK : CAN_FAILINIT);
}


/*********************************************************************************************************
** Function name:           mcp2518_reset
** Descriptions:            reset the device
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_reset(void) {

    spiTransmitBuffer[0] = (uint8_t) (cINSTRUCTION_RESET << 4);
    spiTransmitBuffer[1] = 0;


    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,2,true);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
    delay(10);
}
   
/*********************************************************************************************************
** Function name:           mcp2518fd_readRegister
** Descriptions:            read register
*********************************************************************************************************/
int8_t mcp2518fd::mcp2518fd_readRegister(const byte address,uint8_t *rxd) {
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    spiTransmitBuffer[2] = 0;

    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,3,true);
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
    
    *rxd = spiReceiveBuffer[2];
    return 0;
}

int8_t mcp2518fd::mcp2518fd_readRegisterWord(const byte address,uint8_t *rxd) {
 
     // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,6,true);
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

     // Update data
    *rxd = 0;
    for (i = 2; i < 6; i++) {
        x = (uint32_t) spiReceiveBuffer[i];
        *rxd += x << ((i - 2)*8);
    }
    return 0;
}

int8_t mcp2518fd::mcp2518fd_writeRegister(const byte address,uint8_t txd) {
    
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    spiTransmitBuffer[2] = txd;

    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,3,true);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    return 0;
}


int8_t mcp2518fd::mcp2518fd_writeRegisterWord(const byte address,uint8_t txd) {
    
    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    // Split word into 4 bytes and add them to buffer
    for (i = 0; i < 4; i++) {
        spiTransmitBuffer[i + 2] = (uint8_t) ((txd >> (i * 8)) & 0xFF);
    }
    
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,6,true);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    return 0;
}

int8_t mcp2518fd::mcp2518fd_ReadRegisterHalfWord(uint16_t address, uint16_t *rxd) {

    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,4,true);


     // Update data
    *rxd = 0;
    for (i = 2; i < 4; i++) {
        x = (uint32_t) spiReceiveBuffer[i];
        *rxd += x << ((i - 2)*8);
    }

    return 0;
}


int8_t mcp2518fd::mcp2518fd_WriteRegisterHalfWord(uint16_t address,uint16_t txd) {

    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    // Split word into 2 bytes and add them to buffer
    for (i = 0; i < 2; i++) {
        spiTransmitBuffer[i + 2] = (uint8_t) ((txd >> (i * 8)) & 0xFF);
    }

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,4,true);

    return 0;
}


int8_t mcp2518fd::mcp2518fd_WriteByteSafe(uint16_t address,uint8_t txd) {

    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE_SAFE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    spiTransmitBuffer[2] = txd;

    // Add CRC
    crcResult = DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, 3);
    spiTransmitBuffer[3] = (crcResult >> 8) & 0xFF;
    spiTransmitBuffer[4] = crcResult & 0xFF;

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,5,true);

    return 0;
}

int8_t mcp2518fd::mcp2518fd_WriteWordSafe(uint16_t address,uint32_t txd) {
    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE_SAFE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    // Split word into 4 bytes and add them to buffer
    for (i = 0; i < 4; i++) {
        spiTransmitBuffer[i + 2] = (uint8_t) ((txd >> (i * 8)) & 0xFF);
    }

    / Add CRC
    crcResult = DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, 6);
    spiTransmitBuffer[6] = (crcResult >> 8) & 0xFF;
    spiTransmitBuffer[7] = crcResult & 0xFF;

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,8,true);

    return 0;
}

int8_t mcp2518fd::mcp2518fd_ReadByteArray(uint16_t address,uint8_t *rxd, uint16_t nBytes) {

    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    // Clear data
    for (i = 2; i < spiTransferSize; i++) {
        spiTransmitBuffer[i] = 0;
    }

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nBytes + 2),true); 

    // Update data
    for (i = 0; i < nBytes; i++) {
        rxd[i] = spiReceiveBuffer[i + 2];
    }

    return 0;
}


int8_t mcp2518fd::mcp2518fd_WriteByteArray(uint16_t address,uint8_t *txd, uint16_t nBytes) {

    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);

    // Add data
    for (i = 2; i < spiTransferSize; i++) {
        spiTransmitBuffer[i] = txd[i - 2];
    }

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nBytes + 2),true); 

    return 0;
}


int8_t mcp2518fd::mcp2518fd_ReadByteArrayWithCRC(uint16_t address,uint8_t *rxd, uint16_t nBytes, bool fromRam, bool* crcIsCorrect) {
    
    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_READ_CRC << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    if (fromRam) {
        spiTransmitBuffer[2] = nBytes >> 2;
    } else {
        spiTransmitBuffer[2] = nBytes;
    }

    // Clear data
    for (i = 3; i < spiTransferSize; i++) {
        spiTransmitBuffer[i] = 0;
    }
    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nBytes + 5),true); 

    // Get CRC from controller
    crcFromSpiSlave = (uint16_t) (spiReceiveBuffer[spiTransferSize - 2] << 8) + (uint16_t) (spiReceiveBuffer[spiTransferSize - 1]);

    // Use the receive buffer to calculate CRC
    // First three bytes need to be command
    spiReceiveBuffer[0] = spiTransmitBuffer[0];
    spiReceiveBuffer[1] = spiTransmitBuffer[1];
    spiReceiveBuffer[2] = spiTransmitBuffer[2];
    crcAtController = DRV_CANFDSPI_CalculateCRC16(spiReceiveBuffer, nBytes + 3);

    // Compare CRC readings
    if (crcFromSpiSlave == crcAtController) {
        *crcIsCorrect = true;
    } else {
        *crcIsCorrect = false;
    }

    // Update data
    for (i = 0; i < nBytes; i++) {
        rxd[i] = spiReceiveBuffer[i + 3];
    }

    return 0;

}

int8_t mcp2518fd::mcp2518fd_WriteByteArrayWithCRC(uint16_t address,uint8_t *txd, uint16_t nBytes, bool fromRam) {
    
    // Compose command
    spiTransmitBuffer[0] = (uint8_t) ((cINSTRUCTION_WRITE_CRC << 4) + ((address >> 8) & 0xF));
    spiTransmitBuffer[1] = (uint8_t) (address & 0xFF);
    if (fromRam) {
        spiTransmitBuffer[2] = nBytes >> 2;
    } else {
        spiTransmitBuffer[2] = nBytes;
    }

    // Add data
    for (i = 0; i < nBytes; i++) {
        spiTransmitBuffer[i + 3] = txd[i];
    }

    // Add CRC
    crcResult = DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, spiTransferSize - 2);
    spiTransmitBuffer[spiTransferSize - 2] = (uint8_t) ((crcResult >> 8) & 0xFF);
    spiTransmitBuffer[spiTransferSize - 1] = (uint8_t) (crcResult & 0xFF);

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nBytes + 5),true); 

    return 0;

}

int8_t mcp2518fd::mcp2518fd_ReadWordArray(uint16_t address,uint32_t *rxd, uint16_t nWords) {
    // Compose command
    spiTransmitBuffer[0] = (cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF);
    spiTransmitBuffer[1] = address & 0xFF;

    // Clear data
    for (i = 2; i < spiTransferSize; i++) {
        spiTransmitBuffer[i] = 0;
    }

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nWords * 4 + 2),true); 

    // Convert Byte array to Word array
    n = 2;
    for (i = 0; i < nWords; i++) {
        w.word = 0;
        for (j = 0; j < 4; j++, n++) {
            w.byte[j] = spiReceiveBuffer[n];
        }
        rxd[i] = w.word;
    }

    return 0;
}

int8_t mcp2518fd::mcp2518fd_WriteWordArray(uint16_t address,uint32_t *txd, uint16_t nWords) {
    // Compose command
    spiTransmitBuffer[0] = (cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF);
    spiTransmitBuffer[1] = address & 0xFF;

    // Convert ByteArray to word array
    n = 2;
    for (i = 0; i < nWords; i++) {
        w.word = txd[i];
        for (j = 0; j < 4; j++, n++) {
            spiTransmitBuffer[n] = w.byte[j];
        }
    }

    spi_readwrite((void)spiTransmitBuffer,(void)spiReceiveBuffer,(nWords * 4 + 2),true);

    return 0;
}


void mcp2518fd::mcp2518fd_init_txobj() {
    // Configure transmit message
    txObj.word[0] = 0;
    txObj.word[1] = 0;

    txObj.bF.id.SID = TX_RESPONSE_ID;
    txObj.bF.id.EID = 0;

    txObj.bF.ctrl.BRS = 1;
    txObj.bF.ctrl.DLC = CAN_DLC_64;
    txObj.bF.ctrl.FDF = 1;
    txObj.bF.ctrl.IDE = 0;
    // Configure message data
    for (i = 0; i < MAX_DATA_BYTES; i++) txd[i] = txObj.bF.id.SID + i;
}


/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_sendMsgBuf(const byte* buf) {
    return mcp2518fd_sendMsg(buf);
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_sendMsg(const byte* buf) {
    mcp2518fd_init_txobj();

    // Prepare data
    Nop();
    Nop();
    txObj.bF.id.SID = TX_RESPONSE_ID;

    txObj.bF.ctrl.DLC = 0;
    txObj.bF.ctrl.IDE = 0;
    txObj.bF.ctrl.BRS = true;
    txObj.bF.ctrl.FDF = 1;
    n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC) 0);
    for (i = 0; i < n; i++)
    {
        txd[i] = buf[i]
    }

    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;
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
    uint8_t n = DRV_CANFDSPI_DlcToDataBytes(txObj.bF.ctrl.DLC);

    DRV_CANFDSPI_TransmitChannelLoad(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txObj, txd, n, true);
 
}



/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
*********************************************************************************************************/
byte mcp2518fd::mcp2518fd_init(const byte canSpeed, const byte clock) {

    byte res = 0;
    // Reset device
    mcp2518fd_reset();

    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(DRV_CANFDSPI_INDEX_0);
    
    DRV_CANFDSPI_RamInit(DRV_CANFDSPI_INDEX_0, 0xff);

    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 1;
    config.StoreInTEF = 0;

    DRV_CANFDSPI_Configure(DRV_CANFDSPI_INDEX_0, &config);

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_64;
    txConfig.TxPriority = 1;

    DRV_CANFDSPI_TransmitChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txConfig);

    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_64;

    DRV_CANFDSPI_ReceiveChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxConfig);

    // Setup RX Filter
    fObj.word = 0;
    fObj.bF.SID = 0xda;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &fObj.bF);

    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0;
    mObj.bF.MIDE = 1; // Only allow standard IDs
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

    // Setup Transmit and Receive Interrupts
    DRV_CANFDSPI_GpioModeConfigure(DRV_CANFDSPI_INDEX_0, GPIO_MODE_INT, GPIO_MODE_INT);
	#ifdef APP_USE_TX_INT
    DRV_CANFDSPI_TransmitChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
	#endif
    DRV_CANFDSPI_ReceiveChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    DRV_CANFDSPI_ModuleEventEnable(DRV_CANFDSPI_INDEX_0, CAN_TX_EVENT | CAN_RX_EVENT);

    // Select Normal Mode
    DRV_CANFDSPI_OperationModeSelect(DRV_CANFDSPI_INDEX_0, CAN_NORMAL_MODE);

    }
    return 0;

}