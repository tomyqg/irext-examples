//******************************************************************************
//! \file           npi_data.h
//! \brief          NPI Data structures
//
//  Revised:        $Date: 2015-01-28 17:22:05 -0800 (Wed, 28 Jan 2015) $
//  Revision:       $Revision: 42106 $
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
//******************************************************************************
#ifndef NPI_DATA_H
#define NPI_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************

#include <stdint.h>
  
#include <ti/drivers/UART.h>
#include <ti/drivers/SPI.h>

// ****************************************************************************
// defines
// ****************************************************************************  

//! \brief NPI Constants
//! 
#define NPI_MSG_SOF_VAL                         0xFE
  
//! \brief NPI Command Type.
#define NPI_MSG_TYPE_POLL                       0x00
#define NPI_MSG_TYPE_SYNCREQ                    0x01
#define NPI_MSG_TYPE_ASYNC                      0x02
#define NPI_MSG_TYPE_SYNCRSP                    0x03

//! \brief NPI Message Lengths
#define NPI_MSG_CMD_LENGTH                      2
#define NPI_MSG_LEN_LENGTH                      2
#define NPI_MSG_HDR_LENGTH                      NPI_MSG_CMD_LENGTH + \
                                                NPI_MSG_LEN_LENGTH
  
//! \brief NPI Subsystem IDs
//!
#define RPC_SYS_RES0                            0
#define RPC_SYS_SYS                             1
#define RPC_SYS_MAC                             2
#define RPC_SYS_NWK                             3
#define RPC_SYS_AF                              4
#define RPC_SYS_ZDO                             5
#define RPC_SYS_SAPI                            6
#define RPC_SYS_UTIL                            7
#define RPC_SYS_DBG                             8
#define RPC_SYS_APP                             9
#define RPC_SYS_RCAF                            10
#define RPC_SYS_RCN                             11
#define RPC_SYS_RCN_CLIENT                      12
#define RPC_SYS_BOOT                            13
#define RPC_SYS_ZIPTEST                         14
#define RPC_SYS_DEBUG                           15
#define RPC_SYS_PERIPHERALS                     16
#define RPC_SYS_NFC                             17
#define RPC_SYS_PB_NWK_MGR                      18
#define RPC_SYS_PB_GW                           19
#define RPC_SYS_PB_OTA_MGR                      20
#define RPC_SYS_BLE_SNP                         21
#define RPC_SYS_BLE_HCI                         22
#define RPC_SYS_UNDEF1                          23
#define RPC_SYS_UNDEF2                          24
#define RPC_SYS_UNDEF3                          25
#define RPC_SYS_UNDEF4                          26
#define RPC_SYS_UNDEF5                          27
#define RPC_SYS_UNDEF6                          28
#define RPC_SYS_UNDEF7                          29
#define RPC_SYS_UNDEF8                          30
#define RPC_SYS_SRV_CTRL                        31
  
//! \brief NPI Return Codes
#define NPI_SUCCESS                             0
#define NPI_ROUTING_FULL                        1
#define NPI_SS_NOT_FOUND                        2
#define NPI_INCOMPLETE_PKT                      3
#define NPI_INVALID_PKT                         4
#define NPI_BUSY                                5
#define NPI_TX_MSG_OVERSIZE                     6
#define NPI_TASK_FAILURE                        7 
#define NPI_TASK_INVALID_PARAMS                 8
  
//! \brief Reserved Subsystem ID
#define NPI_SS_RESERVED_ID                      0x00

//! \brief Reserved ICall ID
#define NPI_ICALL_RESERVED_ID                   0x00

//! \brief Masks for cmd0 bits of NPI message
#define NPI_CMD0_TYPE_MASK                      0xE0
#define NPI_CMD0_TYPE_MASK_CLR                  0x1F
#define NPI_CMD0_SS_MASK                        0x1F
#define NPI_CMD0_SS_MASK_CLR                    0xE0
  
#define NPI_SERIAL_TYPE_UART                    0  
#define NPI_SERIAL_TYPE_SPI                     1  // Not supported
  
//! \brief Returns the message type of an NPI message
#define NPI_GET_MSG_TYPE(pMsg)          ((pMsg->cmd0 & NPI_CMD0_TYPE_MASK)>> 5)

//! \brief Returns the subsystem ID of an NPI message
#define NPI_GET_SS_ID(pMsg)             ((pMsg->cmd0) & NPI_CMD0_SS_MASK) 
  
//! \brief Sets the message type of an NPI message
#define NPI_SET_MSG_TYPE(pMsg,TYPE)     pMsg->cmd0 &= NPI_CMD0_TYPE_MASK_CLR; \
                                        pMsg->cmd0 |= ( (TYPE & 0x3) << 5 );

//! \brief Sets the subsystem ID of an NPI message
#define NPI_SET_SS_ID(pMsg,SSID)        pMsg->cmd0 &= NPI_CMD0_SS_MASK_CLR; \
                                        pMsg->cmd0 |= ( (SSID & 0x1F) );

// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief Generic message NPI Frame. All messages sent over NPI Transport Layer
//         will follow this structure. Any messages passed between NPI Task and
//         subsystems will be of this type.
typedef struct _npiFrame_t
{
    uint16_t              dataLen;
    uint8_t               cmd0;
    uint8_t               cmd1;
    uint8_t               *pData;
} _npiFrame_t;

typedef union
{
  UART_Params uartParams;
  SPI_Params spiParams;                 // Not supported
} npiInterfaceParams;

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* NPI_DATA_H */

