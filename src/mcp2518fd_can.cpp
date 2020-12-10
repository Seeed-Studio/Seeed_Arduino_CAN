#include "mcp2518fd_can.h"

uint8_t SPICS = 0;
SPIClass *pSPI;

CAN_CONFIG config;

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
#define TX_REQUEST_ID 0x300
#define TX_RESPONSE_ID 0x301
#define BUTTON_STATUS_ID 0x201
#define LED_STATUS_ID 0x200
#define PAYLOAD_ID 0x101

#define MAX_TXQUEUE_ATTEMPTS 50

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

// Maximum number of data bytes in message
#define MAX_DATA_BYTES 64

CAN_BUS_DIAGNOSTIC busDiagnostics;
uint8_t tec;
uint8_t rec;
CAN_ERROR_STATE errorFlags;

// *****************************************************************************
// *****************************************************************************
// Section: Variables

//! SPI Transmit buffer
uint8_t spiTransmitBuffer[SPI_DEFAULT_BUFFER_LENGTH + 2];

//! SPI Receive buffer
uint8_t spiReceiveBuffer[SPI_DEFAULT_BUFFER_LENGTH];

uint16_t DRV_CANFDSPI_CalculateCRC16(uint8_t *data, uint16_t size) {
  uint16_t init = CRCBASE;
  uint8_t index;

  while (size-- != 0) {
    index = ((uint8_t *)&init)[CRCUPPER] ^ *data++;
    init = (init << 8) ^ crc16_table[index];
  }

  return init;
}


/*********************************************************************************************************
** Function name:           begin
** Descriptions:            init can and set speed
*********************************************************************************************************/
byte mcp2518fd::begin(byte speedset, const byte clockset) {
  SPI.begin();
  byte res = mcp2518fd_init(speedset, clockset);

  return res;
}

