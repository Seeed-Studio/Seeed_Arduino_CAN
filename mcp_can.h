/*
  mcp_can.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2014-1-16
  
  Contributor: 
  
  Cory J. Fowler
  Latonita
  Woodward1
  Mehtajaghvi
  BykeBlast
  TheRo0T
  Tsipizic
  ralfEdmund
  Nathancheek
  BlueAndi
  Adlerweb
  Btetz
  Hurvajs
  
  The MIT License (MIT)

  Copyright (c) 2013 Seeed Technology Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#ifndef _MCP2515_H_
#define _MCP2515_H_

#include "mcp_can_dfs.h"

#define MAX_CHAR_IN_MESSAGE 8

class MCP_CAN
{
    private:
    
    INT8U   m_nExtFlg;                                                  /* identifier xxxID             */
                                                                        /* either extended (the 29 LSB) */
                                                                        /* or standard (the 11 LSB)     */
    INT32U  m_nID;                                                      /* can id                       */
    INT8U   m_nDlc;                                                     /* data length:                 */
    INT8U   m_nDta[MAX_CHAR_IN_MESSAGE];                            	/* data                         */
    INT8U   m_nRtr;                                                     /* rtr                          */
    INT8U   m_nfilhit;
    INT8U   SPICS;

/*
*  mcp2515 driver function 
*/
   // private:
private:

    void mcp2515_reset(void);                                           /* reset mcp2515                */

    INT8U mcp2515_readRegister(const INT8U address);                    /* read mcp2515's register      */
    
    void mcp2515_readRegisterS(const INT8U address, 
	                       INT8U values[], 
                               const INT8U n);
    void mcp2515_setRegister(const INT8U address,                       /* set mcp2515's register       */
                             const INT8U value);

    void mcp2515_setRegisterS(const INT8U address,                      /* set mcp2515's registers      */
                              const INT8U values[],
                              const INT8U n);
    
    void mcp2515_initCANBuffers(void);
    
    void mcp2515_modifyRegister(const INT8U address,                    /* set bit of one register      */
                                const INT8U mask,
                                const INT8U data);

    INT8U mcp2515_readStatus(void);                                     /* read mcp2515's Status        */
    INT8U mcp2515_setCANCTRL_Mode(const INT8U newmode);                 /* set mode                     */
    INT8U mcp2515_configRate(const INT8U canSpeed);                     /* set boadrate                 */
    INT8U mcp2515_init(const INT8U canSpeed);                           /* mcp2515init                  */

    void mcp2515_write_id( const INT8U mcp_addr,                        /* write can id                 */
                               const INT8U ext,
                               const INT32U id );

    void mcp2515_read_id( const INT8U mcp_addr,                         /* read can id                  */
                                    INT8U* ext,
                                    INT32U* id );

    void mcp2515_write_canMsg( const INT8U buffer_sidh_addr );          /* write can msg                */
    void mcp2515_read_canMsg( const INT8U buffer_sidh_addr);            /* read can msg                 */
    void mcp2515_start_transmit(const INT8U mcp_addr);                  /* start transmit               */
    INT8U mcp2515_getNextFreeTXBuf(INT8U *txbuf_n);                     /* get Next free txbuf          */

/*
*  can operator function
*/    

    INT8U setMsg(INT32U id, INT8U ext, INT8U len, INT8U rtr, INT8U *pData); /* set message                  */  
    INT8U setMsg(INT32U id, INT8U ext, INT8U len, INT8U *pData); /* set message                  */  
    INT8U clearMsg();                                               /* clear all message to zero    */
    INT8U readMsg();                                                /* read message                 */
    INT8U sendMsg();                                                /* send message                 */

public:
    MCP_CAN(INT8U _CS);
    INT8U begin(INT8U speedset);                                    /* init can                     */
    INT8U init_Mask(INT8U num, INT8U ext, INT32U ulData);           /* init Masks                   */
    INT8U init_Filt(INT8U num, INT8U ext, INT32U ulData);           /* init filters                 */
    INT8U sendMsgBuf(INT32U id, INT8U ext, INT8U rtr, INT8U len, INT8U *buf);   /* send buf                     */
    INT8U sendMsgBuf(INT32U id, INT8U ext, INT8U len, INT8U *buf);   /* send buf                     */
    INT8U readMsgBuf(INT8U *len, INT8U *buf);                       /* read buf                     */
    INT8U readMsgBufID(INT32U *ID, INT8U *len, INT8U *buf);         /* read buf with object ID      */
    INT8U checkReceive(void);                                       /* if something received        */
    INT8U checkError(void);                                         /* if something error           */
    INT32U getCanId(void);                                          /* get can id when receive      */
    INT8U isRemoteRequest(void);                                    /* get RR flag when receive     */
    INT8U isExtendedFrame(void);                                    /* did we recieve 29bit frame?  */
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
