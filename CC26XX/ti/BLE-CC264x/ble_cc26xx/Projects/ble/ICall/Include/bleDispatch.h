/**************************************************************************************************
  Filename:       bleDispatch.h
  Revised:        $Date: 2015-05-14 15:24:30 -0700 (Thu, 14 May 2015) $
  Revision:       $Revision: 43790 $

  Description:    ICall BLE Dispatcher Definitions.

  Copyright 2013 - 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef ICALLBLE_H
#define ICALLBLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
// Dispatch Subgroup IDs (0x00-0x80)
#define DISPATCH_GENERAL                      0x00 // General
#define DISPATCH_GAP_PROFILE                  0x01 // GAP Profile
#define DISPATCH_GATT_PROFILE                 0x02 // GATT Profile
#define DISPATCH_GAP_GATT_SERV                0x03 // GAP GATT Server (GGS)
#define DISPATCH_GATT_SERV_APP                0x04 // GATT Server App (GSA)
  
// Common Command IDs reserved for each profile (0x00-0x0F)
#define DISPATCH_PROFILE_ADD_SERVICE          0x00 // Add service
#define DISPATCH_PROFILE_DEL_SERVICE          0x01 // Delete service
#define DISPATCH_PROFILE_REG_SERVICE          0x02 // Register service
#define DISPATCH_PROFILE_DEREG_SERVICE        0x03 // Deregister service
#define DISPATCH_PROFILE_REG_CB               0x04 // Register app callbacks
#define DISPATCH_PROFILE_GET_PARAM            0x05 // Get Parameter
#define DISPATCH_PROFILE_SET_PARAM            0x06 // Set Parameter

// GAP Profile Command IDs (0x10-0xFF)
#define DISPATCH_GAP_REG_FOR_MSG              0x10 // Register for HCI messages
#define DISPATCH_GAP_LINKDB_STATE             0x11 // Link DB State
#define DISPATCH_GAP_LINKDB_NUM_CONNS         0x12 // Link DB Num Conns
#define DISPATCH_GAP_BOND_PASSCODE_RSP        0x13 // Bond Mgr Passcode Response
#define DISPATCH_GAP_BOND_LINK_EST            0x14 // Bond Mgr Link Established
#define DISPATCH_GAP_BOND_LINK_TERM           0x15 // Bond Mgr Link Terminated
#define DISPATCH_GAP_BOND_SLAVE_REQ_SEC       0x16 // Bond Mgr Slave Requested Security
#define DISPATCH_GAP_BOND_RESOLVE_ADDR        0x17 // Bond Mgr Resolve Address

// GATT Profile Command IDs (0x10-0xFF)
#define DISPATCH_GATT_REG_FOR_MSG             0x10 // Register for GATT events/messages
#define DISPATCH_GATT_INIT_CLIENT             0x11 // Initialize GATT Client
#define DISPATCH_GATT_REG_4_IND               0x12 // Register for Indication/Notification
#define DISPATCH_GATT_HTA_FLOW_CTRL           0x13 // Host To App Flow Control
#define DISPATCH_GATT_APP_COMPL_MSG           0x14 // App Completed Message
#define DISPATCH_GATT_SEND_RSP                0x15 // GATT Send Response message

// GATT Server App (GSA) Command IDs (0x10-0xFF)
#define DISPATCH_GSA_ADD_QUAL_SERVICE         0x10 // GSA Add Qualification Services
#define DISPATCH_GSA_ADD_TEST_SERVICE         0x11 // GSA Add Test Services
#define DISPATCH_GSA_SERVICE_CHANGE_IND       0x12 // GSA Service Change Indication
   
// ICall Dispatcher General Command IDs (0x10-0xFF)
#define DISPATCH_GENERAL_REG_NPI              0x10 // Register NPI task with stack

/*** Build Revision Command ***/

// Stack Info field bitmaps
#define BLDREV_STK_IAR_PROJ                   0x01 // IAR used to build stack project
#define BLDREV_STK_CCS_PROJ                   0x02 // CCS used to build stack project
#define BLDREV_STK_IAR_LIB                    0x10 // IAR used to build stack library
#define BLDREV_STK_ROM_BLD                    0x80 // ROM build

// Controller Info field bitmaps
#define BLDREV_CTRL_PING_CFG                  0x10 // Ping included
#define BLDREV_CTRL_SLV_FEAT_EXCHG_CFG        0x20 // Slave Feature Exchange included
#define BLDREV_CTRL_CONN_PARAM_REQ_CFG        0x40 // Connection Parameter Request included

// Host Info field bitmaps
#define BLDREV_HOST_GAP_BOND_MGR              0x10 // GAP Bond Manager included
#define BLDREV_HOST_L2CAP_CO_CHANNELS         0x20 // L2CAP CO Channels included

// BM Message Types by layer/module
#define BM_MSG_GATT                           1 // GATT layer
#define BM_MSG_L2CAP                          2 // L2CAP layer
#define BM_MSG_GENERIC                        3 // Lowest layer

/*******************************************************************************
 * TYPEDEFS
 */

// BM allocator and de-allocator function pointer types
typedef void* (*pfnBMAlloc_t)(uint8_t type, uint16_t size, uint16_t connHandle,
                              uint8_t opcode, uint16_t *pSizeAlloc);
typedef void  (*pfnBMFree_t)(uint8_t type, void *pMsg, uint8_t opcode);

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the task
 */
extern void bleDispatch_Init(uint8_t task_id);

/*
 * Task Event Processor for the task
 */
extern uint16_t bleDispatch_ProcessEvent(uint8_t task_id, uint16_t events);

/*
 * Implementation of the BM allocator functionality.
 */
extern void *bleDispatch_BMAlloc(uint8_t type, uint16_t size,
                                 uint16_t connHandle, uint8_t opcode,
                                 uint16_t *pSizeAlloc);
/*
 * Implementation of the BM de-allocator functionality.
 */
extern void bleDispatch_BMFree(uint8_t type, void *pBuf, uint8_t opcode);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ICALLBLE_H */
