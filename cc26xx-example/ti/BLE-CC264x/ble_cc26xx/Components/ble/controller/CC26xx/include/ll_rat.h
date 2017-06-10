/*******************************************************************************
  Filename:       ll_rat.h
  Revised:        $Date: 2011-08-22 08:41:40 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27235 $

  Description:    This file contains the Link Layer (LL) types, constants,
                  API's etc. for the Bluetooth Low Energy (BLE) Controller
                  Radio Access Timer (RAT).

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

#ifndef LL_RAT_H
#define LL_RAT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include <inc/hw_rfc_rat.h>

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define RAT_BASE                       RFC_RAT_BASE

// Registers
#define RAT_CONFIG_REG                 HWREG(RAT_BASE + RFC_RAT_O_RATCFG)
#define RAT_COUNT_REG                  HWREG(RAT_BASE + RFC_RAT_O_RATCNT)
#define RAT_ADJUST_REG                 HWREG(RAT_BASE + RFC_RAT_O_RATADJ)
#define RAT_ARM_CAPTURE_REG            HWREG(RAT_BASE + RFC_RAT_O_RATARMSET)
#define RAT_ARM_CLEAR_REG              HWREG(RAT_BASE + RFC_RAT_O_RATARMCLR)
#define RAT_OUT_EVT_REG                HWREG(RAT_BASE + RFC_RAT_O_RATEV)
#define RAT_OUT_MODE_CONFIG_REG        HWREG(RAT_BASE + RFC_RAT_O_RATOUT)
#define RAT_CHAN_0_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH0CFG)
#define RAT_CHAN_1_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH1CFG)
#define RAT_CHAN_2_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH2CFG)
#define RAT_CHAN_3_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH3CFG)
#define RAT_CHAN_4_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH4CFG)
#define RAT_CHAN_5_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH5CFG)
#define RAT_CHAN_6_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH6CFG)
#define RAT_CHAN_7_CONFIG_REG          HWREG(RAT_BASE + RFC_RAT_O_RATCH7CFG)
#define RAT_CHAN_0_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH0VAL)
#define RAT_CHAN_1_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH1VAL)
#define RAT_CHAN_2_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH2VAL)
#define RAT_CHAN_3_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH3VAL)
#define RAT_CHAN_4_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH4VAL)
#define RAT_CHAN_5_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH5VAL)
#define RAT_CHAN_6_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH6VAL)
#define RAT_CHAN_7_CAPT_COMP_REG       HWREG(RAT_BASE + RFC_RAT_O_RATCH7VAL)

#define LL_MAX_32BIT_TIME_IN_625US     0x07A12000  // 32s in 625us ticks (LSTO limit)
#define LL_MAX_32BIT_TIME              0xFFFFFFFF

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
 * Functions
 */

extern void llInitRAT( void );

extern uint32 llGetCurrentTime( void );

extern uint8 llTimeCompare( uint32 time1, uint32 time2 );

extern uint32 llTimeDelta( uint32 time1, uint32 time2 );

#ifdef __cplusplus
}
#endif

#endif /* LL_RAT_H */
