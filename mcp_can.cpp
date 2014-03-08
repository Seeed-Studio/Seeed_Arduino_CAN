/*    
 * mcp_can.cpp
 * Library for SeeedStudio CANBUS SHIELD
 *   
 * Copyright (c) Seeedstudio   
 * Spread by SeeedStudio
 * Author     : Loovee
 * Create Time: 2012-4-24
 * Change Log :   
 *   
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.  
 */
#include "mcp_can.h"

#define spi_readwrite SPI.transfer
#define spi_read() spi_readwrite(0x00)

MCP_CAN CAN;
/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            reset the device
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/

void MCP_CAN::mcp2515_reset(void)                                      
{
    MCP2515_SELECT();
    spi_readwrite(MCP_RESET);
    MCP2515_UNSELECT();
    delay(10);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            read register
** input parameters:        address :   register address
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_readRegister(const INT8U address)                                                                     
{
    INT8U ret;

    MCP2515_SELECT();
    spi_readwrite(MCP_READ);
    spi_readwrite(address);
    ret = spi_read();
    MCP2515_UNSELECT();

    return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            read registerS
** input parameters:        address :   register address
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void MCP_CAN::mcp2515_readRegisterS(const INT8U address, 
	                            INT8U values[], 
                                    const INT8U n)
{
	INT8U i;
	MCP2515_SELECT();
	spi_readwrite(MCP_READ);
	spi_readwrite(address);
	// mcp2515 has auto-increment of address-pointer
	for (i=0; i<n; i++) {
		values[i] = spi_read();
	}
	MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            set register
** input parameters:        address :   register address
                            value   :   register value
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_setRegister(const INT8U address,                  
                                  const INT8U value)
{
    MCP2515_SELECT();
    spi_readwrite(MCP_WRITE);
    spi_readwrite(address);
    spi_readwrite(value);
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            set registerS
** input parameters:        address : first register 's address
                            values[]: value
                            n       : data length
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_setRegisterS(const INT8U address,                 
                                   const INT8U values[],
                                   const INT8U n)
{
    INT8U i;
    MCP2515_SELECT();
    spi_readwrite(MCP_WRITE);
    spi_readwrite(address);
       
    for (i=0; i<n; i++) {
        spi_readwrite(values[i]);
    }
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            set bit of one register
** input parameters:        address : register address
                            mask    : bit address
                            data    : data
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_modifyRegister(const INT8U address,               /* set bit of one register      */
                                     const INT8U mask,
                                     const INT8U data)
{
    MCP2515_SELECT();
    spi_readwrite(MCP_BITMOD);
    spi_readwrite(address);
    spi_readwrite(mask);
    spi_readwrite(data);
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            read mcp2515's Status
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          the device's status
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_readStatus(void)                             
{
	INT8U i;
	
	MCP2515_SELECT();
	spi_readwrite(MCP_READ_STATUS);
	i = spi_read();
	MCP2515_UNSELECT();
	
	return i;
}

/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            set control mode
** input parameters:        newmode : mode
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setCANCTRL_Mode(const INT8U newmode)
{
    INT8U i;

    mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);

    i = mcp2515_readRegister(MCP_CANCTRL);
    i &= MODE_MASK;

    if ( i == newmode ) {
        return MCP2515_OK;
    }
    else {
        return MCP2515_FAIL;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_configRate
** Descriptions:            set boadrate
** input parameters:        canSpeed    : boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_configRate(const INT8U canSpeed)            
{
    INT8U set, cfg1, cfg2, cfg3;
    set = 1;
    switch (canSpeed) {
        case (CAN_5KBPS):
        cfg1 = MCP_16MHz_5kBPS_CFG1;
        cfg2 = MCP_16MHz_5kBPS_CFG2;
        cfg3 = MCP_16MHz_5kBPS_CFG3;
        break;

        case (CAN_10KBPS):
        cfg1 = MCP_16MHz_10kBPS_CFG1;
        cfg2 = MCP_16MHz_10kBPS_CFG2;
        cfg3 = MCP_16MHz_10kBPS_CFG3;
        break;

        case (CAN_20KBPS):
        cfg1 = MCP_16MHz_20kBPS_CFG1;
        cfg2 = MCP_16MHz_20kBPS_CFG2;
        cfg3 = MCP_16MHz_20kBPS_CFG3;
        break;

        case (CAN_40KBPS):
        cfg1 = MCP_16MHz_40kBPS_CFG1;
        cfg2 = MCP_16MHz_40kBPS_CFG2;
        cfg3 = MCP_16MHz_40kBPS_CFG3;
        break;

        case (CAN_50KBPS):
        cfg1 = MCP_16MHz_50kBPS_CFG1;
        cfg2 = MCP_16MHz_50kBPS_CFG2;
        cfg3 = MCP_16MHz_50kBPS_CFG3;
        break;

        case (CAN_80KBPS):
        cfg1 = MCP_16MHz_80kBPS_CFG1;
        cfg2 = MCP_16MHz_80kBPS_CFG2;
        cfg3 = MCP_16MHz_80kBPS_CFG3;
        break;

        case (CAN_100KBPS):                                             /* 100KBPS                  */
        cfg1 = MCP_16MHz_100kBPS_CFG1;
        cfg2 = MCP_16MHz_100kBPS_CFG2;
        cfg3 = MCP_16MHz_100kBPS_CFG3;
        break;

        case (CAN_125KBPS):
        cfg1 = MCP_16MHz_125kBPS_CFG1;
        cfg2 = MCP_16MHz_125kBPS_CFG2;
        cfg3 = MCP_16MHz_125kBPS_CFG3;
        break;

        case (CAN_200KBPS):
        cfg1 = MCP_16MHz_200kBPS_CFG1;
        cfg2 = MCP_16MHz_200kBPS_CFG2;
        cfg3 = MCP_16MHz_200kBPS_CFG3;
        break;

        case (CAN_250KBPS):
        cfg1 = MCP_16MHz_250kBPS_CFG1;
        cfg2 = MCP_16MHz_250kBPS_CFG2;
        cfg3 = MCP_16MHz_250kBPS_CFG3;
        break;

        case (CAN_500KBPS):
        cfg1 = MCP_16MHz_500kBPS_CFG1;
        cfg2 = MCP_16MHz_500kBPS_CFG2;
        cfg3 = MCP_16MHz_500kBPS_CFG3;
        break;

        default:
        set = 0;
        break;
    }

    if (set) {
        mcp2515_setRegister(MCP_CNF1, cfg1);
        mcp2515_setRegister(MCP_CNF2, cfg2);
        mcp2515_setRegister(MCP_CNF3, cfg3);
        return MCP2515_OK;
    }
    else {
        return MCP2515_FAIL;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_initCANBuffers
** Descriptions:            init canbuffers
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void MCP_CAN::mcp2515_initCANBuffers(void)
{
    INT8U i, a1, a2, a3;
                   
    INT8U ext = 0;
    INT32U ulMask = 0x00, ulFilt = 0x00;


    mcp2515_write_id(MCP_RXM0SIDH, ext, ulMask);
    mcp2515_write_id(MCP_RXM1SIDH, ext, ulMask);
                                                                        /* Anyway, set all filters to 0 */
                                                                        /* :                            */
    mcp2515_write_id(MCP_RXF0SIDH, ext, ulFilt);                        /* RXB0: extended               */
    mcp2515_write_id(MCP_RXF1SIDH, ext, ulFilt);                        /* AND standard                 */
    mcp2515_write_id(MCP_RXF2SIDH, ext, ulFilt);                        /* RXB1: extended               */
    mcp2515_write_id(MCP_RXF3SIDH, ext, ulFilt);                        /* AND standard                 */
    mcp2515_write_id(MCP_RXF4SIDH, ext, ulFilt);
    mcp2515_write_id(MCP_RXF5SIDH, ext, ulFilt);

                                                                        /* Clear, deactivate the three  */
                                                                        /* transmit buffers             */
                                                                        /* TXBnCTRL -> TXBnD7           */
    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;
    for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
        mcp2515_setRegister(a1, 0);
        mcp2515_setRegister(a2, 0);
        mcp2515_setRegister(a3, 0);
        a1++;
        a2++;
        a3++;
    }
    mcp2515_setRegister(MCP_RXB0CTRL, 0);
    mcp2515_setRegister(MCP_RXB1CTRL, 0);
}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
** input parameters:        canSpeed    : boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_init(const INT8U canSpeed)                       /* mcp2515init                  */
{

  INT8U res;

    mcp2515_reset();

    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0)
    {
#if DEBUG_MODE
      Serial.print("Enter setting mode fall\r\n"); 
#endif
      return res;
    }
#if DEBUG_MODE
    Serial.print("Enter setting mode success \r\n");
#endif

                                                                        /* set boadrate                 */
    if(mcp2515_configRate(canSpeed))
    {
#if DEBUG_MODE
      Serial.print("set rate fall!!\r\n");
#endif
      return res;
    }
#if DEBUG_MODE
    Serial.print("set rate success!!\r\n");
#endif

    if ( res == MCP2515_OK ) {

                                                                        /* init canbuffers              */
        mcp2515_initCANBuffers();

                                                                        /* interrupt mode               */
        mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

#if (DEBUG_RXANY==1)
                                                                        /* enable both receive-buffers  */
                                                                        /* to receive any message       */
                                                                        /* and enable rollover          */
        mcp2515_modifyRegister(MCP_RXB0CTRL,
        MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
        MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
        MCP_RXB_RX_ANY);
#else
                                                                        /* enable both receive-buffers  */
                                                                        /* to receive messages          */
                                                                        /* with std. and ext. identifie */
                                                                        /* rs                           */
                                                                        /* and enable rollover          */
        mcp2515_modifyRegister(MCP_RXB0CTRL,
        MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
        MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK );
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
        MCP_RXB_RX_STDEXT);
#endif
                                                                        /* enter normal mode            */
        res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);                                                                
        if(res)
        {
#if DEBUG_MODE        
          Serial.print("Enter Normal Mode Fall!!\r\n");
#endif           
          return res;
        }


#if DEBUG_MODE
          Serial.print("Enter Normal Mode Success!!\r\n");
#endif

    }
    return res;

}

/*********************************************************************************************************
** Function name:           mcp2515_write_id
** Descriptions:            write can id
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_write_id( const INT8U mcp_addr,             
                                const INT8U ext,
                                const INT32U id )
{
    uint16_t canid;
    INT8U tbufdata[4];

    canid = (uint16_t)(id & 0x0FFFF);

    if ( ext == 1) {
        tbufdata[MCP_EID0] = (INT8U) (canid & 0xFF);
        tbufdata[MCP_EID8] = (INT8U) (canid / 256);
        canid = (uint16_t)( id / 0x10000L );
        tbufdata[MCP_SIDL] = (INT8U) (canid & 0x03);
        tbufdata[MCP_SIDL] += (INT8U) ((canid & 0x1C )*8);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (INT8U) (canid / 32 );
    }
    else {
        tbufdata[MCP_SIDH] = (INT8U) (canid / 8 );
        tbufdata[MCP_SIDL] = (INT8U) ((canid & 0x07 )<<5);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
    mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
}

/*********************************************************************************************************
** Function name:           mcp2515_read_id
** Descriptions:            read can id
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void MCP_CAN::mcp2515_read_id( const INT8U mcp_addr,                
                                     INT8U* ext,
                                     INT32U* id )
{
    INT8U tbufdata[4];

    *ext = 0;
    *id = 0;

    mcp2515_readRegisterS( mcp_addr, tbufdata, 4 );

    *id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ( (tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M ) {
                                                                        /* extended id                  */
        *id = (*id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        *id <<= 16;
        *id = *id + (unsigned)(tbufdata[MCP_EID8]<<8) + tbufdata[MCP_EID0];
        *ext = 1;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_write_canMsg
** Descriptions:            write msg
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_write_canMsg( const INT8U buffer_sidh_addr)
{
    INT8U mcp_addr;
    mcp_addr = buffer_sidh_addr;
    mcp2515_setRegisterS(mcp_addr+5, m_nDta, m_nDlc );                  /* write data bytes             */
    if ( m_nRtr == 1)  m_nDlc |= MCP_RTR_MASK;                          /* if RTR set bit in byte       */
    mcp2515_setRegister( (mcp_addr+4), m_nDlc );                        /* write the RTR and DLC        */
    mcp2515_write_id( mcp_addr, m_nExtFlg, m_nID );                 /* write CAN id                 */

}

/*********************************************************************************************************
** Function name:           mcp2515_read_canMsg
** Descriptions:            read message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void MCP_CAN::mcp2515_read_canMsg( const INT8U buffer_sidh_addr)        /* read can msg                 */
{
    INT8U mcp_addr, ctrl;

    mcp_addr = buffer_sidh_addr;

    mcp2515_read_id( mcp_addr, &m_nExtFlg,&m_nID );

    ctrl = mcp2515_readRegister( mcp_addr-1 );
    m_nDlc = mcp2515_readRegister( mcp_addr+4 );

    if ((ctrl & 0x08)) {
        m_nRtr = 1;
    }
    else {
        m_nRtr = 0;
    }

    m_nDlc &= MCP_DLC_MASK;
    mcp2515_readRegisterS( mcp_addr+5, &(m_nDta[0]), m_nDlc );
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void MCP_CAN::mcp2515_start_transmit(const INT8U mcp_addr)              /* start transmit               */
{
    mcp2515_modifyRegister( mcp_addr-1 , MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M );
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_getNextFreeTXBuf(INT8U *txbuf_n)                 /* get Next free txbuf          */
{
    INT8U res, i, ctrlval;
    INT8U ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };

    res = MCP_ALLTXBUSY;
    *txbuf_n = 0x00;

                                                                        /* check all 3 TX-Buffers       */
    for (i=0; i<MCP_N_TXBUFFERS; i++) {
        ctrlval = mcp2515_readRegister( ctrlregs[i] );
        if ( (ctrlval & MCP_TXB_TXREQ_M) == 0 ) {
            *txbuf_n = ctrlregs[i]+1;                                   /* return SIDH-address of Buffe */
                                                                        /* r                            */
            res = MCP2515_OK;
            return res;                                                 /* ! function exit              */
        }
    }
    return res;
}

/*********************************************************************************************************
** Function name:           init
** Descriptions:            init can and set speed
** input parameters:        speedset : can boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::begin(INT8U speedset)
{
    INT8U res;

    MCP2515_UNSELECT();                                                 /* disable chip select to begin with */
    pinMode(SPICS, OUTPUT);                                             /* make chip select pin an output */

    SPI.begin();
    res = mcp2515_init(speedset);
    if (res == MCP2515_OK) return CAN_OK;
    else return CAN_FAILINIT;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            init canid Masks
** input parameters:        num :   Mask number
                            ext :   if ext id
                            ulData: data
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::init_Mask(INT8U num, INT8U ext, INT32U ulData)
{
    INT8U res = MCP2515_OK;
#if DEBUG_MODE
    Serial.print("Begin to set Mask!!\r\n");
#endif
    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0){
#if DEBUG_MODE
    Serial.print("Enter setting mode fall\r\n"); 
#endif
  return res;
}
    
    if (num == 0){
        mcp2515_write_id(MCP_RXM0SIDH, ext, ulData);

    }
    else if(num == 1){
        mcp2515_write_id(MCP_RXM1SIDH, ext, ulData);
    }
    else res =  MCP2515_FAIL;
    
    res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);
    if(res > 0){
#if DEBUG_MODE
    Serial.print("Enter normal mode fall\r\n"); 
#endif
    return res;
  }
#if DEBUG_MODE
    Serial.print("set Mask success!!\r\n");
#endif
    return res;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            init canid filters
** input parameters:        num  :  Filters number
                            ext  :  if ext id
                            ulData: data
** Output parameters:       NONE
** Returned value:          ifsuccess
*********************************************************************************************************/
INT8U MCP_CAN::init_Filt(INT8U num, INT8U ext, INT32U ulData)
{
    INT8U res = MCP2515_OK;
#if DEBUG_MODE
    Serial.print("Begin to set Filter!!\r\n");
#endif
    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0)
    {
#if DEBUG_MODE
      Serial.print("Enter setting mode fall\r\n"); 
#endif
      return res;
    }
    
    switch( num )
    {
        case 0:
        mcp2515_write_id(MCP_RXF0SIDH, ext, ulData);
        break;

        case 1:
        mcp2515_write_id(MCP_RXF1SIDH, ext, ulData);
        break;

        case 2:
        mcp2515_write_id(MCP_RXF2SIDH, ext, ulData);
        break;

        case 3:
        mcp2515_write_id(MCP_RXF3SIDH, ext, ulData);
        break;

        case 4:
        mcp2515_write_id(MCP_RXF4SIDH, ext, ulData);
        break;

        case 5:
        mcp2515_write_id(MCP_RXF5SIDH, ext, ulData);
        break;

        default:
        res = MCP2515_FAIL;
    }
    
    res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);
    if(res > 0)
    {
#if DEBUG_MODE
      Serial.print("Enter normal mode fall\r\nSet filter fail!!\r\n"); 
#endif
      return res;
    }
#if DEBUG_MODE
    Serial.print("set Filter success!!\r\n");
#endif
    
    return res;
}

/*********************************************************************************************************
** Function name:           setMsg
** Descriptions:            set can message, such as dlc, id, dta[] and so on
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::setMsg(INT32U id, INT8U ext, INT8U len, INT8U *pData)
{
    int i = 0;
    m_nExtFlg = ext;
    m_nID     = id;
    m_nDlc    = len;
    for(i = 0; i<MAX_CHAR_IN_MESSAGE; i++)
    m_nDta[i] = *(pData+i);
    return MCP2515_OK;
}

/*********************************************************************************************************
** Function name:           clearMsg
** Descriptions:            set all message to zero
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::clearMsg()
{
    m_nID       = 0;
    m_nDlc      = 0;
    m_nExtFlg   = 0;
    m_nRtr      = 0;
    m_nfilhit   = 0;
    for(int i = 0; i<m_nDlc; i++ )
      m_nDta[i] = 0x00;

    return MCP2515_OK;
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::sendMsg()
{
    INT8U res, res1, txbuf_n;
    uint16_t uiTimeOut = 0;

    do {
        res = mcp2515_getNextFreeTXBuf(&txbuf_n);                       /* info = addr.                 */
        uiTimeOut++;
    } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

    if(uiTimeOut == TIMEOUTVALUE) return CAN_GETTXBFTIMEOUT;            /* get tx buff time out         */
    uiTimeOut = 0;
    mcp2515_write_canMsg( txbuf_n);
    mcp2515_start_transmit( txbuf_n );
    do
    {
        uiTimeOut++;        
        res1= CAN.mcp2515_readRegister(txbuf_n);  			/* read send buff ctrl reg 	*/
        res1 = res1 & 0x08;                               		
    }while(res1 && (uiTimeOut < TIMEOUTVALUE));   
    if(uiTimeOut == TIMEOUTVALUE) return CAN_SENDMSGTIMEOUT;            /* send msg timeout             */	
    return CAN_OK;

}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
** input parameters:        id  :   can id
                            ext :   if ext
                            len :   buf length
                            *buf:   data buf
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
INT8U MCP_CAN::sendMsgBuf(INT32U id, INT8U ext, INT8U len, INT8U *buf)
{
    setMsg(id, ext, len, buf);
    sendMsg();
}

/*********************************************************************************************************
** Function name:           readMsg
** Descriptions:            read message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::readMsg()
{
    INT8U stat, res;

    stat = mcp2515_readStatus();

    if ( stat & MCP_STAT_RX0IF ) {                                      /* Msg in Buffer 0              */
        mcp2515_read_canMsg( MCP_RXBUF_0);
        mcp2515_modifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
        res = CAN_OK;
    }
    else if ( stat & MCP_STAT_RX1IF ) {                                 /* Msg in Buffer 1              */
        mcp2515_read_canMsg( MCP_RXBUF_1);
        mcp2515_modifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
        res = CAN_OK;
    }
    else {
        res = CAN_NOMSG;
    }
    return res;
}

