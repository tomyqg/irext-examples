/**************************************************************************************************
  Filename:       bletime.h
  Revised:        $Date: 2015-06-17 11:11:08 -0700 (Wed, 17 Jun 2015) $
  Revision:       $Revision: 44123 $

  Description:    This file contains GATT service discovery and 
                  configuration definitions and prototypes for discovering
                  Time services.

  Copyright 2014 - 2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef BLETIME_H
#define BLETIME_H

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

// Time discovery states
enum
{
  DISC_IDLE = 0x00,                       // Idle state
  
  DISC_CURR_TIME_START = 0x10,            // Current time service
  DISC_CURR_TIME_SVC,                     // Discover service
  DISC_CURR_TIME_CHAR,                    // Discover all characteristics
  DISC_CURR_TIME_CT_TIME_CCCD,            // Discover CT time CCCD
  
  DISC_DST_CHG_START = 0x20,              // DST change service
  DISC_DST_CHG_SVC,                       // Discover service
  DISC_DST_CHG_CHAR,                      // Discover all characteristics

  DISC_REF_TIME_START = 0x30,             // Reference time service
  DISC_REF_TIME_SVC,                      // Discover service
  DISC_REF_TIME_CHAR,                     // Discover all characteristics

  DISC_NWA_START = 0x40,                  // NwA service
  DISC_NWA_SVC,                           // Discover service
  DISC_NWA_CHAR,                          // Discover all characteristics
  DISC_NWA_NWA_CCCD,                      // Discover NwA CCCD
    
  DISC_ALERT_NTF_START = 0x50,            // Alert notification service
  DISC_ALERT_NTF_SVC,                     // Discover service
  DISC_ALERT_NTF_CHAR,                    // Discover all characteristics
  DISC_ALERT_NTF_NEW_CCCD,                // Discover new alert CCCD
  DISC_ALERT_NTF_UNREAD_CCCD,             // Discover unread alert status CCCD

  DISC_BATT_START = 0x60,                 // Battery service
  DISC_BATT_SVC,                          // Discover service
  DISC_BATT_CHAR,                         // Discover all characteristics
  DISC_BATT_LVL_CCCD,                     // Discover battery level CCCD

  DISC_PAS_START = 0x70,                  // Phone alert status service
  DISC_PAS_SVC,                           // Discover service
  DISC_PAS_CHAR,                          // Discover all characteristics
  DISC_PAS_ALERT_CCCD,                    // Discover alert status CCCD
  DISC_PAS_RINGER_CCCD,                   // Discover ringer setting CCCD

  DISC_FAILED = 0xFF                      // Discovery failed
};

// Time handle cache indexes
enum
{
  HDL_CURR_TIME_CT_TIME_START,            // Current time start handle
  HDL_CURR_TIME_CT_TIME_END,              // Current time end handle
  HDL_CURR_TIME_CT_TIME_CCCD,             // Current time CCCD
  HDL_CURR_TIME_LOC_INFO,                 // Local time info
  HDL_CURR_TIME_REF_INFO,                 // Reference time info
  
  HDL_DST_CHG_TIME_DST,                   // Time with DST
  
  HDL_REF_TIME_UPD_CTRL,                  // Time update control point
  HDL_REF_TIME_UPD_STATE,                 // Time update state
  
  HDL_NWA_NWA_START,                      // NwA start handle
  HDL_NWA_NWA_END,                        // NwA end handle
  HDL_NWA_NWA_CCCD,                       // NwA CCCD
  
  HDL_ALERT_NTF_NEW_START,                // New alert start handle
  HDL_ALERT_NTF_NEW_END,                  // New alert end handle
  HDL_ALERT_NTF_UNREAD_START,             // Unread alert status start handle
  HDL_ALERT_NTF_UNREAD_END,               // Unread alert status end handle
  HDL_ALERT_NTF_CTRL,                     // Alert notification control point
  HDL_ALERT_NTF_NEW_CAT,                  // Supported New Alert Category
  HDL_ALERT_NTF_UNREAD_CAT,               // Supported Unread Alert Category
  HDL_ALERT_NTF_NEW_CCCD,                 // New alert CCCD
  HDL_ALERT_NTF_UNREAD_CCCD,              // Alert unread alert status CCCD

  HDL_BATT_LEVEL_START,                   // Battery level start handle
  HDL_BATT_LEVEL_END,                     // Battery level end handle
  HDL_BATT_LEVEL_CCCD,                    // Battery level CCCD

  HDL_PAS_ALERT_START,                    // Alert status start handle
  HDL_PAS_ALERT_END,                      // Alert status end handle
  HDL_PAS_RINGER_START,                   // Ringer setting start handle
  HDL_PAS_RINGER_END,                     // Ringer setting end handle
  HDL_PAS_CTRL,                           // Ringer control point
  HDL_PAS_ALERT_CCCD,                     // Alert status CCCD
  HDL_PAS_RINGER_CCCD,                    // Ringer setting CCCD

  HDL_CACHE_LEN
};

// Configuration states
#define TIME_CONFIG_START              0x00
#define TIME_CONFIG_CMPL               0xFF

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL
 */

// Connection handle
extern uint16_t Time_connHandle;

// Handle cache
extern uint16_t Time_handleCache[HDL_CACHE_LEN];

// Task ID
extern uint8_t Time_configDone;

/*********************************************************************
 * FUNCTIONS
 */

/* 
 * Time service discovery functions.
 */
extern uint8_t Time_discStart(void);
extern uint8_t Time_discGattMsg(uint8_t state, gattMsgEvent_t *pMsg);

/* 
 * Time characteristic configuration functions.
 */
extern uint8_t Time_configNext(uint8_t state);
extern uint8_t Time_configGattMsg(uint8_t state, gattMsgEvent_t *pMsg);

/* 
 * Time indication and notification handling functions.
 */
extern void Time_indGattMsg(gattMsgEvent_t *pMsg);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BLETIME_H */
