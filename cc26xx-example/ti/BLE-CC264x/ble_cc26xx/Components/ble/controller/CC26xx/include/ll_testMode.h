/*******************************************************************************
  Filename:       ll_testMode.h
  Revised:        $Date: 2010-08-30 14:26:12 -0700 (Mon, 30 Aug 2010) $
  Revision:       $Revision: 23573 $

  Description:    This file contains the Link Layer (LL) types, constants,
                  API's etc. to support LL-only Test Mode.

  Copyright 2009-2015 Texas Instruments Incorporated. All rights reserved.

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
*******************************************************************************/

#ifndef LL_TEST_MODE_H
#define LL_TEST_MODE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include "comdef.h"
#include "ll.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

// Test Mode
#define NUM_BYTES_FOR_CHAN_MAP               5
#define ENC_RAND_LEN                         8
#define ENC_EDIV_LEN                         2
#define ENC_LTK_LEN                          16
#define ENC_IV_LEN                           8
#define ENC_SKD_LEN                          16
#define ENC_SK_LEN                           16

/*******************************************************************************
 * TYPEDEFS
 */

/*
** Test Mode Data
*/
typedef struct
{
  // Encryption stuff
  struct
  {
    uint8 IV[ ENC_IV_LEN ];                         // combined master and slave IV values concatenated
    uint8 SKD [ ENC_SKD_LEN ];                      // combined master and slave SKD values concatenated
    uint8 RAND[ ENC_RAND_LEN ];                     // random vector from Master
    uint8 EDIV[ ENC_EDIV_LEN ];                     // encrypted diversifier from Master
    uint8 SK[ ENC_SK_LEN ];                         // session key derived from LTK and SKD
    uint8 LTK[ ENC_LTK_LEN ];
  } encInfo;

  // Master stuff
  uint8  winSize;                                    // specify a window size value
  uint16 winOffset;                                  // specify a window offset value
  uint8  updateChanMap[ NUM_BYTES_FOR_CHAN_MAP ];    // specify a data channel map update
  uint8  SCA;                                        // specify a sleep clock accuracy
  uint16 instant;                                    // specify the number of events before a control update takes place
  uint8  scanChanOverride;                           // flag to indicate scan channel is being overridden
  uint8  scanChanValue;                              // scan channel to be used during an override
  uint8  flipTxMIC;                                  // flag to indicate a MIC error should be forced

  // Slave stuff

} testInfo_t;

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * Test Callbacks
 */

/*******************************************************************************
 * @fn          LL_TEST_ConnectionEventCback Callback
 *
 * @brief       This Callback is used by the LL for test support. It provides
 *              the current connection ID and the current event count for test
 *              software.
 *
 * input parameters
 *
 * @param       connId       - The current connection ID.
 * @param       currentEvent - The current event count.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
extern void LL_TEST_ConnectionEventCback( uint16 connId,
                                          uint16 currentEvent );


/*******************************************************************************
 * @fn          LL_TEST_AdvEventCback Callback
 *
 * @brief       This Callback is used by the LL for test support. It provides
 *              an opportunity for the test software to make updates while
 *              advertising.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
extern void LL_TEST_AdvEventCback( void );


/*******************************************************************************
 * @fn          LL_TEST_ScanEventCback Callback
 *
 * @brief       This Callback is used by the LL for test support. It provides
 *              an opportunity for the test software to make updates while
 *              scanning.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
extern void LL_TEST_ScanEventCback( void );


/*******************************************************************************
 * @fn          LL_TEST_PostScanEventCback Callback
 *
 * @brief       This Callback is used by the LL for test support. It provides
 *              an opportunity for the test software to make updates while
 *              scanning after the Task Done post-processing has completed.
 *              This is necessary for test cases that require the scanner to
 *              stop then start again, which should not be done within the
 *              context of Task Done post-processing.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
extern void LL_TEST_PostScanEventCback( void );


#ifdef __cplusplus
}
#endif

#endif /* LL_TEST_MODE_H */

