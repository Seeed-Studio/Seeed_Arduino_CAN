#include "mcp2518fd_can.h"

CAN_CONFIG config;
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
** Function name:           enableTxInterrupt
** Descriptions:            enable interrupt for all tx buffers
*********************************************************************************************************/
void mcp2518fd::enableTxInterrupt(bool enable) {
    byte interruptStatus = mcp2518fd_readRegister(MCP_CANINTE);

    if (enable) {
        interruptStatus |= MCP_TX_INT;
    } else {
        interruptStatus &= ~MCP_TX_INT;
    }

    mcp2518fd_setRegister(MCP_CANINTE, interruptStatus);
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
** Function name:           init_Mask
** Descriptions:            init canid Masks
*********************************************************************************************************/
byte mcp2518fd::init_Mask(byte num, byte ext, unsigned long ulData) {
    byte res = MCP2515_OK;
    #if DEBUG_EN
    Serial.print("Begin to set Mask!!\r\n");
    #else
    delay(10);
    #endif
    res = mcp2518fd_setCANCTRL_Mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_EN
        Serial.print("Enter setting mode fall\r\n");
        #else
        delay(10);
        #endif
        return res;
    }

    if (num == 0) {
        mcp2518fd_write_id(MCP_RXM0SIDH, ext, ulData);

    } else if (num == 1) {
        mcp2518fd_write_id(MCP_RXM1SIDH, ext, ulData);
    } else {
        res =  MCP2515_FAIL;
    }

    res = mcp2518fd_setCANCTRL_Mode(mcpMode);
    if (res > 0) {
        #if DEBUG_EN
        Serial.print("Enter normal mode fall\r\n");
        #else
        delay(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    Serial.print("set Mask success!!\r\n");
    #else
    delay(10);
    #endif
    return res;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            init canid filters
*********************************************************************************************************/
byte mcp2518fd::init_Filt(byte num, byte ext, unsigned long ulData) {
    byte res = MCP2515_OK;
    #if DEBUG_EN
    Serial.print("Begin to set Filter!!\r\n");
    #else
    delay(10);
    #endif
    res = mcp25fd_setCANCTRL_Mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_EN
        Serial.print("Enter setting mode fall\r\n");
        #else
        delay(10);
        #endif
        return res;
    }

    switch (num) {
        case 0:
            mcp2518fd_write_id(MCP_RXF0SIDH, ext, ulData);
            break;

        case 1:
            mcp2518fd_write_id(MCP_RXF1SIDH, ext, ulData);
            break;

        case 2:
            mcp2518fd_write_id(MCP_RXF2SIDH, ext, ulData);
            break;

        case 3:
            mcp2518fd_write_id(MCP_RXF3SIDH, ext, ulData);
            break;

        case 4:
            mcp2518fd_write_id(MCP_RXF4SIDH, ext, ulData);
            break;

        case 5:
            mcp2518fd_write_id(MCP_RXF5SIDH, ext, ulData);
            break;

        default:
            res = MCP2515_FAIL;
    }

    res = mcp2518fd_setCANCTRL_Mode(mcpMode);
    if (res > 0) {
        #if DEBUG_EN
        Serial.print("Enter normal mode fall\r\nSet filter fail!!\r\n");
        #else
        delay(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    Serial.print("set Filter success!!\r\n");
    #else
    delay(10);
    #endif

    return res;
}


/*********************************************************************************************************
** Function name:           setSleepWakeup
** Descriptions:            Enable or disable the wake up interrupt (If disabled the MCP2515 will not be woken up by CAN bus activity)
*********************************************************************************************************/
void mcp2518fd::setSleepWakeup(const byte enable) {
    mcp2518fd_modifyRegister(MCP_CANINTE, MCP_WAKIF, enable ? MCP_WAKIF : 0);
}

/*********************************************************************************************************
** Function name:           sleep
** Descriptions:            Put mcp2515 in sleep mode to save power
*********************************************************************************************************/
byte mcp2518fd::sleep() {
    if (getMode() != MODE_SLEEP) {
        return mcp2518fd_setCANCTRL_Mode(MODE_SLEEP);
    } else {
        return CAN_OK;
    }
}

/*********************************************************************************************************
** Function name:           wake
** Descriptions:            wake MCP2515 manually from sleep. It will come back in the mode it was before sleeping.
*********************************************************************************************************/
byte mcp2518fd::wake() {
    byte currMode = getMode();
    if (currMode != mcpMode) {
        return mcp2518FD_setCANCTRL_Mode(mcpMode);
    } else {
        return CAN_OK;
    }
}

/*********************************************************************************************************
** Function name:           setMode
** Descriptions:            Sets control mode
*********************************************************************************************************/
byte mcp2518fd::setMode(const byte opMode) {
    if (opMode !=
            MODE_SLEEP) { // if going to sleep, the value stored in opMode is not changed so that we can return to it later
        mcpMode = opMode;
    }
    return mcp2518fd_setCANCTRL_Mode(opMode);
}

/*********************************************************************************************************
** Function name:           getMode
** Descriptions:            Returns current control mode
*********************************************************************************************************/
byte mcp2518fd::getMode() {
   return mcp2518fd_readRegister(MCP_CANSTAT) & MODE_MASK;
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            check if got something
*********************************************************************************************************/
byte mcp2518fd::checkReceive(void) {
    byte res;
    res = mcp2518fd_readStatus();                                         // RXnIF in Bit 1 and 0
    return ((res & MCP_STAT_RXIF_MASK) ? CAN_MSGAVAIL : CAN_NOMSG);
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            if something error
*********************************************************************************************************/
byte mcp2518fd::checkError(void) {
    byte eflg = mcp2518fd_readRegister(MCP_EFLG);
    return ((eflg & MCP_EFLG_ERRORMASK) ? CAN_CTRLERROR : CAN_OK);
}


/*********************************************************************************************************
** Function name:           readMsgBufID
** Descriptions:            Read message buf and can bus source ID according to status.
**                          Status has to be read with readRxTxStatus.
*********************************************************************************************************/
byte mcp2518fd::readMsgBufID(byte status, volatile unsigned long* id, volatile byte* ext, volatile byte* rtrBit,
                           volatile byte* len, volatile byte* buf) {
    byte rc = CAN_NOMSG;

    if (status & MCP_RX0IF) {                                        // Msg in Buffer 0
        mcp2518fd_read_canMsg(MCP_READ_RX0, id, ext, rtrBit, len, buf);
        rc = CAN_OK;
    } else if (status & MCP_RX1IF) {                                 // Msg in Buffer 1
        mcp2518fd_read_canMsg(MCP_READ_RX1, id, ext, rtrBit, len, buf);
        rc = CAN_OK;
    }

    if (rc == CAN_OK) {
        rtr = *rtrBit;
        // dta_len=*len; // not used on any interface function
        ext_flg = *ext;
        can_id = *id;
    } else {
        *len = 0;
    }

    return rc;
}

/*********************************************************************************************************
** Function name:           trySendMsgBuf
** Descriptions:            Try to send message. There is no delays for waiting free buffer.
*********************************************************************************************************/
byte mcp2518fd::trySendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, byte iTxBuf) {
    byte txbuf_n;

    if (iTxBuf < MCP_N_TXBUFFERS) { // Use specified buffer
        if (mcp2518fd_isTXBufFree(&txbuf_n, iTxBuf) != MCP2515_OK) {
            return CAN_FAILTX;
        }
    } else {
        if (mcp2518fd_getNextFreeTXBuf(&txbuf_n) != MCP2515_OK) {
            return CAN_FAILTX;
        }
    }

    mcp2518fd_write_canMsg(txbuf_n, id, ext, rtrBit, len, buf);

    return CAN_OK;
}


/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            Send message by using buffer read as free from CANINTF status
**                          Status has to be read with readRxTxStatus and filtered with checkClearTxStatus
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit, byte len, volatile const byte* buf) {
    byte txbuf_n = statusToTxSidh(status);

    if (txbuf_n == 0) {
        return CAN_FAILTX;    // Invalid status
    }

    mcp2518fd_modifyRegister(MCP_CANINTF, status, 0);  // Clear interrupt flag
    mcp2518fd_write_canMsg(txbuf_n, id, ext, rtrBit, len, buf);

    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len, const byte* buf, bool wait_sent) {
    return sendMsg(id, ext, rtrBit, len, buf, wait_sent);
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(unsigned long id, byte ext, byte len, const byte* buf, bool wait_sent) {
    return sendMsg(id, ext, 0, len, buf, wait_sent);
}


/*********************************************************************************************************
** Function name:           clearBufferTransmitIfFlags
** Descriptions:            Clear transmit interrupt flags for specific buffer or for all unreserved buffers.
**                          If interrupt will be used, it is important to clear all flags, when there is no
**                          more data to be sent. Otherwise IRQ will newer change state.
*********************************************************************************************************/
void mcp2518fd::clearBufferTransmitIfFlags(byte flags) {
    flags &= MCP_TX_INT;
    if (flags == 0) {
        return;
    }
    mcp2518fd_modifyRegister(MCP_CANINTF, flags, 0);
}

/*********************************************************************************************************
** Function name:           readRxTxStatus
** Descriptions:            Read RX and TX interrupt bits. Function uses status reading, but translates.
**                          result to MCP_CANINTF. With this you can check status e.g. on interrupt sr
**                          with one single call to save SPI calls. Then use checkClearRxStatus and
**                          checkClearTxStatus for testing.
*********************************************************************************************************/
byte mcp2518fd::readRxTxStatus(void) {
    byte ret = (mcp2518fd_readStatus() & (MCP_STAT_TXIF_MASK | MCP_STAT_RXIF_MASK));
    ret = (ret & MCP_STAT_TX0IF ? MCP_TX0IF : 0) |
          (ret & MCP_STAT_TX1IF ? MCP_TX1IF : 0) |
          (ret & MCP_STAT_TX2IF ? MCP_TX2IF : 0) |
          (ret & MCP_STAT_RXIF_MASK); // Rx bits happend to be same on status and MCP_CANINTF
    return ret;
}

/*********************************************************************************************************
** Function name:           checkClearRxStatus
** Descriptions:            Return first found rx CANINTF status and clears it from parameter.
**                          Note that this does not affect to chip CANINTF at all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
byte mcp2518fd::checkClearRxStatus(byte* status) {
    byte ret;

    ret = *status & MCP_RX0IF; *status &= ~MCP_RX0IF;

    if (ret == 0) {
        ret = *status & MCP_RX1IF;
        *status &= ~MCP_RX1IF;
    }

    return ret;
}

/*********************************************************************************************************
** Function name:           checkClearTxStatus
** Descriptions:            Return specified buffer of first found tx CANINTF status and clears it from parameter.
**                          Note that this does not affect to chip CANINTF at all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
byte mcp2518fd::checkClearTxStatus(byte* status, byte iTxBuf) {
    byte ret;

    if (iTxBuf < MCP_N_TXBUFFERS) { // Clear specific buffer flag
        ret = *status & txIfFlag(iTxBuf); *status &= ~txIfFlag(iTxBuf);
    } else {
        ret = 0;
        for (byte i = 0; i < MCP_N_TXBUFFERS - nReservedTx; i++) {
            ret = *status & txIfFlag(i);
            if (ret != 0) {
                *status &= ~txIfFlag(i);
                return ret;
            }
        };
    }

    return ret;
}

/*********************************************************************************************************
** Function name:           mcpPinMode
** Descriptions:            switch supported pins between HiZ, interrupt, output or input
*********************************************************************************************************/
bool mcp2518fd::mcpPinMode(const byte pin, const byte mode) {
    byte res;
    bool ret = true;

    switch (pin) {
        case MCP_RX0BF:
            switch (mode) {
                case MCP_PIN_HIZ:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B0BFE, 0);
                    break;
                case MCP_PIN_INT:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B0BFM | B0BFE, B0BFM | B0BFE);
                    break;
                case MCP_PIN_OUT:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B0BFM | B0BFE, B0BFE);
                    break;
                default:
                    #if DEBUG_EN
                    Serial.print("Invalid pin mode request\r\n");
                    #endif
                    return false;
            }
            return true;
            break;
        case MCP_RX1BF:
            switch (mode) {
                case MCP_PIN_HIZ:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B1BFE, 0);
                    break;
                case MCP_PIN_INT:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B1BFM | B1BFE, B1BFM | B1BFE);
                    break;
                case MCP_PIN_OUT:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B1BFM | B1BFE, B1BFE);
                    break;
                default:
                    #if DEBUG_EN
                    Serial.print("Invalid pin mode request\r\n");
                    #endif
                    return false;
            }
            return true;
            break;
        case MCP_TX0RTS:
            res = mcp2518fd_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                Serial.print("Entering Configuration Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B0RTSM, B0RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B0RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    Serial.print("Invalid pin mode request\r\n");
                    #endif
                    ret = false;
            }
            res = mcp2518fd_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                Serial.print("`Setting ID Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            return ret;
            break;
        case MCP_TX1RTS:
            res = mcp2518fd_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                Serial.print("Entering Configuration Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B1RTSM, B1RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B1RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    Serial.print("Invalid pin mode request\r\n");
                    #endif
                    ret = false;
            }
            res = mcp2518fd_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                Serial.print("`Setting ID Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            return ret;
            break;
        case MCP_TX2RTS:
            res = mcp2518fd_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                Serial.print("Entering Configuration Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B2RTSM, B2RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2518fd_modifyRegister(MCP_TXRTSCTRL, B2RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    Serial.print("Invalid pin mode request\r\n");
                    #endif
                    ret = false;
            }
            res = mcp2518fd_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                Serial.print("`Setting ID Mode Failure...\r\n");
                #else
                delay(10);
                #endif
                return false;
            }
            return ret;
            break;
        default:
            #if DEBUG_EN
            Serial.print("Invalid pin for mode request\r\n");
            #endif
            return false;
    }
}