/*********************************************************************************************************
** Function name:           readMsgBuf
** Descriptions:            read message buf
** input parameters:        len:    data length
** Output parameters:       buf:    data buf
** Returned value:          if success
*********************************************************************************************************/
INT8U MCP_CAN::readMsgBuf(INT8U *len, INT8U buf[])
{
    readMsg();
    *len = m_nDlc;
    for(int i = 0; i<m_nDlc; i++)
    {
      buf[i] = m_nDta[i];
    }
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            check if got something
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if something
*********************************************************************************************************/
INT8U MCP_CAN::checkReceive(void)
{
    INT8U res;
    res = mcp2515_readStatus();                                         /* RXnIF in Bit 1 and 0         */
    if ( res & MCP_STAT_RXIF_MASK ) {
        return CAN_MSGAVAIL;
    }
    else {
        return CAN_NOMSG;
    }
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            if something error
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
INT8U MCP_CAN::checkError(void)
{
    INT8U eflg = mcp2515_readRegister(MCP_EFLG);

    if ( eflg & MCP_EFLG_ERRORMASK ) {
        return CAN_CTRLERROR;
    }
    else {
        return CAN_OK;
    }
}

/*********************************************************************************************************
** Function name:           getCanId
** Descriptions:            when receive something ,u can get the can id!!
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
INT32U MCP_CAN::getCanId(void)
{
  return m_nID;
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
