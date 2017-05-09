/*******************************************************************************
  Filename:       ll_userConfig.h
  Revised:        $Date: 2015-01-09 08:33:33 -0800 (Fri, 09 Jan 2015) $
  Revision:       $Revision: 41687 $

  Description:    This file contains user configurable variables for the BLE
                  Controller.

                  To change the default value of configurable variable:
                    - Include "ll_userConfig.h" in your OSAL_ICallBle.c file.
                    - Set the variables at the start of stack_main. Actually,
                      it is okay to set the variables anywhere in stack_main
                      as long as it is BEFORE osal_init_system, but best to
                      set at the very start of stack_main.

                  Note: User configurable variables are only used during the
                        initialization of the Controller. Changing the values
                        of these variables after this will have no effect.

                  For example:
                    int stack_main()
                    {
                      // user reconfiguration of Controller variables
                      llUserConfig.maxNumConns  = 1;
                      llUserConfig.numTxEntries = 10;
                                 :

                  Default values:
                    maxNumConns  : 3
                    numTxEntries : 6

  Copyright 2014-2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef LL_USER_CONFIG_H
#define LL_USER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include "bleUserConfig.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8          maxNumConns;        // Max number of BLE connections
  uint8          numTxEntries;       // Max number of BLE connection Tx buffers
  uint8          maxPduSize;         // Max PDU data size
  uint8          rfFeModeBias;       // RF Front End Mode and Bias
  regOverride_t *rfRegPtr;           // RF Override Registers
  txPwrTbl_t    *txPwrTblPtr;        // Tx Power Table
  uint32         startupMarginUsecs; // Startup Margin
} llUserCfg_t;

/*******************************************************************************
 * LOCAL VARIABLES
 */


/*******************************************************************************
 * GLOBAL VARIABLES
 */

extern llUserCfg_t llUserConfig;

#ifdef __cplusplus
}
#endif

#endif /* LL_USER_CONFIG_H */
