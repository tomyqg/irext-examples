/*******************************************************************************
  Filename:       startup.c
  Revised:        $Date: 2014-02-13 12:53:30 -0800 (Thu, 13 Feb 2014) $
  Revision:       $Revision: 37225 $

  Description:    Startup code for CC2650 for use with IAR.

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
*******************************************************************************/

// Check if compiler is IAR
#if !(defined(__IAR_SYSTEMS_ICC__) || defined(__TI_COMPILER_VERSION__))
#error "startup.c: Unsupported compiler!"
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include <ICall.h>
#include <ICallAddrs.h>

#if defined( FLASH_ROM_BUILD )
#include "ROM_Init.h"
#include "CommonROM_Init.h"
#endif // FLASH_ROM_BUILD

/*******************************************************************************
 * EXTERNS
 */

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define CRC16_POLYNOMIAL  0x1021

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

ICall_Dispatcher ICall_dispatcher;
ICall_EnterCS    ICall_enterCriticalSection;
ICall_LeaveCS    ICall_leaveCriticalSection;

/*******************************************************************************
 * @fn          checkCRC16
 *
 * @brief       A simple implementation of the crc-16 checksum algorithm,
 *              courtesy of IAR (circa 2007).
 *
 *              Note:  If you use this function, you need to call it with an
 *                     extra number of zero bytes equal to the size of the
 *                     checksum (in this case 2), to "rotate out the answer".
 *
 * input parameters
 *
 * @param       crc   - The CRC value to start with.
 * @param       pAddr - Pointer to an array of bytes to run the CRC over.
 * @param       len   - The number of bytes to process.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      CRC-16 result.
 */
uint16 slow_crc16( uint16 crc, uint8 *pAddr, uint32 len )
{
  while (len--)
  {
    uint8 i;
    uint8 byte = *(pAddr++);

    for (i=0; i<8; ++i)
    {
      uint32 temp = crc;

      crc <<= 1;

      if ( byte & 0x80 )
      {
        crc |= 1;
      }

      if ( temp & 0x8000 )
      {
        crc ^= CRC16_POLYNOMIAL;
      }

      byte <<= 1;
    }
  }

  return crc;
}


/*******************************************************************************
 * @fn          validChecksum
 *
 * @brief       Check if the ROM checksum is valid.
 *
 *              Note: This routine assumes the CRC directly follows the end
 *                    address!
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE=Valid Checksum; FALSE=Invalid Checksum.
 */
uint8 validChecksum( const uint32 *beginAddr, const uint32 *endAddr )
{
  uint16 crc      = 0;
  uint16 romCRC   = *((uint16 *)(((uint8 *)endAddr)+1));
  uint8  zeros[2] = {0, 0};

  // calculate the ROM checksum
  crc = slow_crc16( crc,
                    (uint8 *)beginAddr,
                    (uint32)endAddr - (uint32)beginAddr + 1 );

  // needed to rotate out the answer
  crc = slow_crc16( crc, zeros, 2 );

  // Compare the calculated checksum with the stored
  return( (crc==romCRC)? TRUE : FALSE );
}


/*******************************************************************************
 * @fn          startup_entry
 *
 * @brief       This is the BLE stack entry point.
 *
 * input parameters
 *
 * @param       arg0   argument containing remote dispatch function pointers
 * @param       arg1   custom initialization parameter
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
#ifdef __TI_COMPILER_VERSION__
#pragma CODE_SECTION(startup_entry,"EntrySection")
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location="EntrySection"
#endif
void startup_entry( const ICall_RemoteTaskArg *arg0, void *arg1 )
{
  extern int stack_main( void *arg );

#if defined(__IAR_SYSTEMS_ICC__)
  extern void __iar_data_init3(void);
  __iar_data_init3();
#elif defined(__TI_COMPILER_VERSION__)
  extern void __TI_auto_init(void);
  __TI_auto_init();
#else
#error "Error: Must specify a compiler!"
#endif

  ICall_dispatcher = arg0->dispatch;
  ICall_enterCriticalSection = arg0->entercs;
  ICall_leaveCriticalSection = arg0->leavecs;

#if defined( FLASH_ROM_BUILD )
  // initialize the Common ROM
  CommonROM_Init();

  // initialize the BLE Controller ROM
  ROM_Init();
#endif // FLASH_ROM_BUILD

  stack_main( arg1 );
}
