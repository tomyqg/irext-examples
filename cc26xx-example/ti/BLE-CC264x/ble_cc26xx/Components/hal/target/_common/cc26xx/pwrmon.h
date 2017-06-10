/*******************************************************************************
  Filename:       pwrmon.h
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    Declarations for CC26xx Vdd monitoring subsystem.


  Copyright 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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
#ifndef PWRMON_H
#define PWRMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------------
 * Includes
 * -----------------------------------------------------------------------------
 */
#include "hal_types.h"

/* -----------------------------------------------------------------------------
 * Default Voltage Thresholds
 * -----------------------------------------------------------------------------
 */

// Voltage thresholds are defined in "units" compatible with the CC26xx
// AON_BATMON_O_BAT register (see Technical Reference Manual for details).
// The register provides the voltage reading in 11 bits, masked by 0x7FF,
// where 'whole' volts are located in bits 8-10 (0x700) and the 'fraction'
// is located in bits 0-7 (0x0FF). The 'fraction' (0.00-0.99) is scaled
// linearly across the digital values of 0x00-0xFF, with 0x80 = 0.50 volt,
// 0xFF = 0.99 volt, etc.
// The default values provided below indicate settings slightly above and
// somewhat below the CC26xx brown-out threshold. It is intended that the
// user will modify these values appropriately for their system needs.

#if !defined (MIN_VDD_INIT)
#define MIN_VDD_INIT   0x1CE  // 1.80 volts (0.80=206/256 -> 206=0xCE)
#endif

#if !defined (MIN_VDD_POLL)
#define MIN_VDD_POLL   0x1DA  // 1.85 volts (0.85=218/256 -> 218=0xDA)
#endif

#if !defined (MIN_VDD_FLASH)
#define MIN_VDD_FLASH  0x180  // 1.50 volts (0.50=128/256 -> 128=0x80)
//#define MIN_VDD_FLASH  0x200  // 2.00 volts (0.00=  0/256 ->   0=0x00)
#endif

/*******************************************************************************
 * @fn      PWRMON_check()
 *
 * @brief   Checks the caller supplied voltage threshold against the value read
 *          from the CC26xx BATMON register.
 *
 * @param   threshold - voltage to compare device Vdd to (AON_BATMON_O_BAT)
 *
 * @return  false if device voltage less than limit, otherwise true
 *******************************************************************************
 */
extern bool PWRMON_check(uint16_t threshold);

/*******************************************************************************
 * @fn      PWRMON_init()
 *
 * @brief   Enable AON battery monitor and set update rate
 *
 * @param   None
 *
 * @return  None
 *******************************************************************************
 */
extern void PWRMON_init(void);

/*******************************************************************************
*/

#ifdef __cplusplus
};
#endif

#endif