/*********************************************************************************************************
** Function name:           mcpDigitalWrite
** Descriptions:            write HIGH or LOW to RX0BF/RX1BF
*********************************************************************************************************/
bool mcp2518fd::mcpDigitalWrite(const byte pin, const byte mode) {
    switch (pin) {
        case MCP_RX0BF:
            switch (mode) {
                case HIGH:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B0BFS, B0BFS);
                    return true;
                    break;
                default:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B0BFS, 0);
                    return true;
            }
            break;
        case MCP_RX1BF:
            switch (mode) {
                case HIGH:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B1BFS, B1BFS);
                    return true;
                    break;
                default:
                    mcp2518fd_modifyRegister(MCP_BFPCTRL, B1BFS, 0);
                    return true;
            }
            break;
        default:
            #if DEBUG_EN
            Serial.print("Invalid pin for mcpDigitalWrite\r\n");
            #endif
            return false;
    }
}

/*********************************************************************************************************
** Function name:           mcpDigitalRead
** Descriptions:            read HIGH or LOW from supported pins
*********************************************************************************************************/
byte mcp2518fd::mcpDigitalRead(const byte pin) {
    switch (pin) {
        case MCP_RX0BF:
            if ((mcp2518fd_readRegister(MCP_BFPCTRL) & B0BFS) > 0) {
                return HIGH;
            } else {
                return LOW;
            }
            break;
        case MCP_RX1BF:
            if ((mcp2518fd_readRegister(MCP_BFPCTRL) & B1BFS) > 0) {
                return HIGH;
            } else {
                return LOW;
            }
            break;
        case MCP_TX0RTS:
            if ((mcp2518fd_readRegister(MCP_TXRTSCTRL) & B0RTS) > 0) {
                return HIGH;
            } else {
                return LOW;
            }
            break;
        case MCP_TX1RTS:
            if ((mcp2518fd_readRegister(MCP_TXRTSCTRL) & B1RTS) > 0) {
                return HIGH;
            } else {
                return LOW;
            }
            break;
        case MCP_TX2RTS:
            if ((mcp2518fd_readRegister(MCP_TXRTSCTRL) & B2RTS) > 0) {
                return HIGH;
            } else {
                return LOW;
            }
            break;
        default:
            #if DEBUG_EN
            Serial.print("Invalid pin for mcpDigitalRead\r\n");
            #endif
            return LOW;
    }
}


