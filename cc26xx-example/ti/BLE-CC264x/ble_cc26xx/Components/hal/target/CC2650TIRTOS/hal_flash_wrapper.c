/**************************************************************************************************
  Filename:       hal_flash_wrapper.c
  Revised:        $Date: 2013-10-01 10:39:43 -0700 (Tue, 01 Oct 2013) $
  Revision:       $Revision: 35500 $

  Description:    This file implements the hal_flash interface for the flash driver.


  Copyright 2005-2014 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board.h"
#include "hal_types.h"
#include <driverlib/flash.h>
#include <inc/hw_memmap.h>
   
uint8* HalFlashGetAddress( uint8 pg, uint16 offset );
/**************************************************************************************************
 * @fn          HalFlashRead
 *
 * @brief       This function reads 'cnt' bytes from the internal flash.
 *
 * input parameters
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 * @param       offset - Valid offset into the page (so < HAL_NV_PAGE_SIZE and byte-aligned is ok).
 * @param       buf - Valid buffer space at least as big as the 'cnt' parameter.
 * @param       cnt - Valid number of bytes to read: a read cannot cross into the next 32KB bank.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashRead(uint8 pg, uint16 offset, uint8 *buf, uint16 cnt)
{
  halIntState_t cs;
  
  // Calculate the offset into the containing flash bank as it gets mapped into XDATA.
  uint8 *ptr = HalFlashGetAddress(pg, offset);
  
  // Enter Critical Section.
  HAL_ENTER_CRITICAL_SECTION(cs);
  
  // Read data.
  while (cnt--)
  {
    *buf++ = *ptr++;
  }

  // Exit Critical Section.
  HAL_EXIT_CRITICAL_SECTION(cs);
}

/**************************************************************************************************
 * @fn          HalFlashWrite
 *
 * @brief       This function reads 'cnt' bytes from the internal flash.
 *
 * input parameters
 *
 * @param       addr - Valid HAL flash write address: actual addr / 4 and quad-aligned.
 * @param       buf - Valid buffer space at least as big as the 'cnt' parameter.
 * @param       cnt - Valid number of bytes to write: a write cannot cross into the next 32KB bank.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWrite(uint32 addr, uint8 *buf, uint16 cnt)
{
  FlashProgram( buf, addr, cnt );
}

/**************************************************************************************************
 * @fn          HalFlashErase
 *
 * @brief       This function erases 'cnt' pages of the internal flash.
 *
 * input parameters
 *
 * @param       pg - Valid HAL flash page number (ie < 128) to erase.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashErase(uint8 pg)
{  
  FlashSectorErase( (uint32)HalFlashGetAddress(pg, 0));
}

/**************************************************************************************************
 * @fn          HalFlashGetAddress
 *
 * @brief       This function maps a page and offset to the flash address
 *
 * input parameters
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 *
 * @param       offset - Valid HAL flash offset (ie < 4096).
 *
 * output parameters
 *
 * None.
 *
 * @return      flashAddr - the flash address to map to.
 **************************************************************************************************
 */
uint8* HalFlashGetAddress( uint8 pg, uint16 offset )
{
#ifndef FEATURE_OAD
  // Calculate the offset into the containing flash bank as it gets mapped into XDATA.
  uint8 *flashAddr = (uint8 *)(offset + HAL_NV_START_ADDR) + ((pg % HAL_NV_PAGE_BEG )* HAL_FLASH_PAGE_SIZE);
  
  return flashAddr;
#else //FEATURE_OAD
  // The actual address is a 4-KiloByte multiple of the page number plus offset in bytes.
  return (uint8*)((pg << 12) + offset);
#endif //FEATURE_OAD
}


/**************************************************************************************************
*/
