/*******************************************************************************
  Filename:       R2F_CommonFlashJT.h
  Revised:        $Date: 2015-07-22 10:45:09 -0700 (Wed, 22 Jul 2015) $
  Revision:       $Revision: 44392 $

  Description:    This file contains the defines for every flash function call
                  using the ROM-to-Flash Flash Jump Table.

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

#ifndef R2F_COMMON_FLASH_JT_H
#define R2F_COMMON_FLASH_JT_H

#if defined( COMMON_ROM_BUILD )

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"

/*******************************************************************************
 * EXTERNS
 */

// Common ROM's RAM table for pointers to ICall functions and flash jump tables.
// Note: This linker imported symbol is treated as a variable by the compiler.
// 0: iCall Dispatch Function Pointer
// 1: iCall Enter Critical Section Function Pointer
// 2: iCall Leave Critical Section Function Pointer
// 3: R2F Flash Jump Table Pointer
// 4: R2R Flash Jump Table Pointer
extern uint32 COMMON_RAM_BASE_ADDR[];

/*******************************************************************************
 * CONSTANTS
 */

// Common ROM's RAM table offset to R2F flash jump table pointer.
#define COMMON_ROM_RAM_TABLE_R2F   3

// Defines used for the flash jump table routines that are not part of build.
// Note: Any change to this table must accompany a change to Flash_JT[]!
#define R2F_JT_LOCATION            (&COMMON_RAM_BASE_ADDR[COMMON_ROM_RAM_TABLE_R2F])

#define R2F_JT_BASE                (*((uint32 **)R2F_JT_LOCATION))
#define R2F_JT_OFFSET(index)       (*(R2F_JT_BASE+(index)))

// ROM-to-Flash Functions
#define osal_mem_alloc             ((void *(*) (uint16))                       R2F_JT_OFFSET(0))
#define osal_mem_free              ((void  (*) (void *))                       R2F_JT_OFFSET(1))
#define osal_bm_free               ((void  (*) (void *))                       R2F_JT_OFFSET(2))
#define osal_memcpy                ((void *(*) (void *, const void *, uint32)) R2F_JT_OFFSET(3))
#define osal_memset                ((void *(*) (void *, uint8, int))           R2F_JT_OFFSET(4))
#define halAssertHandler           ((void  (*) (void))                         R2F_JT_OFFSET(5))
#endif // COMMON_ROM_BUILD

#endif /* R2F_COMMON_FLASH_JT_H */



