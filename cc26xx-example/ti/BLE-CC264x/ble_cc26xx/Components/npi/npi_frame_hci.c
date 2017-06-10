//*****************************************************************************
//! \file           npiframehci.c
//! \brief          This file contains the Network Processor Interface (NPI)
//!                 data frame specific function implementations for the MT
//!                 serial interface.
//
//   Revised        $Date: 2015-07-28 14:31:44 -0700 (Tue, 28 Jul 2015) $
//   Revision:      $Revision: 44419 $
//
//  Copyright 2015 Texas Instruments Incorporated. All rights reserved.
//
// IMPORTANT: Your use of this Software is limited to those specific rights
// granted under the terms of a software license agreement between the user
// who downloaded the software, his/her employer (which must be your employer)
// and Texas Instruments Incorporated (the "License").  You may not use this
// Software unless you agree to abide by the terms of the License. The License
// limits your use, and you acknowledge, that the Software may not be modified,
// copied or distributed unless used solely and exclusively in conjunction with
// a Texas Instruments radio frequency device, which is integrated into
// your product.  Other than for the foregoing purpose, you may not use,
// reproduce, copy, prepare derivative works of, modify, distribute, perform,
// display or sell this Software and/or its documentation for any purpose.
//
//  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
//  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,l
//  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
//  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
//  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
//  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
//  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
//  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
//  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
//  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
//  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
//
//  Should you have any questions regarding your right to use this Software,
//  contact Texas Instruments Incorporated at www.TI.com.
//*****************************************************************************

// ****************************************************************************
// includes
// ****************************************************************************

#include <string.h>
#include <xdc/std.h>

#include "inc/npi_rxbuf.h"
#include "inc/npi_frame.h"

#include "OSAL.h"
#include "ICall.h"
#include "Board.h"
#include "hal_types.h"
#include "hci_tl.h"

#include "inc/npi_ble.h"

// ****************************************************************************
// defines
// ****************************************************************************

#define NPIFRAMEHCI_CMD_PKT_HDR_LEN             4
#define NPIFRAMEHCI_DATA_PKT_HDR_LEN            5
#define NPIFRAMEHCI_SPNP_CMD_PKT_HDR_LEN        5

// States for Command and Data packet parser
#define NPIFRAMEHCI_STATE_PKT_TYPE              0
//
#define NPIFRAMEHCI_CMD_STATE_OPCODE0           1
#define NPIFRAMEHCI_CMD_STATE_OPCODE1           2
#define NPIFRAMEHCI_CMD_STATE_LENGTH            3
#define NPIFRAMEHCI_CMD_STATE_DATA              4
//
#define NPIFRAMEHCI_DATA_STATE_HANDLE0          5
#define NPIFRAMEHCI_DATA_STATE_HANDLE1          6
#define NPIFRAMEHCI_DATA_STATE_LENGTH0          7
#define NPIFRAMEHCI_DATA_STATE_LENGTH1          8
#define NPIFRAMEHCI_DATA_STATE_DATA             9
//
#define NPIFRAMEHCI_STATE_FLUSH                 10

#define NPIFRAMEHCI_SPNP_CMD_STATE_OPCODE0      10
#define NPIFRAMEHCI_SPNP_CMD_STATE_OPCODE1      11
#define NPIFRAMEHCI_SPNP_CMD_STATE_LENGTH0      12
#define NPIFRAMEHCI_SPNP_CMD_STATE_LENGTH1      13
#define NPIFRAMEHCI_SPNP_CMD_STATE_DATA         14

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************
npiIncomingFrameCBack_t incomingFrameCBFunc = NULL;

//! \name HCI serial collection globals
//@{
static uint8_t state = NPIFRAMEHCI_STATE_PKT_TYPE;
static uint8_t PKT_Token = 0;
static uint16_t LEN_Token = 0;
static uint16_t OPCODE_Token = 0;
static uint16_t HANDLE_Token = 0;
static uint16_t bytesToFlush = 0;
static hciPacket_t *pCmdMsg = NULL;
static hciDataPacket_t *pDataMsg = NULL;
static npiPkt_t *pFromStackMsg = NULL;

