/*******************************************************************************
  Filename:       ll_timerDrift.h
  Revised:        $Date: 2011-11-14 12:19:37 -0800 (Mon, 14 Nov 2011) $
  Revision:       $Revision: 28328 $

  Description:    This file contains the Link Layer (LL) types, constants,
                  API's etc. for the Bluetooth Low Energy (ULE) Controller
                  Timer Drift routines.

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

#ifndef LL_TIMER_DRIFT_H
#define LL_TIMER_DRIFT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include "ble.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define LL_SYNTH_CALIBRATION                (RAT_TICKS_IN_256US)
#define LL_RX_SETTLE_TIME                   (RAT_TICKS_IN_64US)
#define LL_RX_RAMP_OVERHEAD                 (LL_SYNTH_CALIBRATION + LL_RX_SETTLE_TIME)
#define LL_JITTER_CORRECTION                (RAT_TICKS_IN_16US)
#define LL_RX_SYNCH_OVERHEAD                (RAT_TICKS_IN_140US) //(RAT_TICKS_IN_85US)
#define LL_TIMESTAMP_CORRECTION             (RAT_TICKS_IN_6US)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * API
 */

extern NEAR_FUNC uint16 llCalcScaFactor( uint8 masterSCA );

#ifdef __cplusplus
}
#endif

#endif /* LL_TIMER_DRIFT_H */