/*********************************************************************************************************
** Function name:           mcp2518fd_reset
** Descriptions:            reset the device
*********************************************************************************************************/
int8_t mcp2518fd::mcp2518fd_reset(void) {
  uint16_t spiTransferSize = 2;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] = (uint8_t)(cINSTRUCTION_RESET << 4);
  spiTransmitBuffer[1] = 0;

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadByte(uint16_t address, uint8_t *rxd) {
  uint16_t spiTransferSize = 3;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
  spiTransmitBuffer[2] = 0;

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spiReceiveBuffer[2] = spi_readwrite(0x00);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);
  // Update data
  *rxd = spiReceiveBuffer[2];

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteByte(uint16_t address, uint8_t txd) {
  uint16_t spiTransferSize = 3;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
  spiTransmitBuffer[2] = txd;

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadWord(uint16_t address, uint32_t *rxd) {
  uint8_t i;
  uint32_t x;
  uint16_t spiTransferSize = 6;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 2; i < 6; i++) {
    spiReceiveBuffer[i] = spi_readwrite(0x00);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  // Update data
  *rxd = 0;
  for (i = 2; i < 6; i++) {
    x = (uint32_t)spiReceiveBuffer[i];
    *rxd += x << ((i - 2) * 8);
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteWord(uint16_t address, uint32_t txd) {
  uint8_t i;
  uint16_t spiTransferSize = 6;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

  // Split word into 4 bytes and add them to buffer
  for (i = 0; i < 4; i++) {
    spiTransmitBuffer[i + 2] = (uint8_t)((txd >> (i * 8)) & 0xFF);
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  spi_readwrite(spiTransmitBuffer[3]);
  spi_readwrite(spiTransmitBuffer[4]);
  spi_readwrite(spiTransmitBuffer[5]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadHalfWord(uint16_t address, uint16_t *rxd) {
  uint8_t i;
  uint32_t x;
  uint16_t spiTransferSize = 4;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 2; i < 4; i++) {
    spiReceiveBuffer[i] = spi_readwrite(0x00);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  // Update data
  *rxd = 0;
  for (i = 2; i < 4; i++) {
    x = (uint32_t)spiReceiveBuffer[i];
    *rxd += x << ((i - 2) * 8);
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteHalfWord(uint16_t address, uint16_t txd) {
  uint8_t i;
  uint16_t spiTransferSize = 4;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

  // Split word into 2 bytes and add them to buffer
  for (i = 0; i < 2; i++) {
    spiTransmitBuffer[i + 2] = (uint8_t)((txd >> (i * 8)) & 0xFF);
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  spi_readwrite(spiTransmitBuffer[3]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadByteArray(uint16_t address, uint8_t *rxd,
                                          uint16_t nBytes) {
  uint16_t i;
  uint16_t spiTransferSize = nBytes + 2;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

  // Clear data
  for (i = 2; i < spiTransferSize; i++) {
    spiTransmitBuffer[i] = 0;
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 0; i < nBytes; i++) {
    spiReceiveBuffer[i + 2] = spi_readwrite(0x00);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  // Update data
  for (i = 0; i < nBytes; i++) {
    rxd[i] = spiReceiveBuffer[i + 2];
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteByteArray(uint16_t address, uint8_t *txd,
                                           uint16_t nBytes) {
  uint16_t i;
  uint16_t spiTransferSize = nBytes + 2;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
  // Add data
  for (i = 2; i < spiTransferSize; i++) {
    spiTransmitBuffer[i] = txd[i - 2];
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 2; i < spiTransferSize; i++) {
    spi_readwrite(spiTransmitBuffer[i]);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteByteSafe(uint16_t address, uint8_t txd) {
  uint16_t crcResult = 0;
  uint16_t spiTransferSize = 5;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE_SAFE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
  spiTransmitBuffer[2] = txd;

  // Add CRC
  crcResult = DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, 3);
  spiTransmitBuffer[3] = (crcResult >> 8) & 0xFF;
  spiTransmitBuffer[4] = crcResult & 0xFF;

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  spi_readwrite(spiTransmitBuffer[3]);
  spi_readwrite(spiTransmitBuffer[4]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteWordSafe(uint16_t address, uint32_t txd) {
  uint8_t i;
  uint16_t crcResult = 0;
  uint16_t spiTransferSize = 8;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE_SAFE << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);

  // Split word into 4 bytes and add them to buffer
  for (i = 0; i < 4; i++) {
    spiTransmitBuffer[i + 2] = (uint8_t)((txd >> (i * 8)) & 0xFF);
  }

  // Add CRC
  crcResult = DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, 6);
  spiTransmitBuffer[6] = (crcResult >> 8) & 0xFF;
  spiTransmitBuffer[7] = crcResult & 0xFF;

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  spi_readwrite(spiTransmitBuffer[3]);
  spi_readwrite(spiTransmitBuffer[4]);
  spi_readwrite(spiTransmitBuffer[5]);
  spi_readwrite(spiTransmitBuffer[6]);
  spi_readwrite(spiTransmitBuffer[7]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadByteArrayWithCRC(uint16_t address, uint8_t *rxd,
                                                 uint16_t nBytes, bool fromRam,
                                                 bool *crcIsCorrect) {
  uint8_t i;
  uint16_t crcFromSpiSlave = 0;
  uint16_t crcAtController = 0;
  uint16_t spiTransferSize =
      nBytes + 5; // first two bytes for sending command & address, third for
                  // size, last two bytes for CRC
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_READ_CRC << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
  if (fromRam) {
    spiTransmitBuffer[2] = nBytes >> 2;
  } else {
    spiTransmitBuffer[2] = nBytes;
  }

  // Clear data
  for (i = 3; i < spiTransferSize; i++) {
    spiTransmitBuffer[i] = 0;
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  for (i = 3; i < spiTransferSize; i++) {
    spiReceiveBuffer[i] = spi_readwrite(0x00);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  // Get CRC from controller
  crcFromSpiSlave = (uint16_t)(spiReceiveBuffer[spiTransferSize - 2] << 8) +
                    (uint16_t)(spiReceiveBuffer[spiTransferSize - 1]);

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

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteByteArrayWithCRC(uint16_t address,
                                                  uint8_t *txd, uint16_t nBytes,
                                                  bool fromRam) {
  uint16_t i;
  uint16_t crcResult = 0;
  uint16_t spiTransferSize = nBytes + 5;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] =
      (uint8_t)((cINSTRUCTION_WRITE_CRC << 4) + ((address >> 8) & 0xF));
  spiTransmitBuffer[1] = (uint8_t)(address & 0xFF);
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
  crcResult =
      DRV_CANFDSPI_CalculateCRC16(spiTransmitBuffer, spiTransferSize - 2);
  spiTransmitBuffer[spiTransferSize - 2] = (uint8_t)((crcResult >> 8) & 0xFF);
  spiTransmitBuffer[spiTransferSize - 1] = (uint8_t)(crcResult & 0xFF);

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  spi_readwrite(spiTransmitBuffer[2]);
  for (i = 0; i < nBytes; i++) {
    spi_readwrite(spiTransmitBuffer[i + 3]);
  }
  spi_readwrite(spiTransmitBuffer[spiTransferSize - 2]);
  spi_readwrite(spiTransmitBuffer[spiTransferSize - 1]);
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReadWordArray(uint16_t address, uint32_t *rxd,
                                          uint16_t nWords) {
  uint16_t i, j, n;
  REG_t w;
  uint16_t spiTransferSize = nWords * 4 + 2;
  int8_t spiTransferError = 0;

  // Compose command
  spiTransmitBuffer[0] = (cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF);
  spiTransmitBuffer[1] = address & 0xFF;

  // Clear data
  for (i = 2; i < spiTransferSize; i++) {
    spiTransmitBuffer[i] = 0;
  }

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 2; i < spiTransferSize; i++) {
    // for (i = 2; i < 6; i++) {
    spiReceiveBuffer[i] = spi_readwrite(0x00);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);
  // Convert Byte array to Word array
  n = 2;
  for (i = 0; i < nWords; i++) {
    w.word = 0;
    for (j = 0; j < 4; j++, n++) {
      w.byte[j] = spiReceiveBuffer[n];
    }
    rxd[i] = w.word;
  }
  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_WriteWordArray(uint16_t address, uint32_t *txd,
                                           uint16_t nWords) {
  uint16_t i, j, n;
  REG_t w;
  uint16_t spiTransferSize = nWords * 4 + 2;
  int8_t spiTransferError = 0;

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

#ifdef SPI_HAS_TRANSACTION
  SPI_BEGIN();
#endif
  MCP2518fd_SELECT();
  spi_readwrite(spiTransmitBuffer[0]);
  spi_readwrite(spiTransmitBuffer[1]);
  for (i = 2; i < spiTransferSize; i++) {
    spi_readwrite(spiTransmitBuffer[i]);
  }
  MCP2518fd_UNSELECT();
#ifdef SPI_HAS_TRANSACTION
  SPI_END();
#endif
  delay(10);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_EccEnable() {
  int8_t spiTransferError = 0;
  uint8_t d = 0;

  // Read
  spiTransferError = mcp2518fd_ReadByte(cREGADDR_ECCCON, &d);
  if (spiTransferError) {
    return -1;
  }
  // Modify
  d |= 0x01;

  // Write
  spiTransferError = mcp2518fd_WriteByte(cREGADDR_ECCCON, d);
  if (spiTransferError) {
    return -2;
  }

  return 0;
}

int8_t mcp2518fd::mcp2518fd_RamInit(uint8_t d) {
  uint8_t txd[SPI_DEFAULT_BUFFER_LENGTH];
  uint32_t k;
  int8_t spiTransferError = 0;

  // Prepare data
  for (k = 0; k < SPI_DEFAULT_BUFFER_LENGTH; k++) {
    txd[k] = d;
  }

  uint16_t a = cRAMADDR_START;

  for (k = 0; k < (cRAM_SIZE / SPI_DEFAULT_BUFFER_LENGTH); k++) {
    spiTransferError =
        mcp2518fd_WriteByteArray(a, txd, SPI_DEFAULT_BUFFER_LENGTH);
    if (spiTransferError) {
      return -1;
    }
    a += SPI_DEFAULT_BUFFER_LENGTH;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ConfigureObjectReset(CAN_CONFIG *config) {
  REG_CiCON ciCon;
  ciCon.word = canControlResetValues[cREGADDR_CiCON / 4];

  config->DNetFilterCount = ciCon.bF.DNetFilterCount;
  config->IsoCrcEnable = ciCon.bF.IsoCrcEnable;
  config->ProtocolExpectionEventDisable =
      ciCon.bF.ProtocolExceptionEventDisable;
  config->WakeUpFilterEnable = ciCon.bF.WakeUpFilterEnable;
  config->WakeUpFilterTime = ciCon.bF.WakeUpFilterTime;
  config->BitRateSwitchDisable = ciCon.bF.BitRateSwitchDisable;
  config->RestrictReTxAttempts = ciCon.bF.RestrictReTxAttempts;
  config->EsiInGatewayMode = ciCon.bF.EsiInGatewayMode;
  config->SystemErrorToListenOnly = ciCon.bF.SystemErrorToListenOnly;
  config->StoreInTEF = ciCon.bF.StoreInTEF;
  config->TXQEnable = ciCon.bF.TXQEnable;
  config->TxBandWidthSharing = ciCon.bF.TxBandWidthSharing;

  return 0;
}

int8_t mcp2518fd::mcp2518fd_Configure(CAN_CONFIG *config) {
  REG_CiCON ciCon;
  int8_t spiTransferError = 0;

  ciCon.word = canControlResetValues[cREGADDR_CiCON / 4];

  ciCon.bF.DNetFilterCount = config->DNetFilterCount;
  ciCon.bF.IsoCrcEnable = config->IsoCrcEnable;
  ciCon.bF.ProtocolExceptionEventDisable =
      config->ProtocolExpectionEventDisable;
  ciCon.bF.WakeUpFilterEnable = config->WakeUpFilterEnable;
  ciCon.bF.WakeUpFilterTime = config->WakeUpFilterTime;
  ciCon.bF.BitRateSwitchDisable = config->BitRateSwitchDisable;
  ciCon.bF.RestrictReTxAttempts = config->RestrictReTxAttempts;
  ciCon.bF.EsiInGatewayMode = config->EsiInGatewayMode;
  ciCon.bF.SystemErrorToListenOnly = config->SystemErrorToListenOnly;
  ciCon.bF.StoreInTEF = config->StoreInTEF;
  ciCon.bF.TXQEnable = config->TXQEnable;
  ciCon.bF.TxBandWidthSharing = config->TxBandWidthSharing;

  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiCON, ciCon.word);
  if (spiTransferError) {
    return -1;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_TransmitChannelConfigureObjectReset(
    CAN_TX_FIFO_CONFIG *config) {
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = canFifoResetValues[0]; // 10010010100101010000

  config->RTREnable = ciFifoCon.txBF.RTREnable;
  config->TxPriority = ciFifoCon.txBF.TxPriority;
  config->TxAttempts = ciFifoCon.txBF.TxAttempts;
  config->FifoSize = ciFifoCon.txBF.FifoSize;
  config->PayLoadSize = ciFifoCon.txBF.PayLoadSize;

  return 0;
}

int8_t
mcp2518fd::mcp2518fd_TransmitChannelConfigure(CAN_FIFO_CHANNEL channel,
                                              CAN_TX_FIFO_CONFIG *config) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;
  // Setup FIFO
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = canFifoResetValues[0];
  ciFifoCon.txBF.TxEnable = 1;
  ciFifoCon.txBF.FifoSize = config->FifoSize;
  ciFifoCon.txBF.PayLoadSize = config->PayLoadSize;
  ciFifoCon.txBF.TxAttempts = config->TxAttempts;
  ciFifoCon.txBF.TxPriority = config->TxPriority;
  ciFifoCon.txBF.RTREnable = config->RTREnable;

  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_WriteWord(a, ciFifoCon.word);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReceiveChannelConfigureObjectReset(
    CAN_RX_FIFO_CONFIG *config) {
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = canFifoResetValues[0];

  config->FifoSize = ciFifoCon.rxBF.FifoSize;
  config->PayLoadSize = ciFifoCon.rxBF.PayLoadSize;
  config->RxTimeStampEnable = ciFifoCon.rxBF.RxTimeStampEnable;

  return 0;
}

int8_t
mcp2518fd::mcp2518fd_ReceiveChannelConfigure(CAN_FIFO_CHANNEL channel,
                                             CAN_RX_FIFO_CONFIG *config) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  if (channel == CAN_TXQUEUE_CH0) {
    return -100;
  }

  // Setup FIFO
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = canFifoResetValues[0];

  ciFifoCon.rxBF.TxEnable = 0;
  ciFifoCon.rxBF.FifoSize = config->FifoSize;
  ciFifoCon.rxBF.PayLoadSize = config->PayLoadSize;
  ciFifoCon.rxBF.RxTimeStampEnable = config->RxTimeStampEnable;

  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_WriteWord(a, ciFifoCon.word);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_FilterObjectConfigure(CAN_FILTER filter,
                                                  CAN_FILTEROBJ_ID *id) {
  uint16_t a;
  REG_CiFLTOBJ fObj;
  int8_t spiTransferError = 0;

  // Setup
  fObj.word = 0;
  fObj.bF = *id;
  a = cREGADDR_CiFLTOBJ + (filter * CiFILTER_OFFSET);

  spiTransferError = mcp2518fd_WriteWord(a, fObj.word);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_FilterMaskConfigure(CAN_FILTER filter,
                                                CAN_MASKOBJ_ID *mask) {
  uint16_t a;
  REG_CiMASK mObj;
  int8_t spiTransferError = 0;

  // Setup
  mObj.word = 0;
  mObj.bF = *mask;
  a = cREGADDR_CiMASK + (filter * CiFILTER_OFFSET);

  spiTransferError = mcp2518fd_WriteWord(a, mObj.word);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_FilterToFifoLink(CAN_FILTER filter,
                                             CAN_FIFO_CHANNEL channel,
                                             bool enable) {
  uint16_t a;
  REG_CiFLTCON_BYTE fCtrl;
  int8_t spiTransferError = 0;

  // Enable
  if (enable) {
    fCtrl.bF.Enable = 1;
  } else {
    fCtrl.bF.Enable = 0;
  }

  // Link
  fCtrl.bF.BufferPointer = channel;
  a = cREGADDR_CiFLTCON + filter;

  spiTransferError = mcp2518fd_WriteByte(a, fCtrl.byte);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_BitTimeConfigureNominal40MHz(
    MCP2518FD_BITTIME_SETUP bitTime) {
  int8_t spiTransferError = 0;
  REG_CiNBTCFG ciNbtcfg;

  ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

  // Arbitration Bit rate
  switch (bitTime) {
    // All 500K
  case CAN_500K_1M:
  case CAN_500K_2M:
  case CAN_500K_3M:
  case CAN_500K_4M:
  case CAN_500K_5M:
  case CAN_500K_6M7:
  case CAN_500K_8M:
  case CAN_500K_10M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 62;
    ciNbtcfg.bF.TSEG2 = 15;
    ciNbtcfg.bF.SJW = 15;
    break;

    // All 250K
  case CAN_250K_500K:
  case CAN_250K_833K:
  case CAN_250K_1M:
  case CAN_250K_1M5:
  case CAN_250K_2M:
  case CAN_250K_3M:
  case CAN_250K_4M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 126;
    ciNbtcfg.bF.TSEG2 = 31;
    ciNbtcfg.bF.SJW = 31;
    break;

  case CAN_1000K_4M:
  case CAN_1000K_8M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 30;
    ciNbtcfg.bF.TSEG2 = 7;
    ciNbtcfg.bF.SJW = 7;
    break;

  case CAN_125K_500K:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 254;
    ciNbtcfg.bF.TSEG2 = 63;
    ciNbtcfg.bF.SJW = 63;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiNBTCFG, ciNbtcfg.word);

  return spiTransferError;
}

int8_t
mcp2518fd::mcp2518fd_BitTimeConfigureData40MHz(MCP2518FD_BITTIME_SETUP bitTime,
                                               CAN_SSP_MODE sspMode) {
  int8_t spiTransferError = 0;
  REG_CiDBTCFG ciDbtcfg;
  REG_CiTDC ciTdc;
  //    sspMode;

  ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
  ciTdc.word = 0;

  // Configure Bit time and sample point
  ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
  uint32_t tdcValue = 0;

  // Data Bit rate and SSP
  switch (bitTime) {
  case CAN_500K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 30;
    ciDbtcfg.bF.TSEG2 = 7;
    ciDbtcfg.bF.SJW = 7;
    // SSP
    ciTdc.bF.TDCOffset = 31;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_2M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 14;
    ciDbtcfg.bF.TSEG2 = 3;
    ciDbtcfg.bF.SJW = 3;
    // SSP
    ciTdc.bF.TDCOffset = 15;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_3M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 8;
    ciDbtcfg.bF.TSEG2 = 2;
    ciDbtcfg.bF.SJW = 2;
    // SSP
    ciTdc.bF.TDCOffset = 9;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_4M:
  case CAN_1000K_4M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_5M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 4;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 5;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_6M7:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 3;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 4;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_8M:
  case CAN_1000K_8M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 2;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 3;
    ciTdc.bF.TDCValue = 1;
    break;
  case CAN_500K_10M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 1;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 2;
    ciTdc.bF.TDCValue = 0;
    break;

  case CAN_250K_500K:
  case CAN_125K_500K:
    ciDbtcfg.bF.BRP = 1;
    ciDbtcfg.bF.TSEG1 = 30;
    ciDbtcfg.bF.TSEG2 = 7;
    ciDbtcfg.bF.SJW = 7;
    // SSP
    ciTdc.bF.TDCOffset = 31;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_833K:
    ciDbtcfg.bF.BRP = 1;
    ciDbtcfg.bF.TSEG1 = 17;
    ciDbtcfg.bF.TSEG2 = 4;
    ciDbtcfg.bF.SJW = 4;
    // SSP
    ciTdc.bF.TDCOffset = 18;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 30;
    ciDbtcfg.bF.TSEG2 = 7;
    ciDbtcfg.bF.SJW = 7;
    // SSP
    ciTdc.bF.TDCOffset = 31;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_1M5:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 18;
    ciDbtcfg.bF.TSEG2 = 5;
    ciDbtcfg.bF.SJW = 5;
    // SSP
    ciTdc.bF.TDCOffset = 19;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_2M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 14;
    ciDbtcfg.bF.TSEG2 = 3;
    ciDbtcfg.bF.SJW = 3;
    // SSP
    ciTdc.bF.TDCOffset = 15;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_3M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 8;
    ciDbtcfg.bF.TSEG2 = 2;
    ciDbtcfg.bF.SJW = 2;
    // SSP
    ciTdc.bF.TDCOffset = 9;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_4M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiDBTCFG, ciDbtcfg.word);
  if (spiTransferError) {
    return -2;
  }

  // Write Transmitter Delay Compensation
#ifdef REV_A
  ciTdc.bF.TDCOffset = 0;
  ciTdc.bF.TDCValue = 0;
#endif

  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiTDC, ciTdc.word);
  if (spiTransferError) {
    return -3;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_BitTimeConfigureNominal20MHz(
    MCP2518FD_BITTIME_SETUP bitTime) {
  int8_t spiTransferError = 0;
  REG_CiNBTCFG ciNbtcfg;

  ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

  // Arbitration Bit rate
  switch (bitTime) {
    // All 500K
  case CAN_500K_1M:
  case CAN_500K_2M:
  case CAN_500K_4M:
  case CAN_500K_5M:
  case CAN_500K_6M7:
  case CAN_500K_8M:
  case CAN_500K_10M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 30;
    ciNbtcfg.bF.TSEG2 = 7;
    ciNbtcfg.bF.SJW = 7;
    break;

    // All 250K
  case CAN_250K_500K:
  case CAN_250K_833K:
  case CAN_250K_1M:
  case CAN_250K_1M5:
  case CAN_250K_2M:
  case CAN_250K_3M:
  case CAN_250K_4M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 62;
    ciNbtcfg.bF.TSEG2 = 15;
    ciNbtcfg.bF.SJW = 15;
    break;

  case CAN_1000K_4M:
  case CAN_1000K_8M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 14;
    ciNbtcfg.bF.TSEG2 = 3;
    ciNbtcfg.bF.SJW = 3;
    break;

  case CAN_125K_500K:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 126;
    ciNbtcfg.bF.TSEG2 = 31;
    ciNbtcfg.bF.SJW = 31;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiNBTCFG, ciNbtcfg.word);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t
mcp2518fd::mcp2518fd_BitTimeConfigureData20MHz(MCP2518FD_BITTIME_SETUP bitTime,
                                               CAN_SSP_MODE sspMode) {
  int8_t spiTransferError = 0;
  REG_CiDBTCFG ciDbtcfg;
  REG_CiTDC ciTdc;
  //    sspMode;

  ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
  ciTdc.word = 0;

  // Configure Bit time and sample point
  ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
  uint32_t tdcValue = 0;

  // Data Bit rate and SSP
  switch (bitTime) {
  case CAN_500K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 14;
    ciDbtcfg.bF.TSEG2 = 3;
    ciDbtcfg.bF.SJW = 3;
    // SSP
    ciTdc.bF.TDCOffset = 15;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_2M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_4M:
  case CAN_1000K_4M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 2;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 3;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_5M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 1;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 2;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_6M7:
  case CAN_500K_8M:
  case CAN_500K_10M:
  case CAN_1000K_8M:
    // qDebug("Data Bitrate not feasible with this clock!");
    return -1;
    break;

  case CAN_250K_500K:
  case CAN_125K_500K:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 30;
    ciDbtcfg.bF.TSEG2 = 7;
    ciDbtcfg.bF.SJW = 7;
    // SSP
    ciTdc.bF.TDCOffset = 31;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_833K:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 17;
    ciDbtcfg.bF.TSEG2 = 4;
    ciDbtcfg.bF.SJW = 4;
    // SSP
    ciTdc.bF.TDCOffset = 18;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 14;
    ciDbtcfg.bF.TSEG2 = 3;
    ciDbtcfg.bF.SJW = 3;
    // SSP
    ciTdc.bF.TDCOffset = 15;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_1M5:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 8;
    ciDbtcfg.bF.TSEG2 = 2;
    ciDbtcfg.bF.SJW = 2;
    // SSP
    ciTdc.bF.TDCOffset = 9;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_2M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_3M:
    // qDebug("Data Bitrate not feasible with this clock!");
    return -1;
    break;
  case CAN_250K_4M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 2;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 3;
    ciTdc.bF.TDCValue = tdcValue;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiDBTCFG, ciDbtcfg.word);
  if (spiTransferError) {
    return -2;
  }

  // Write Transmitter Delay Compensation
#ifdef REV_A
  ciTdc.bF.TDCOffset = 0;
  ciTdc.bF.TDCValue = 0;
#endif

  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiTDC, ciTdc.word);
  if (spiTransferError) {
    return -3;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_BitTimeConfigureNominal10MHz(
    MCP2518FD_BITTIME_SETUP bitTime) {
  int8_t spiTransferError = 0;
  REG_CiNBTCFG ciNbtcfg;

  ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

  // Arbitration Bit rate
  switch (bitTime) {
    // All 500K
  case CAN_500K_1M:
  case CAN_500K_2M:
  case CAN_500K_4M:
  case CAN_500K_5M:
  case CAN_500K_6M7:
  case CAN_500K_8M:
  case CAN_500K_10M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 14;
    ciNbtcfg.bF.TSEG2 = 3;
    ciNbtcfg.bF.SJW = 3;
    break;

    // All 250K
  case CAN_250K_500K:
  case CAN_250K_833K:
  case CAN_250K_1M:
  case CAN_250K_1M5:
  case CAN_250K_2M:
  case CAN_250K_3M:
  case CAN_250K_4M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 30;
    ciNbtcfg.bF.TSEG2 = 7;
    ciNbtcfg.bF.SJW = 7;
    break;

  case CAN_1000K_4M:
  case CAN_1000K_8M:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 7;
    ciNbtcfg.bF.TSEG2 = 2;
    ciNbtcfg.bF.SJW = 2;
    break;

  case CAN_125K_500K:
    ciNbtcfg.bF.BRP = 0;
    ciNbtcfg.bF.TSEG1 = 62;
    ciNbtcfg.bF.TSEG2 = 15;
    ciNbtcfg.bF.SJW = 15;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiNBTCFG, ciNbtcfg.word);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t
mcp2518fd::mcp2518fd_BitTimeConfigureData10MHz(MCP2518FD_BITTIME_SETUP bitTime,
                                               CAN_SSP_MODE sspMode) {
  int8_t spiTransferError = 0;
  REG_CiDBTCFG ciDbtcfg;
  REG_CiTDC ciTdc;
  //    sspMode;

  ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
  ciTdc.word = 0;

  // Configure Bit time and sample point
  ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
  uint32_t tdcValue = 0;

  // Data Bit rate and SSP
  switch (bitTime) {
  case CAN_500K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_2M:
    // Data BR
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 2;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 3;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_500K_4M:
  case CAN_500K_5M:
  case CAN_500K_6M7:
  case CAN_500K_8M:
  case CAN_500K_10M:
  case CAN_1000K_4M:
  case CAN_1000K_8M:
    // qDebug("Data Bitrate not feasible with this clock!");
    return -1;
    break;

  case CAN_250K_500K:
  case CAN_125K_500K:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 14;
    ciDbtcfg.bF.TSEG2 = 3;
    ciDbtcfg.bF.SJW = 3;
    // SSP
    ciTdc.bF.TDCOffset = 15;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_833K:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 7;
    ciDbtcfg.bF.TSEG2 = 2;
    ciDbtcfg.bF.SJW = 2;
    // SSP
    ciTdc.bF.TDCOffset = 8;
    ciTdc.bF.TDCValue = tdcValue;
    ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
    break;
  case CAN_250K_1M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 6;
    ciDbtcfg.bF.TSEG2 = 1;
    ciDbtcfg.bF.SJW = 1;
    // SSP
    ciTdc.bF.TDCOffset = 7;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_1M5:
    // qDebug("Data Bitrate not feasible with this clock!");
    return -1;
    break;
  case CAN_250K_2M:
    ciDbtcfg.bF.BRP = 0;
    ciDbtcfg.bF.TSEG1 = 2;
    ciDbtcfg.bF.TSEG2 = 0;
    ciDbtcfg.bF.SJW = 0;
    // SSP
    ciTdc.bF.TDCOffset = 3;
    ciTdc.bF.TDCValue = tdcValue;
    break;
  case CAN_250K_3M:
  case CAN_250K_4M:
    // qDebug("Data Bitrate not feasible with this clock!");
    return -1;
    break;

  default:
    return -1;
    break;
  }

  // Write Bit time registers
  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiDBTCFG, ciDbtcfg.word);
  if (spiTransferError) {
    return -2;
  }

  // Write Transmitter Delay Compensation
#ifdef REV_A
  ciTdc.bF.TDCOffset = 0;
  ciTdc.bF.TDCValue = 0;
#endif

  spiTransferError = mcp2518fd_WriteWord(cREGADDR_CiTDC, ciTdc.word);
  if (spiTransferError) {
    return -3;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_BitTimeConfigure(MCP2518FD_BITTIME_SETUP bitTime,
                                             CAN_SSP_MODE sspMode,
                                             CAN_SYSCLK_SPEED clk) {
  int8_t spiTransferError = 0;

  // Decode clk
  switch (clk) {
  case CAN_SYSCLK_40M:
    spiTransferError = mcp2518fd_BitTimeConfigureNominal40MHz(bitTime);
    if (spiTransferError)
      return spiTransferError;

    spiTransferError = mcp2518fd_BitTimeConfigureData40MHz(bitTime, sspMode);
    break;
  case CAN_SYSCLK_20M:
    spiTransferError = mcp2518fd_BitTimeConfigureNominal20MHz(bitTime);
    if (spiTransferError)
      return spiTransferError;

    spiTransferError = mcp2518fd_BitTimeConfigureData20MHz(bitTime, sspMode);
    break;
  case CAN_SYSCLK_10M:
    spiTransferError = mcp2518fd_BitTimeConfigureNominal10MHz(bitTime);
    if (spiTransferError)
      return spiTransferError;

    spiTransferError = mcp2518fd_BitTimeConfigureData10MHz(bitTime, sspMode);
    break;
  default:
    spiTransferError = -1;
    break;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_GpioModeConfigure(GPIO_PIN_MODE gpio0,
                                              GPIO_PIN_MODE gpio1) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read
  a = cREGADDR_IOCON + 3;
  REG_IOCON iocon;
  iocon.word = 0;

  spiTransferError = mcp2518fd_ReadByte(a, &iocon.byte[3]);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  iocon.bF.PinMode0 = gpio0;
  iocon.bF.PinMode1 = gpio1;

  // Write
  spiTransferError = mcp2518fd_WriteByte(a, iocon.byte[3]);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t
mcp2518fd::mcp2518fd_TransmitChannelEventEnable(CAN_FIFO_CHANNEL channel,
                                                CAN_TX_FIFO_EVENT flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read Interrupt Enables
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = 0;

  spiTransferError = mcp2518fd_ReadByte(a, &ciFifoCon.byte[0]);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  ciFifoCon.byte[0] |= (flags & CAN_TX_FIFO_ALL_EVENTS);

  // Write
  spiTransferError = mcp2518fd_WriteByte(a, ciFifoCon.byte[0]);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReceiveChannelEventEnable(CAN_FIFO_CHANNEL channel,
                                                      CAN_RX_FIFO_EVENT flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  if (channel == CAN_TXQUEUE_CH0)
    return -100;

  // Read Interrupt Enables
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
  REG_CiFIFOCON ciFifoCon;
  ciFifoCon.word = 0;

  spiTransferError = mcp2518fd_ReadByte(a, &ciFifoCon.byte[0]);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  ciFifoCon.byte[0] |= (flags & CAN_RX_FIFO_ALL_EVENTS);

  // Write
  spiTransferError = mcp2518fd_WriteByte(a, ciFifoCon.byte[0]);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ModuleEventEnable(CAN_MODULE_EVENT flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read Interrupt Enables
  a = cREGADDR_CiINTENABLE;
  REG_CiINTENABLE intEnables;
  intEnables.word = 0;

  spiTransferError = mcp2518fd_ReadHalfWord(a, &intEnables.word);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  intEnables.word |= (flags & CAN_ALL_EVENTS);

  // Write
  spiTransferError = mcp2518fd_WriteHalfWord(a, intEnables.word);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_OperationModeSelect(CAN_OPERATION_MODE opMode) {
  uint8_t d = 0;
  int8_t spiTransferError = 0;

  // Read
  spiTransferError = mcp2518fd_ReadByte(cREGADDR_CiCON + 3, &d);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  d &= ~0x07;
  d |= opMode;

  // Write
  spiTransferError = mcp2518fd_WriteByte(cREGADDR_CiCON + 3, d);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

CAN_OPERATION_MODE mcp2518fd::mcp2518fd_OperationModeGet() {
  uint8_t d = 0;
  CAN_OPERATION_MODE mode = CAN_INVALID_MODE;
  int8_t spiTransferError = 0;

  // Read Opmode
  spiTransferError = mcp2518fd_ReadByte(cREGADDR_CiCON + 2, &d);
  if (spiTransferError) {
    return CAN_INVALID_MODE;
  }

  // Get Opmode bits
  d = (d >> 5) & 0x7;

  // Decode Opmode
  switch (d) {
  case CAN_NORMAL_MODE:
    mode = CAN_NORMAL_MODE;
    break;
  case CAN_SLEEP_MODE:
    mode = CAN_SLEEP_MODE;
    break;
  case CAN_INTERNAL_LOOPBACK_MODE:
    mode = CAN_INTERNAL_LOOPBACK_MODE;
    break;
  case CAN_EXTERNAL_LOOPBACK_MODE:
    mode = CAN_EXTERNAL_LOOPBACK_MODE;
    break;
  case CAN_LISTEN_ONLY_MODE:
    mode = CAN_LISTEN_ONLY_MODE;
    break;
  case CAN_CONFIGURATION_MODE:
    mode = CAN_CONFIGURATION_MODE;
    break;
  case CAN_CLASSIC_MODE:
    mode = CAN_CLASSIC_MODE;
    break;
  case CAN_RESTRICTED_MODE:
    mode = CAN_RESTRICTED_MODE;
    break;
  default:
    mode = CAN_INVALID_MODE;
    break;
  }

  return mode;
}

int8_t mcp2518fd::mcp2518fd_TransmitChannelEventGet(CAN_FIFO_CHANNEL channel,
                                                    CAN_TX_FIFO_EVENT *flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read Interrupt flags
  REG_CiFIFOSTA ciFifoSta;
  ciFifoSta.word = 0;
  a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_ReadByte(a, &ciFifoSta.byte[0]);
  if (spiTransferError) {
    return -1;
  }

  // Update data
  *flags = (CAN_TX_FIFO_EVENT)(ciFifoSta.byte[0] & CAN_TX_FIFO_ALL_EVENTS);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ErrorCountStateGet(uint8_t *tec, uint8_t *rec,
                                               CAN_ERROR_STATE *flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read Error
  a = cREGADDR_CiTREC;
  REG_CiTREC ciTrec;
  ciTrec.word = 0;

  spiTransferError = mcp2518fd_ReadWord(a, &ciTrec.word);
  if (spiTransferError) {
    return -1;
  }

  // Update data
  *tec = ciTrec.byte[1];
  *rec = ciTrec.byte[0];
  *flags = (CAN_ERROR_STATE)(ciTrec.byte[2] & CAN_ERROR_ALL);

  return spiTransferError;
}

// *****************************************************************************
// *****************************************************************************
// Section: Miscellaneous
uint32_t DRV_CANFDSPI_DlcToDataBytes(CAN_DLC dlc) {
  uint32_t dataBytesInObject = 0;

  Nop();
  Nop();

  if (dlc < CAN_DLC_12) {
    dataBytesInObject = dlc;
  } else {
    switch (dlc) {
    case CAN_DLC_12:
      dataBytesInObject = 12;
      break;
    case CAN_DLC_16:
      dataBytesInObject = 16;
      break;
    case CAN_DLC_20:
      dataBytesInObject = 20;
      break;
    case CAN_DLC_24:
      dataBytesInObject = 24;
      break;
    case CAN_DLC_32:
      dataBytesInObject = 32;
      break;
    case CAN_DLC_48:
      dataBytesInObject = 48;
      break;
    case CAN_DLC_64:
      dataBytesInObject = 64;
      break;
    default:
      break;
    }
  }

  return dataBytesInObject;
}

int8_t mcp2518fd::mcp2518fd_TransmitChannelLoad(CAN_FIFO_CHANNEL channel,
                                                CAN_TX_MSGOBJ *txObj,
                                                uint8_t *txd,
                                                uint32_t txdNumBytes,
                                                bool flush) {
  uint16_t a;
  uint32_t fifoReg[3];
  uint32_t dataBytesInObject;
  REG_CiFIFOCON ciFifoCon;
  REG_CiFIFOSTA ciFifoSta;
  REG_CiFIFOUA ciFifoUa;
  int8_t spiTransferError = 0;

  // Get FIFO registers
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_ReadWordArray(a, fifoReg, 3);
  if (spiTransferError) {
    return -1;
  }

  // Check that it is a transmit buffer
  ciFifoCon.word = fifoReg[0];
  if (!ciFifoCon.txBF.TxEnable) {
    return -2;
  }

  // Check that DLC is big enough for data
  dataBytesInObject = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC)txObj->bF.ctrl.DLC);
  if (dataBytesInObject < txdNumBytes) {
    return -3;
  }

  // Get status
  ciFifoSta.word = fifoReg[1];

  // Get address
  ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
  a = 4 * ciFifoUa.bF.UserAddress;
#else
  a = ciFifoUa.bF.UserAddress;
#endif
  a += cRAMADDR_START;
  uint8_t txBuffer[MAX_MSG_SIZE];

  txBuffer[0] = txObj->byte[0]; // not using 'for' to reduce no of instructions
  txBuffer[1] = txObj->byte[1];
  txBuffer[2] = txObj->byte[2];
  txBuffer[3] = txObj->byte[3];

  txBuffer[4] = txObj->byte[4];
  txBuffer[5] = txObj->byte[5];
  txBuffer[6] = txObj->byte[6];
  txBuffer[7] = txObj->byte[7];

  uint8_t i;
  for (i = 0; i < txdNumBytes; i++) {
    txBuffer[i + 8] = txd[i];
  }

  // Make sure we write a multiple of 4 bytes to RAM
  uint16_t n = 0;
  uint8_t j = 0;

  if (txdNumBytes % 4) {
    // Need to add bytes
    n = 4 - (txdNumBytes % 4);
    i = txdNumBytes + 8;

    for (j = 0; j < n; j++) {
      txBuffer[i + 8 + j] = 0;
    }
  }
  spiTransferError = mcp2518fd_WriteByteArray(a, txBuffer, txdNumBytes + 8 + n);
  if (spiTransferError) {
    return -4;
  }

  // Set UINC and TXREQ
  spiTransferError = mcp2518fd_TransmitChannelUpdate(channel, flush);
  if (spiTransferError) {
    return -5;
  }
  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReceiveChannelEventGet(CAN_FIFO_CHANNEL channel,
                                                   CAN_RX_FIFO_EVENT *flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  if (channel == CAN_TXQUEUE_CH0)
    return -100;

  // Read Interrupt flags
  REG_CiFIFOSTA ciFifoSta;
  ciFifoSta.word = 0;
  a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_ReadByte(a, &ciFifoSta.byte[0]);
  if (spiTransferError) {
    return -1;
  }

  // Update data
  *flags = (CAN_RX_FIFO_EVENT)(ciFifoSta.byte[0] & CAN_RX_FIFO_ALL_EVENTS);
  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReceiveMessageGet(CAN_FIFO_CHANNEL channel,
                                              CAN_RX_MSGOBJ *rxObj,
                                              uint8_t *rxd, uint8_t nBytes) {
  uint8_t n = 0;
  uint8_t i = 0;
  uint16_t a;
  uint32_t fifoReg[3];
  REG_CiFIFOCON ciFifoCon;
  REG_CiFIFOSTA ciFifoSta;
  REG_CiFIFOUA ciFifoUa;
  int8_t spiTransferError = 0;

  // Get FIFO registers
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_ReadWordArray(a, fifoReg, 3);
  if (spiTransferError) {
    return -1;
  }

  // Check that it is a receive buffer
  ciFifoCon.word = fifoReg[0];
  ciFifoCon.txBF.TxEnable = 0;
  if (ciFifoCon.txBF.TxEnable) {
    return -2;
  }

  // Get Status
  ciFifoSta.word = fifoReg[1];
  // Get address
  ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
  a = 4 * ciFifoUa.bF.UserAddress;
#else
  a = ciFifoUa.bF.UserAddress;
#endif
  a += cRAMADDR_START;

  // Number of bytes to read
  n = nBytes + 8; // Add 8 header bytes

  if (ciFifoCon.rxBF.RxTimeStampEnable) {
    n += 4; // Add 4 time stamp bytes
  }

  // Make sure we read a multiple of 4 bytes from RAM
  if (n % 4) {
    n = n + 4 - (n % 4);
  }

  // Read rxObj using one access
  uint8_t ba[MAX_MSG_SIZE];

  if (n > MAX_MSG_SIZE) {
    n = MAX_MSG_SIZE;
  }

  spiTransferError = mcp2518fd_ReadByteArray(a, ba, n);
  if (spiTransferError) {
    return -3;
  }

  // Assign message header
  REG_t myReg;

  myReg.byte[0] = ba[0];
  myReg.byte[1] = ba[1];
  myReg.byte[2] = ba[2];
  myReg.byte[3] = ba[3];
  rxObj->word[0] = myReg.word;

  myReg.byte[0] = ba[4];
  myReg.byte[1] = ba[5];
  myReg.byte[2] = ba[6];
  myReg.byte[3] = ba[7];
  rxObj->word[1] = myReg.word;

  if (ciFifoCon.rxBF.RxTimeStampEnable) {
    myReg.byte[0] = ba[8];
    myReg.byte[1] = ba[9];
    myReg.byte[2] = ba[10];
    myReg.byte[3] = ba[11];
    rxObj->word[2] = myReg.word;

    // Assign message data
    for (i = 0; i < nBytes; i++) {
      rxd[i] = ba[i + 12];
    }
  } else {
    rxObj->word[2] = 0;

    // Assign message data
    for (i = 0; i < nBytes; i++) {
      rxd[i] = ba[i + 8];
    }
  }

  // UINC channel
  spiTransferError = mcp2518fd_ReceiveChannelUpdate(channel);
  if (spiTransferError) {
    return -4;
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ReceiveChannelUpdate(CAN_FIFO_CHANNEL channel) {
  uint16_t a = 0;
  REG_CiFIFOCON ciFifoCon;
  int8_t spiTransferError = 0;
  ciFifoCon.word = 0;

  // Set UINC
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) +
      1; // Byte that contains FRESET
  ciFifoCon.rxBF.UINC = 1;

  // Write byte
  spiTransferError = mcp2518fd_WriteByte(a, ciFifoCon.byte[1]);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_TransmitChannelUpdate(CAN_FIFO_CHANNEL channel,
                                                  bool flush) {
  uint16_t a;
  REG_CiFIFOCON ciFifoCon;
  int8_t spiTransferError = 0;

  // Set UINC
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) +
      1; // Byte that contains FRESET
  ciFifoCon.word = 0;
  ciFifoCon.txBF.UINC = 1;

  // Set TXREQ
  if (flush) {
    ciFifoCon.txBF.TxRequest = 1;
  }

  spiTransferError = mcp2518fd_WriteByte(a, ciFifoCon.byte[1]);
  if (spiTransferError) {
    return -1;
  }

  return spiTransferError;
}

int8_t
mcp2518fd::mcp2518fd_ReceiveChannelStatusGet(CAN_FIFO_CHANNEL channel,
                                             CAN_RX_FIFO_STATUS *status) {
  uint16_t a;
  REG_CiFIFOSTA ciFifoSta;
  int8_t spiTransferError = 0;

  // Read
  ciFifoSta.word = 0;
  a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

  spiTransferError = mcp2518fd_ReadByte(a, &ciFifoSta.byte[0]);
  if (spiTransferError) {
    return -1;
  }

  // Update data
  *status = (CAN_RX_FIFO_STATUS)(ciFifoSta.byte[0] & 0x0F);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ErrorStateGet(CAN_ERROR_STATE *flags) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read Error state
  a = cREGADDR_CiTREC + 2;
  uint8_t f = 0;

  spiTransferError = mcp2518fd_ReadByte(a, &f);
  if (spiTransferError) {
    return -1;
  }

  // Update data
  *flags = (CAN_ERROR_STATE)(f & CAN_ERROR_ALL);

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ModuleEventRxCodeGet(CAN_RXCODE *rxCode) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;
  uint8_t rxCodeByte = 0;

  // Read
  a = cREGADDR_CiVEC + 3;

  spiTransferError = mcp2518fd_ReadByte(a, &rxCodeByte);
  if (spiTransferError) {
    return -1;
  }

  // Decode data
  // 0x40 = "no interrupt" (CAN_FIFO_CIVEC_NOINTERRUPT)
  if ((rxCodeByte < CAN_RXCODE_TOTAL_CHANNELS) ||
      (rxCodeByte == CAN_RXCODE_NO_INT)) {
    *rxCode = (CAN_RXCODE)rxCodeByte;
  } else {
    *rxCode = CAN_RXCODE_RESERVED; // shouldn't get here
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_ModuleEventTxCodeGet(CAN_TXCODE *txCode) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;
  uint8_t txCodeByte = 0;

  // Read
  a = cREGADDR_CiVEC + 2;

  spiTransferError = mcp2518fd_ReadByte(a, &txCodeByte);
  if (spiTransferError) {
    return -1;
  }

  // Decode data
  // 0x40 = "no interrupt" (CAN_FIFO_CIVEC_NOINTERRUPT)
  if ((txCodeByte < CAN_TXCODE_TOTAL_CHANNELS) ||
      (txCodeByte == CAN_TXCODE_NO_INT)) {
    *txCode = (CAN_TXCODE)txCodeByte;
  } else {
    *txCode = CAN_TXCODE_RESERVED; // shouldn't get here
  }

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_TransmitChannelEventAttemptClear(CAN_FIFO_CHANNEL channel)
{
    int8_t spiTransferError = 0;
    uint16_t a = 0;

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;

    spiTransferError = mcp2518fd_ReadByte(a, &ciFifoSta.byte[0]);
    if (spiTransferError) {
        return -1;
    }

    // Modify
    ciFifoSta.byte[0] &= ~CAN_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT;

    // Write
    spiTransferError = mcp2518fd_WriteByte(a, ciFifoSta.byte[0]);
    if (spiTransferError) {
        return -2;
    }

    return spiTransferError;
}


int8_t mcp2518fd::mcp2518fd_LowPowerModeEnable() {
  int8_t spiTransferError = 0;
  uint8_t d = 0;

#ifdef MCP2517FD
  // LPM not implemented
  spiTransferError = -100;
#else
  // Read
  spiTransferError = mcp2518fd_ReadByte(cREGADDR_OSC, &d);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  d |= 0x08;

  // Write
  spiTransferError = mcp2518fd_WriteByte(cREGADDR_OSC, d);
  if (spiTransferError) {
    return -2;
  }
#endif

  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_LowPowerModeDisable() {
  int8_t spiTransferError = 0;
  uint8_t d = 0;

#ifdef MCP2517FD
  // LPM not implemented
  spiTransferError = -100;
#else
  // Read
  spiTransferError = mcp2518fd_ReadByte(cREGADDR_OSC, &d);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  d &= ~0x08;

  // Write
  spiTransferError = mcp2518fd_WriteByte(cREGADDR_OSC, d);
  if (spiTransferError) {
    return -2;
  }
#endif

  return spiTransferError;
}

void mcp2518fd::mcp2518fd_TransmitMessageQueue(void) {
  uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

  // Check if FIFO is not full
  do {
    mcp2518fd_TransmitChannelEventGet(APP_TX_FIFO, &txFlags);
    if (attempts == 0) {
      Nop();
      Nop();
      mcp2518fd_ErrorCountStateGet(&tec, &rec, &errorFlags);
      return;
    }
    attempts--;
  } while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

  // Load message and transmit
  uint8_t n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC)txObj.bF.ctrl.DLC);
  mcp2518fd_TransmitChannelLoad(APP_TX_FIFO, &txObj, txd, n, true);
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
byte mcp2518fd::mcp2518fd_sendMsgBuf(const byte *buf, byte len,
                                     unsigned long id, byte ext,
                                     bool wait_sent) {
  return mcp2518fd_sendMsg(buf, len, id, ext, wait_sent);
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
*********************************************************************************************************/
byte mcp2518fd::mcp2518fd_sendMsg(const byte *buf, byte len, unsigned long id,
                                  byte ext, bool wait_sent) {
  uint8_t n;
  int i;
  byte spiTransferError = 0;
  // Configure message data
  txObj.word[0] = 0;
  txObj.word[1] = 0;
  txObj.bF.id.SID = id;
  txObj.bF.ctrl.DLC = len;
  if (ext == 1) {
    txObj.bF.ctrl.IDE = 1;
    txObj.bF.ctrl.FDF = 1;
  }
  if (ext == 0) {
    txObj.bF.ctrl.IDE = 0;
    txObj.bF.ctrl.FDF = 0;
  }
  if (len > 8) {
    txObj.bF.ctrl.FDF = 1;
  }
  txObj.bF.ctrl.BRS = true;
  n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC)txObj.bF.ctrl.DLC);
  // Prepare data
  for (i = 0; i < n; i++) {
    txd[i] = buf[i];
  }

  mcp2518fd_TransmitMessageQueue();
  return spiTransferError;
}

int8_t mcp2518fd::mcp2518fd_receiveMsg() {
  mcp2518fd_ReceiveChannelEventGet(APP_RX_FIFO, &rxFlags);

  if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
    mcp2518fd_ReceiveMessageGet(APP_RX_FIFO, &rxObj, rxd, 8);
    for (int i = 0; i < 8; i++) {
      Serial.println(rxd[i]);
      Serial.println("\t");
    }
    Serial.println();
  }

  return 0;
}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
*********************************************************************************************************/
uint8_t mcp2518fd::mcp2518fd_init(byte speedset, const byte clock) {
  // Reset device
  mcp2518fd_reset();

  // Enable ECC and initialize RAM
  mcp2518fd_EccEnable();

  mcp2518fd_RamInit(0xff);

  // Configure device
  mcp2518fd_ConfigureObjectReset(&config);
  config.IsoCrcEnable = 1;
  config.StoreInTEF = 0;
  mcp2518fd_Configure(&config);

  // Setup TX FIFO
  mcp2518fd_TransmitChannelConfigureObjectReset(&txConfig);
  txConfig.FifoSize = 7;
  txConfig.PayLoadSize = CAN_PLSIZE_64;
  txConfig.TxPriority = 1;
  mcp2518fd_TransmitChannelConfigure(APP_TX_FIFO, &txConfig);

  // Setup RX FIFO
  mcp2518fd_ReceiveChannelConfigureObjectReset(&rxConfig);
  rxConfig.FifoSize = 15;
  rxConfig.PayLoadSize = CAN_PLSIZE_64;
  mcp2518fd_ReceiveChannelConfigure(APP_RX_FIFO, &rxConfig);

  // Setup RX Filter
  fObj.word = 0;
  fObj.bF.SID = 0;
  fObj.bF.EXIDE = 0;
  fObj.bF.EID = 0x00;

  mcp2518fd_FilterObjectConfigure(CAN_FILTER0, &fObj.bF);

  // Setup RX Mask
  mObj.word = 0;
  mObj.bF.MSID = 0;
  mObj.bF.MIDE = 0; // Only allow standard IDs
  mObj.bF.MEID = 0x0;
  mcp2518fd_FilterMaskConfigure(CAN_FILTER0, &mObj.bF);

  // Link FIFO and Filter
  mcp2518fd_FilterToFifoLink(CAN_FILTER0, APP_RX_FIFO, true);

  // Setup Bit Time
  mcp2518fd_BitTimeConfigure((MCP2518FD_BITTIME_SETUP)speedset,
                             CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

  // Setup Transmit and Receive Interrupts
  mcp2518fd_GpioModeConfigure(GPIO_MODE_INT, GPIO_MODE_INT);
#ifdef APP_USE_TX_INT
  mcp2518fd_TransmitChannelEventEnable(APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
#endif
  mcp2518fd_ReceiveChannelEventEnable(APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
  mcp2518fd_ModuleEventEnable((CAN_MODULE_EVENT)(CAN_TX_EVENT | CAN_RX_EVENT));

  // Select Normal Mode
  // mcp2518fd_OperationModeSelect(CAN_CLASSIC_MODE);
  setMode(mcpMode);

  return 0;
}

/*********************************************************************************************************
** Function name:           enableTxInterrupt
** Descriptions:            enable interrupt for all tx buffers
*********************************************************************************************************/
void mcp2518fd::enableTxInterrupt(bool enable) {
  if (enable == true)
  {
    mcp2518fd_ModuleEventEnable(CAN_TX_EVENT);
  }
  return;
}

byte mcp2518fd::init_Mask(byte num, byte ext, unsigned long ulData) {

  int8_t err;
  mcp2518fd_OperationModeSelect(CAN_CONFIGURATION_MODE);

  // Setup RX Mask
  mObj.word = 0;
  mObj.bF.MSID = ulData;
  mObj.bF.MIDE = ext; // Only allow standard IDs
  mObj.bF.MEID = 0x0;
  err = mcp2518fd_FilterMaskConfigure((CAN_FILTER)num, &mObj.bF);
  mcp2518fd_OperationModeSelect(mcpMode);

  return err;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            init canid filters
*********************************************************************************************************/
byte mcp2518fd::init_Filt(byte num, byte ext, unsigned long ulData) {
  int8_t err;
  err = mcp2518fd_OperationModeSelect(CAN_CONFIGURATION_MODE);

  // Setup RX Filter
  fObj.word = 0;
  if (ext == 0) {
    fObj.bF.SID = ulData;
    fObj.bF.EXIDE = 0; // standard identifier
    fObj.bF.EID = 0x00;
  } else if (ext == 1) {
    fObj.bF.SID = 0;
    fObj.bF.EXIDE = 1; // extended identifier
    fObj.bF.EID = ulData;
  }
  mcp2518fd_FilterObjectConfigure((CAN_FILTER)num, &fObj.bF);
  mcp2518fd_OperationModeSelect(mcpMode);
  return err;
}

/*********************************************************************************************************
** Function name:           setSleepWakeup
** Descriptions:            Enable or disable the wake up interrupt (If disabled
*the MCP2515 will not be woken up by CAN bus activity)
*********************************************************************************************************/
void mcp2518fd::setSleepWakeup(const byte enable) {
  if (enable) {
    mcp2518fd_LowPowerModeEnable();
  } else {
    mcp2518fd_LowPowerModeDisable();
  }
}

/*********************************************************************************************************
** Function name:           sleep
** Descriptions:            Put mcp2515 in sleep mode to save power
*********************************************************************************************************/
byte mcp2518fd::sleep() {
  if (getMode() != 0x01) {
    return mcp2518fd_OperationModeSelect(CAN_SLEEP_MODE);
  } else {
    return CAN_OK;
  }
}

/*********************************************************************************************************
** Function name:           wake
** Descriptions:            wake MCP2515 manually from sleep. It will come back
*in the mode it was before sleeping.
*********************************************************************************************************/
byte mcp2518fd::wake() {
  byte currMode = getMode();
  if (currMode != mcpMode) {
    return mcp2518fd_OperationModeSelect(mcpMode);
  } else {
    return CAN_OK;
  }
}

/*********************************************************************************************************
** Function name:           getMode
** Descriptions:            Returns current control mode
*********************************************************************************************************/
byte mcp2518fd::getMode() {
  byte ret;
  CAN_OPERATION_MODE mode;
  mode = mcp2518fd_OperationModeGet();
  ret = (byte)mode;
  return ret;
}

/*********************************************************************************************************
** Function name:           setMode
** Descriptions:              Sets control mode
*********************************************************************************************************/
byte mcp2518fd::setMode(const byte opMode) {
  if ((CAN_OPERATION_MODE)opMode !=
      CAN_SLEEP_MODE) { // if going to sleep, the value stored in opMode is not
                        // changed so that we can return to it later
    mcpMode = (CAN_OPERATION_MODE)opMode;
  }
  return mcp2518fd_OperationModeSelect(mcpMode);
}

/*********************************************************************************************************
** Function name:           getCanId
** Descriptions:            when receive something, you can get the can id!!
*********************************************************************************************************/
unsigned long mcp2518fd::getCanId(void) { return can_id; }

/*********************************************************************************************************
** Function name:           isRemoteRequest
** Descriptions:            when receive something, you can check if it was a
*request
*********************************************************************************************************/
byte mcp2518fd::isRemoteRequest(void) { return rtr; }

/*********************************************************************************************************
** Function name:           isExtendedFrame
** Descriptions:            did we just receive standard 11bit frame or extended
*29bit? 0 = std, 1 = ext
*********************************************************************************************************/
byte mcp2518fd::isExtendedFrame(void) { return ext_flg; }

/*********************************************************************************************************
** Function name:           readMsgBufID
** Descriptions:            Read message buf and can bus source ID according to
*status.
**                          Status has to be read with readRxTxStatus.
*********************************************************************************************************/
byte mcp2518fd::readMsgBufID(byte status, volatile unsigned long *id,
                             volatile byte *ext, volatile byte *rtrBit,
                             volatile byte *len, volatile byte *buf) {

  return mcp2518fd_readMsgBufID(len, buf);
}

/*********************************************************************************************************
** Function name:           readMsgBuf
** Descriptions:            read message buf
*********************************************************************************************************/
byte mcp2518fd::readMsgBuf(byte *len, byte buf[]) {
  return mcp2518fd_readMsgBufID(len, buf);
}

/*********************************************************************************************************
** Function name:           readMsgBufID
** Descriptions:            read message buf and can bus source ID
*********************************************************************************************************/
byte mcp2518fd::readMsgBufID(unsigned long *ID, byte *len, byte buf[]) {
  return readMsgBufID(readRxTxStatus(), ID, &ext_flg, &rtr, len, buf);
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            check if got something
*********************************************************************************************************/
byte mcp2518fd::checkReceive(void) {
  CAN_RX_FIFO_STATUS status;                                       //
  // RXnIF in Bit 1 and 0 return ((res & MCP_STAT_RXIF_MASK) ? CAN_MSGAVAIL :
  // CAN_NOMSG);
  mcp2518fd_ReceiveChannelStatusGet(APP_RX_FIFO, &status);

  byte res = (byte)(status & CAN_RX_FIFO_NOT_EMPTY_EVENT) + 2;
  return res;
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            if something error
*********************************************************************************************************/
byte mcp2518fd::checkError(void) {

  CAN_ERROR_STATE flags;
  mcp2518fd_ErrorStateGet(&flags);
  byte eflg = (byte)flags;
  return eflg;  
}

// /*********************************************************************************************************
// ** Function name:           readMsgBufID
// ** Descriptions:            Read message buf and can bus source ID according
// to status.
// **                          Status has to be read with readRxTxStatus.
// *********************************************************************************************************/
byte mcp2518fd::mcp2518fd_readMsgBufID(volatile byte *len, volatile byte *buf) {
  mcp2518fd_ReceiveMessageGet(APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);
  can_id = (unsigned long)rxObj.bF.id.SID;
  uint8_t n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC)rxObj.bF.ctrl.DLC);
  *len = n;

  for (int i = 0; i < n; i++) {
    buf[i] = rxd[i];
  }

  return 0;
}

/*********************************************************************************************************
** Function name:           trySendMsgBuf
** Descriptions:            Try to send message. There is no delays for waiting
*free buffer.
*********************************************************************************************************/
byte mcp2518fd::trySendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len,
                              const byte *buf, byte iTxBuf) {

  return CAN_OK;
}

/*********************************************************************************************************
** Function name:           clearBufferTransmitIfFlags
** Descriptions:            Clear transmit interrupt flags for specific buffer
*or for all unreserved buffers.
**                          If interrupt will be used, it is important to clear
*all flags, when there is no
**                          more data to be sent. Otherwise IRQ will newer
*change state.
*********************************************************************************************************/
void mcp2518fd::clearBufferTransmitIfFlags(byte flags) {
  mcp2518fd_TransmitChannelEventAttemptClear(APP_TX_FIFO);
  return;
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            Send message by using buffer read as free from
*CANINTF status
**                          Status has to be read with readRxTxStatus and
*filtered with checkClearTxStatus
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(byte status, unsigned long id, byte ext, byte rtrBit,
                           byte len, volatile const byte *buf) {
  return CAN_OK;
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(unsigned long id, byte ext, byte rtrBit, byte len,
                           const byte *buf, bool wait_sent) {
  return mcp2518fd_sendMsg(buf, len, id, ext, wait_sent);
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
byte mcp2518fd::sendMsgBuf(unsigned long id, byte ext, byte len,
                           const byte *buf, bool wait_sent) {
  return mcp2518fd_sendMsg(buf, len, id, ext, wait_sent);
}

/*********************************************************************************************************
** Function name:           readRxTxStatus
** Descriptions:            Read RX and TX interrupt bits. Function uses status
*reading, but translates.
**                          result to MCP_CANINTF. With this you can check
*status e.g. on interrupt sr
**                          with one single call to save SPI calls. Then use
*checkClearRxStatus and
**                          checkClearTxStatus for testing.
*********************************************************************************************************/
byte mcp2518fd::readRxTxStatus(void) {
  byte ret;
  mcp2518fd_ReceiveChannelEventGet(APP_RX_FIFO, &rxFlags);
  ret = (byte)rxFlags;
  return ret;
}

/*********************************************************************************************************
** Function name:           checkClearRxStatus
** Descriptions:            Return first found rx CANINTF status and clears it
*from parameter.
**                          Note that this does not affect to chip CANINTF at
*all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
byte mcp2518fd::checkClearRxStatus(byte *status) {
  return 1;
}

/*********************************************************************************************************
** Function name:           checkClearTxStatus
** Descriptions:            Return specified buffer of first found tx CANINTF
*status and clears it from parameter.
**                          Note that this does not affect to chip CANINTF at
*all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
byte mcp2518fd::checkClearTxStatus(byte *status, byte iTxBuf) {
  return 1;
}

/*********************************************************************************************************
** Function name:           mcpPinMode
** Descriptions:            switch supported pins between HiZ, interrupt, output
*or input
*********************************************************************************************************/
bool mcp2518fd::mcpPinMode(const byte pin, const byte mode) {
  int8_t spiTransferError = 1;
  uint16_t a = 0;

  // Read
  a = cREGADDR_IOCON + 3;
  REG_IOCON iocon;
  iocon.word = 0;

  mcp2518fd_ReadByte(a, &iocon.byte[3]);

  if (pin == GPIO_PIN_0) {
    // Modify
    iocon.bF.PinMode0 = (GPIO_PIN_MODE)mode;
  }
  if (pin == GPIO_PIN_1) {
    // Modify
    iocon.bF.PinMode1 = (GPIO_PIN_MODE)mode;
  }
  // Write
  mcp2518fd_WriteByte(a, iocon.byte[3]);

  return spiTransferError;
}

/*********************************************************************************************************
** Function name:           mcpDigitalWrite
** Descriptions:            write HIGH or LOW to RX0BF/RX1BF
*********************************************************************************************************/
bool mcp2518fd::mcpDigitalWrite(const byte pin, const byte mode) {
  int8_t spiTransferError = 0;
  uint16_t a = 0;

  // Read
  a = cREGADDR_IOCON + 1;
  REG_IOCON iocon;
  iocon.word = 0;

  spiTransferError = mcp2518fd_ReadByte(a, &iocon.byte[1]);
  if (spiTransferError) {
    return -1;
  }

  // Modify
  switch (pin) {
  case GPIO_PIN_0:
    iocon.bF.LAT0 = (GPIO_PIN_STATE)mode;
    break;
  case GPIO_PIN_1:
    iocon.bF.LAT1 = (GPIO_PIN_STATE)mode;
    break;
  default:
    return -1;
    break;
  }

  // Write
  spiTransferError = mcp2518fd_WriteByte(a, iocon.byte[1]);
  if (spiTransferError) {
    return -2;
  }

  return spiTransferError;
}

/*********************************************************************************************************
** Function name:           mcpDigitalRead
** Descriptions:            read HIGH or LOW from supported pins
*********************************************************************************************************/
byte mcp2518fd::mcpDigitalRead(const byte pin) {

  GPIO_PIN_STATE state;
  uint16_t a = 0;

  // Read
  a = cREGADDR_IOCON + 2;
  REG_IOCON iocon;
  iocon.word = 0;

  mcp2518fd_ReadByte(a, &iocon.byte[2]);

  // Update data
  switch (pin) {
  case GPIO_PIN_0:
    state = (GPIO_PIN_STATE)iocon.bF.GPIO0;
    break;
  case GPIO_PIN_1:
    state = (GPIO_PIN_STATE)iocon.bF.GPIO1;
    break;
  default:
    return -1;
    break;
  }

  byte ret = (byte)state;

  return ret;
}