#if defined(__TI_COMPILER_VERSION) || defined(__TI_COMPILER_VERSION__)
#pragma NOINIT(tempDataLen)
static uint8_t tempDataLen;
#elif defined (__IAR_SYSTEMS_ICC__)
static __no_init uint8_t tempDataLen;
#endif
//@}

extern ICall_EntityID npiAppEntityID;

//*****************************************************************************
// function prototypes
//*****************************************************************************

// ----------------------------------------------------------------------------
//! \brief      Initialize Frame module with NPI callbacks.
//!
//! \param[in]  incomingFrameCB   Call back for complete inbound (from host)
//!                               messages
//!
//! \return     void
// ----------------------------------------------------------------------------
void NPIFrame_initialize(npiIncomingFrameCBack_t incomingFrameCB)
{
    // Send BLE Stack the NPI Task Entity ID
    NPI_RegisterTask(npiAppEntityID);

    incomingFrameCBFunc = incomingFrameCB;
}

// ----------------------------------------------------------------------------
//! \brief      Bundles message into HCI Transport Layer frame.  When complete,
//!             this function should return the framed message buffer via the
//!             callback function above: npiFrameCompleteCBack_t.
//!
//!             NOTE: The passed in container pMsg is used to return the framed
//!             message and should not be free'd!
//!
//! \param[in]  pData     Pointer to NPI container w/HCI message buffer.
//!
//! \return     void
// ----------------------------------------------------------------------------
NPIMSG_msg_t *NPIFrame_frameMsg(uint8 *pMsg)
{
    // Cast generic array into appropriate container
    // All messages we receive from stack will be of type npiPacket_t
    pFromStackMsg = (npiPkt_t *)pMsg;
    
    // Alloc NPI Message
    NPIMSG_msg_t *npiMsg = (NPIMSG_msg_t *)ICall_malloc( sizeof(NPIMSG_msg_t));
    
    if ( npiMsg )
    {
      // All HCI messages are Asynchronous
      npiMsg->msgType = NPIMSG_Type_ASYNC;
      
      // Allocate memory for container, and pkt payload
      npiMsg->pBuf = (uint8 *)ICall_allocMsg(pFromStackMsg->pktLen);
      npiMsg->pBufSize = pFromStackMsg->pktLen;
      
      if (npiMsg->pBuf)
      {
          // Payload
          memcpy(npiMsg->pBuf,pFromStackMsg->pData,pFromStackMsg->pktLen);
      }
    }

    // Free original message and send new NPIMSG to a TX Queue
    ICall_freeMsg(pMsg);
    return npiMsg;
}

