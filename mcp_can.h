/*
  mcp_can.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  Contributor: Cory J. Fowler
  2014-1-16
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/
#ifndef _MCP2515_H_
#define _MCP2515_H_

#include "mcp_can_dfs.h"

class MCP_CAN
{
    private:
        INT8U   SPICS;
        INT8U   m_mode;

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
                               const bool ext,
                               const INT32U id );

    void startSPI();
    void endSPI();

public:
    MCP_CAN(INT8U _CS, INT8U mode);
    INT8U begin(INT8U speedset);                                    /* init can                     */
    INT8U init_Mask(INT8U num, bool ext, INT32U ulData);           /* init Masks                   */
    INT8U init_Filt(INT8U num, bool ext, INT32U ulData);           /* init filters                 */
    INT8U sendMessage(const INT32U id, const bool ext, const bool rtr, const INT8U len, const INT8U *buf);   /* send buf                     */
    INT8U readMessage(const INT8U buffer_sidh_addr, INT32U *id, INT8U *dlc, INT8U buf[], bool *rtr, bool *ext);            /* read can msg                 */
    INT8U readMessage(INT32U *ID, INT8U *len, INT8U buf[], bool *rtr, bool *ext);         /* read buf with object ID      */
    INT8U checkReceive(void);                                       /* if something received        */
    INT8U checkError(void);                                         /* if something error           */
    INT8U getInterrupts(void);
    INT8U getInterruptMask(void);
    void clearInterrupts(void);
    void clearTXInterrupts(void);
    INT8U getCanStat(void);
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
