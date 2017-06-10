/*******************************************************************************
  Filename:       bleUserConfig.c
  Revised:        $Date: 2014-05-06 13:15:59 -0700 (Tue, 06 May 2014) $
  Revision:       $Revision: 38426 $

  Description:    This file contains user configurable variables for the BLE
                  Controller and Host.

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

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "ll_userConfig.h"
#include "bleUserConfig.h"

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

#include "bleDispatch.h"
#include "l2cap.h"

#endif // ( CENTRAL_CFG | PERIPHERAL_CFG )

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
 * @fn      setBleUserConfig
 *
 * @brief   Set the user configurable variables for the BLE
 *          Controller and Host.
 *
 *          Note: This function should be called at the start
 *                of stack_main.
 *
 * @param   userCfg - pointer to user configuration
 *
 * @return  none
 */
void setBleUserConfig( bleUserCfg_t *userCfg )
{
  if ( userCfg != NULL )
  {
#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )
    l2capUserCfg_t l2capUserCfg;

    // user reconfiguration of Host variables
    l2capUserCfg.maxNumPSM = userCfg->maxNumPSM;
    l2capUserCfg.maxNumCoChannels = userCfg->maxNumCoChannels;

    L2CAP_SetUserConfig( &l2capUserCfg );

    if ( userCfg->pfnBMAlloc != NULL )
    {
      *userCfg->pfnBMAlloc = bleDispatch_BMAlloc;
    }

    if ( userCfg->pfnBMFree != NULL )
    {
      *userCfg->pfnBMFree = bleDispatch_BMFree;
    }
#endif // ( CENTRAL_CFG | PERIPHERAL_CFG )

    // user reconfiguration of Controller variables
    llUserConfig.maxNumConns  = userCfg->maxNumConns;
    llUserConfig.numTxEntries = userCfg->maxNumPDUs;
    llUserConfig.maxPduSize   = userCfg->maxPduSize;

    // RF Front End Mode and Bias (based on package)
    llUserConfig.rfFeModeBias = userCfg->rfFeModeBias;

    // RF Override Registers
    llUserConfig.rfRegPtr     = userCfg->rfRegTbl;

    // Tx Power Table
    llUserConfig.txPwrTblPtr  = userCfg->txPwrTbl;

    // PM Startup Margin
    llUserConfig.startupMarginUsecs = userCfg->startupMarginUsecs;
  }

  return;
}

/*******************************************************************************
 */