// ----------------------------------------------------------------------------
//! \brief  HCI command packet format:
//!         Packet Type + Command opcode + length + command payload
//!         | 1 octet   |      2         |   1   |      n        |
//!
//!         HCI data packet format:
//!         Packet Type +   Conn Handle  + length + data payload
//!         | 1 octet   |      2         |   2   |      n      |
//!
//! \return     void
// ----------------------------------------------------------------------------
void NPIFrame_collectFrameData(void)
{
    uint16_t rxBufLen = 0;
    uint8_t ch;

    while (NPIRxBuf_GetRxBufCount())
    {
        NPIRxBuf_ReadFromRxBuf(&ch, 1);
        switch (state)
        {
            // Packet Type
            case NPIFRAMEHCI_STATE_PKT_TYPE:
                PKT_Token = ch;
                switch (ch)
                {
                    case HCI_CMD_PACKET:
                        state = NPIFRAMEHCI_CMD_STATE_OPCODE0;
                        break;
                    case HCI_ACL_DATA_PACKET:
                    case HCI_SCO_DATA_PACKET:
                        state = NPIFRAMEHCI_DATA_STATE_HANDLE0;
                        break;
                    default:
                        // If we do not receive a legal packet type we will 
			            // skip this byte.
                        state = NPIFRAMEHCI_STATE_PKT_TYPE;
                }
                break;

            // Command Opcode Byte 0
            case NPIFRAMEHCI_CMD_STATE_OPCODE0:
                OPCODE_Token = ch;
                state = NPIFRAMEHCI_CMD_STATE_OPCODE1;
                break;

            // Command Opcode Byte 1
            case NPIFRAMEHCI_CMD_STATE_OPCODE1:
                OPCODE_Token |= ((uint16)ch << 8);
                state = NPIFRAMEHCI_CMD_STATE_LENGTH;
                break;

            // Command Payload Length
            case NPIFRAMEHCI_CMD_STATE_LENGTH:
                LEN_Token = (uint16)ch;
                tempDataLen = 0;
                
                if (LEN_Token > NPI_TL_BUF_SIZE)
                {
                    // Length of packet is greater than RX Buf size. Flush packet
                    bytesToFlush = LEN_Token;
                    state = NPIFRAMEHCI_STATE_FLUSH;
                }
                else
                {
                    /* Allocate memory for the data */
                    pCmdMsg = (hciPacket_t *) ICall_allocMsg(sizeof(hciPacket_t) + 
                                              NPIFRAMEHCI_CMD_PKT_HDR_LEN + LEN_Token);

                    if (pCmdMsg)
                    {
                        // Set pData to the first byte after the hciPacket bytes
                        pCmdMsg->pData = (uint8 *)pCmdMsg + sizeof(hciPacket_t);

                        pCmdMsg->pData[0] = PKT_Token;
                        pCmdMsg->pData[1] = ((uint8 *)&OPCODE_Token)[0];
                        pCmdMsg->pData[2] = ((uint8 *)&OPCODE_Token)[1];
                        pCmdMsg->pData[3] = LEN_Token;

                        // set header specific fields
                        pCmdMsg->hdr.status = 0xFF;

                        // check if a Controller Link Layer VS command
                        if (((OPCODE_Token >> 10) == VENDOR_SPECIFIC_OGF) &&
                            (((OPCODE_Token >> 7) & 0x07) != 
                            HCI_OPCODE_CSG_LINK_LAYER))
                        {
                            // this is a vendor specific command
                            pCmdMsg->hdr.event = HCI_EXT_CMD_EVENT;

                            // so strip the OGF (i.e. the most significant 6 
                            // bits of the opcode)
                            pCmdMsg->pData[2] &= 0x03;
                        }
                        else // specification specific command
                        {
                            // this is a normal host-to-controller event
                            pCmdMsg->hdr.event = HCI_HOST_TO_CTRL_CMD_EVENT;
                        }


                        state = NPIFRAMEHCI_CMD_STATE_DATA;
                    }

                    if (LEN_Token == 0)
                    {
                        // No Payload to read so go ahead and send to Stack
                        if (incomingFrameCBFunc)
                        {
                            incomingFrameCBFunc(sizeof(hciPacket_t) + NPIFRAMEHCI_CMD_PKT_HDR_LEN, 
                                                (uint8 *)pCmdMsg, NPIMSG_Type_ASYNC);
                        }

                        state = NPIFRAMEHCI_STATE_PKT_TYPE;
                    }
                }
                break;

            // Command Payload
            case NPIFRAMEHCI_CMD_STATE_DATA:
                pCmdMsg->pData[NPIFRAMEHCI_CMD_PKT_HDR_LEN + tempDataLen++] = ch;

                rxBufLen = NPIRxBuf_GetRxBufCount();

                /* If the remain of the data is there, read them all, 
				   otherwise, just read enough */
                if (rxBufLen <= LEN_Token - tempDataLen)
                {
                    NPIRxBuf_ReadFromRxBuf(&pCmdMsg->pData[NPIFRAMEHCI_CMD_PKT_HDR_LEN + tempDataLen], 
                                           rxBufLen);
                    
                    tempDataLen += rxBufLen;
                }
                else
                {
                    NPIRxBuf_ReadFromRxBuf(&pCmdMsg->pData[NPIFRAMEHCI_CMD_PKT_HDR_LEN + tempDataLen], 
                                           LEN_Token - tempDataLen);
                    
                    tempDataLen += (LEN_Token - tempDataLen);
                }

                /* If number of bytes read is equal to data length, 
				   send msg back to NPI */
                if (tempDataLen == LEN_Token)
                {
                    if (incomingFrameCBFunc)
                    {
                        incomingFrameCBFunc(sizeof(hciPacket_t) + NPIFRAMEHCI_CMD_PKT_HDR_LEN + LEN_Token, 
                                            (uint8 *)pCmdMsg, NPIMSG_Type_ASYNC);
                    }

                    state = NPIFRAMEHCI_STATE_PKT_TYPE;
                }
                break;

            // Data Handle Byte 0
            case NPIFRAMEHCI_DATA_STATE_HANDLE0:
                HANDLE_Token = ch;
                state = NPIFRAMEHCI_DATA_STATE_HANDLE1;
                break;

            // Data Handle Byte 1
            case NPIFRAMEHCI_DATA_STATE_HANDLE1:
                HANDLE_Token |= ((uint16)ch << 8);
                state = NPIFRAMEHCI_DATA_STATE_LENGTH0;
                break;

            // Data Len Byte 0
            case NPIFRAMEHCI_DATA_STATE_LENGTH0:
                LEN_Token = ch;
                state = NPIFRAMEHCI_DATA_STATE_LENGTH1;
                break;

            // Data Len Byte 1
            case NPIFRAMEHCI_DATA_STATE_LENGTH1:
                LEN_Token |= ((uint16)ch << 8);
                tempDataLen = 0;

                if (LEN_Token > NPI_TL_BUF_SIZE)
                {
                    // Length of packet is greater than RX Buf size. Flush packet
                    bytesToFlush = LEN_Token;
                    state = NPIFRAMEHCI_STATE_FLUSH;
                }
                else 
                {
                    /* Allocate memory for the data */
                    pDataMsg = (hciDataPacket_t *) ICall_allocMsg(sizeof(hciDataPacket_t) + LEN_Token);

                    if (pDataMsg)
                    {
                        pDataMsg->hdr.event = HCI_HOST_TO_CTRL_DATA_EVENT;
                        pDataMsg->hdr.status = 0xFF;

                        pDataMsg->pktType = PKT_Token;
                        // mask out PB and BC Flags
                        pDataMsg->connHandle = HANDLE_Token & 0x0FFF; 
                        //isolate PB Flag
                        pDataMsg->pbFlag = (HANDLE_Token & 0x3000) >> 12; 
                        pDataMsg->pktLen = LEN_Token;

                        // Set pData to the first byte after the hciPacket bytes
                        pDataMsg->pData = (uint8 *)pDataMsg + sizeof(hciDataPacket_t);

                        state = NPIFRAMEHCI_DATA_STATE_DATA;
                    }
                    else
                    {
                        state = NPIFRAMEHCI_STATE_PKT_TYPE;
                        return;
                    }

                    if (LEN_Token == 0)
                    {
                        if (incomingFrameCBFunc)
                        {
                            incomingFrameCBFunc(sizeof(hciDataPacket_t), 
                                                (uint8 *)pDataMsg, NPIMSG_Type_ASYNC);
                        }

                        state = NPIFRAMEHCI_STATE_PKT_TYPE;
                    }
                }
                break;

            // Data Payload
            case NPIFRAMEHCI_DATA_STATE_DATA:
                pDataMsg->pData[tempDataLen++] = ch;

                rxBufLen = NPIRxBuf_GetRxBufCount();

                /* If the remain of the data is there, read them all, 
				   otherwise, just read enough */
                if (rxBufLen <= LEN_Token - tempDataLen)
                {
                    NPIRxBuf_ReadFromRxBuf(&pDataMsg->pData[tempDataLen], 
                                           rxBufLen);
                    
                    tempDataLen += rxBufLen;
                }
                else
                {
                    NPIRxBuf_ReadFromRxBuf(&pDataMsg->pData[tempDataLen], 
                                           LEN_Token - tempDataLen);
                    
                    tempDataLen += (LEN_Token - tempDataLen);
                }

                /* If number of bytes read is equal to data length, 
				   send msg back to NPI */
                if (tempDataLen == LEN_Token)
                {
                    if (incomingFrameCBFunc)
                    {
                        incomingFrameCBFunc(sizeof(hciDataPacket_t) + LEN_Token, 
                                            (uint8 *)pDataMsg, NPIMSG_Type_ASYNC);
                    }

                    state = NPIFRAMEHCI_STATE_PKT_TYPE;
                }
                break;
            case NPIFRAMEHCI_STATE_FLUSH:
                bytesToFlush--;
                
                if (bytesToFlush == 0)
                {
                    // Flushed all bytes in frame. Return to initial state to
                    // parse next frame
                    state = NPIFRAMEHCI_STATE_PKT_TYPE;
                }
                
                break;
            default:
                break;
        }
    }
}

