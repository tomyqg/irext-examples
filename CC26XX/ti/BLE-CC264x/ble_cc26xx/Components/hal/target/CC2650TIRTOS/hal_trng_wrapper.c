/*******************************************************************************
  Filename:       hal_trng_wrapper.c
  Revised:        $Date: 2011-12-01 18:45:35 -0800 (Thu, 01 Dec 2011) $
  Revision:       $Revision: 28536 $

  Description:    This file contains an API for returning a True Random
                  Number Generator until one is provided elsewhere.

  Copyright 2011 - 2015 Texas Instruments Incorporated. All rights reserved.

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

#include <inc/hw_types.h>
#include <inc/hw_sysctl.h>
#include "hal_trng_wrapper.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

static uint32 lastTrngVal;

/*
** Software FIFO Application Programming Interface
*/

/*******************************************************************************
 * @fn          HalTRNG_InitTRNG
 *
 * @brief       This routine initializes the TRNG hardware.
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
void HalTRNG_InitTRNG( void )
{
  // configure TRNG
  // Note: Min=4x64, Max=1x256, ClkDiv=1+1 gives the same startup and refill
  //       time, and takes about 11us (~14us with overhead).
  TRNGConfigure( 256, 256, 0x01 );

  // enable TRNG
  TRNGEnable();

  // init variable to hold the last value read
  lastTrngVal = 0;

  return;
}


/*******************************************************************************
 * @fn          HalTRNG_WaitForReady
 *
 * @brief       This routine waits until the TRNG hardware is ready to be used.
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
void HalTRNG_WaitForReady( void )
{
  // poll status
  while(!(TRNGStatusGet() & TRNG_NUMBER_READY));

  return;
}


/*******************************************************************************
 * @fn          HalTRNG_GetTRNG
 *
 * @brief       This routine returns a 32 bit TRNG number.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      A 32 bit TRNG number.
 */
uint32 HalTRNG_GetTRNG( void )
{
  uint32 trngVal;

  // initialize and enable TRNG if TRNG is not enabled
  if (0 == (HWREG(TRNG_BASE + TRNG_O_CTL) & TRNG_CTL_TRNG_EN))
  {
    HalTRNG_InitTRNG();
  }

  // check that a valid value is ready
  while(!(TRNGStatusGet() & TRNG_NUMBER_READY));

  // check to be sure we're not getting the same value repeatedly
  if ( (trngVal = TRNGNumberGet(TRNG_LOW_WORD)) == lastTrngVal )
  {
    return( 0xDEADBEEF );
  }
  else // value changed!
  {
    // so save last TRNG value
    lastTrngVal = trngVal;

    return( trngVal );
  }
}


/*******************************************************************************
 */


