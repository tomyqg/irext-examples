/**************************************************************************************************
  Filename:       time_config.c
  Revised:        $Date: 2015-07-06 17:12:12 -0700 (Mon, 06 Jul 2015) $
  Revision:       $Revision: 44317 $

  Description:    Time characteristic configuration routines.

  Copyright 2011 - 2014 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "time_clock.h"
#include "bletime.h"

/*********************************************************************
 * MACROS
 */

// Used to determine the end of Time_configList[]
#define TIME_CONFIG_MAX      (sizeof(Time_configList) / sizeof(uint8_t))

/*********************************************************************
 * CONSTANTS
 */

// Array of handle cache indexes.  This list determines the
// characteristics that are read or written during configuration.
const uint8_t Time_configList[] =
{
  HDL_CURR_TIME_CT_TIME_START,            // Current time
  HDL_CURR_TIME_CT_TIME_CCCD,             // Current time CCCD
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Connection handle.
uint16_t Time_connHandle = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

uint8_t Time_configDone = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      Time_configNext()
 *
 * @brief   Perform the characteristic configuration read or
 *          write procedure.
 *
 * @param   state - Configuration state.
 *
 * @return  New configuration state.
 */
uint8_t Time_configNext(uint8_t state)
{
  bool read;

  // Find next non-zero cached handle of interest
  while (state < TIME_CONFIG_MAX &&
          Time_handleCache[Time_configList[state]] == 0)
  {
    state++;
  }

  // Return if reached end of list
  if (state >= TIME_CONFIG_MAX)
  {
    return TIME_CONFIG_CMPL;
  }

  // Determine what to do with characteristic
  switch (Time_configList[state])
  {
    // Read these characteristics
    case HDL_CURR_TIME_CT_TIME_START:
      read = TRUE;
      break;

    // Set notification for these characteristics
    case HDL_CURR_TIME_CT_TIME_CCCD:
      read = FALSE;
      break;

    default:
      return state;
  }

  if(Time_configDone==TRUE)
  {
    return state;
  }
  
  // Do a GATT read or write
  if (read)
  {
    attReadReq_t  readReq;
      
    readReq.handle = Time_handleCache[Time_configList[state]];
    
    // Send the read request
    GATT_ReadCharValue(Time_connHandle, &readReq, ICall_getEntityId());
    
    // Only reading time right now
    Time_configDone = TRUE;
  }
  else
  {
    attWriteReq_t writeReq;
    
    writeReq.pValue = GATT_bm_alloc(Time_connHandle, ATT_WRITE_REQ, 2, NULL);
    if (writeReq.pValue != NULL)
    {
      writeReq.len = 2;
      writeReq.pValue[0] = LO_UINT16(GATT_CLIENT_CFG_NOTIFY);
      writeReq.pValue[1] = HI_UINT16(GATT_CLIENT_CFG_NOTIFY);
      writeReq.sig = 0;
      writeReq.cmd = 0;

      writeReq.handle = Time_handleCache[Time_configList[state]];
      
      // Send the read request
      if (GATT_WriteCharValue(Time_connHandle, &writeReq, 
                              ICall_getEntityId()) != SUCCESS)
      {
        GATT_bm_free((gattMsg_t *)&writeReq, ATT_WRITE_REQ);
      }
    }
  }

  return state;
}

/*********************************************************************
 * @fn      Time_configGattMsg()
   *
 * @brief   Handle GATT messages for characteristic configuration.
 *
 * @param   state - Discovery state.
 * @param   pMsg - GATT message.
 *
 * @return  New configuration state.
 */
uint8_t Time_configGattMsg(uint8_t state, gattMsgEvent_t *pMsg)
{
  
  if ((pMsg->method == ATT_READ_RSP || pMsg->method == ATT_WRITE_RSP) &&
       (pMsg->hdr.status == SUCCESS))
  {
    // Process response
    switch (Time_configList[state])
    {
      case HDL_CURR_TIME_CT_TIME_START:
        // Set clock to time read from time server
        Time_clockSet(pMsg->msg.readRsp.pValue);
        break;

      default:
        break;
    }
  }
  return Time_configNext(state + 1);
}


/*********************************************************************
*********************************************************************/
