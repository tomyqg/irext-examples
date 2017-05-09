/**
  @file nvocop.h
  $Date: 2015-02-10 14:18:04 -0800 (Tue, 10 Feb 2015) $
  $Revision: 42483 $

  @brief NV definitions for CC26xx devices - On-Chip One-Page Flash Memory

  <!--
  Copyright 2014 - 2015 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  -->
*/
#ifndef NVOCOP_H
#define NVOCOP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_board_cfg.h"
#include "NVINTF.h"

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

#ifndef SNV_FIRST_PAGE
// One page reserved for this module
#define SNV_FIRST_PAGE  0x1E
#endif
  
#ifndef FLASH_PAGE_SIZE
// Common page size for all CC26xx devices
#define FLASH_PAGE_SIZE  HAL_FLASH_PAGE_SIZE
#endif

#ifndef FLASH_WORD_SIZE
#define FLASH_WORD_SIZE HAL_FLASH_WORD_SIZE
#endif

// Maximum length of a single NV item
#define MAX_NV_LEN     (FLASH_PAGE_SIZE / 4)

// Maximum ID parameters
#define MAX_NV_SYSID   0x003F  //  6 bits
#define MAX_NV_ITEMID  0x03FF  // 10 bits
#define MAX_NV_SUBID   0x03FF  // 10 bits

//*****************************************************************************
// Macros
//*****************************************************************************

//*****************************************************************************
// Typedefs
//*****************************************************************************

//*****************************************************************************
// Functions
//*****************************************************************************

extern uint8 NVOCOP_initNV( void *param );
extern uint8 NVOCOP_compactNV( uint16 minAvail );
extern uint8 NVOCOP_readItem( NVINTF_itemID_t nv_id, uint16 offset, uint16 len,
                              void *pBuf );
extern uint8 NVOCOP_writeItem( NVINTF_itemID_t nv_id, uint16 len, void *pBuf );

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* NVOCOP_H */