/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            reset the device
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_reset(void) {
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite(MCP_RESET);
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
byte mcp2518fd::mcp2518fd_readRegister(const byte address) {
    byte ret;

    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((MCP_READ << 4) + ((address >> 8) & 0xF));
    spi_readwrite(address & 0xFF);
    spi_readwrite(0);
    ret = spi_read();
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    return ret;
}

/*********************************************************************************************************
** Function name:           mcp2518fd_readRegisterS
** Descriptions:            read registerS
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_readRegisterS(const byte address, byte values[], const byte n) {
    byte i;
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((MCP_READ << 4) + ((address >> 8) & 0xF));
    spi_readwrite(address & 0xFF);
    spi_readwrite(0);
    // mcp2518 has auto-increment of address-pointer
    for (i = 0; i < n && i < CAN_MAX_CHAR_IN_MESSAGE; i++) {
        values[i] = spi_read();
    }
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}


/*********************************************************************************************************
** Function name:           mcp2518_setRegister
** Descriptions:            set register
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_setRegister(const byte address, const byte value) {
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((MCP_WRITE << 4) + ((address >> 8) & 0xF));
    spi_readwrite(address & 0xFF);
    spi_readwrite(value);
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2518_setRegisterS
** Descriptions:            set registerS
*********************************************************************************************************/
void mcp2518fd::mcp2518fd_setRegisterS(const byte address, const byte values[], const byte n) {
    byte i;
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2518fd_SELECT();
    spi_readwrite((MCP_WRITE << 4) + ((address >> 8) & 0xF));
    spi_readwrite(address & 0xFF);

    for (i = 0; i < n; i++) {
        spi_readwrite(values[i]);
    }
    MCP2518fd_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}


/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
*********************************************************************************************************/
byte mcp2518fd::mcp2518fd_init(const byte canSpeed, const byte clock) {

    byte res;
    // Reset device
    mcp2518fd_reset();
    
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
    return res;

}