/*******************************************************************************
  Filename:       pwrmon.c
  Revised:        $Date: 2015-02-10 09:22:17 -0800 (Tue, 10 Feb 2015) $
  Revision:       $Revision: 42470 $

  Description:    Implementation for CC26xx Vdd monitoring subsystem.


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

/* -----------------------------------------------------------------------------
 * Includes
 * -----------------------------------------------------------------------------
 */
#include "pwrmon.h"

#include <driverlib/aon_batmon.h>

/* -----------------------------------------------------------------------------
 * Macros
 * -----------------------------------------------------------------------------
 */
#define BATMON_ON (HWREG(AON_BATMON_BASE + AON_BATMON_O_CTL) ? true : false)

/*******************************************************************************
 * @fn      PWRMON_check()
 *
 * @brief   Checks the caller supplied voltage threshold against the value read
 *          from the CC26xx BATMON register.
 *
 * @param   threshold - voltage to compare measured device Vdd to
 *
 * @return  false if device voltage less than limit, otherwise true
 *******************************************************************************
 */
bool PWRMON_check(uint16_t threshold)
{
    if(BATMON_ON && (AONBatMonBatteryVoltageGet() < threshold))
    {
        // Measured device voltage is below threshold
        return(false);
    }

    return(true);
}

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
void PWRMON_init(void)
{
    // Enable the CC26xx Battery Monitor
    AONBatMonEnable();

    // Configure compare/measure every 32 AON clock cycles,
    // measurement results are only updated if changes occur.
    AONBatMonMeasurementCycleSet(AON_BATMON_CYCLE_32);
}

/*******************************************************************************
*/
