/**
  @file  nvintf.h
  $Date: 2015-02-15 18:34:28 -0800 (Sun, 15 Feb 2015) $
  $Revision: 42614 $

  @brief Function pointer interface to the NV API

  <!--
  Copyright 2014-2015 Texas Instruments Incorporated. All rights reserved.

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
  -->
*/
#ifndef NVINTF_H
#define NVINTF_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

// NV system ID codes
#define NVINTF_SYSID_NVDRVR 0
#define NVINTF_SYSID_ZSTACK 1
#define NVINTF_SYSID_TIMAC  2
#define NVINTF_SYSID_REMOTI 3
#define NVINTF_SYSID_BLE    4
#define NVINTF_SYSID_6MESH  5
#define NVINTF_SYSID_APP    6

// NV driver status codes
#define NVINTF_SUCCESS      0
#define NVINTF_FAILURE      1
#define NVINTF_CORRUPT      2
#define NVINTF_NOTREADY     3
#define NVINTF_BADPARAM     4
#define NVINTF_BADLENGTH    5
#define NVINTF_BADOFFSET    6
#define NVINTF_BADITEMID    7
#define NVINTF_BADSUBID     8
#define NVINTF_BADSYSID     9
#define NVINTF_NOTFOUND     10
#define NVINTF_LOWPOWER     11
#define NVINTF_BADVERSION   12

//*****************************************************************************
// Typedefs
//*****************************************************************************

/**
 * NV Item Identification structure
 */
typedef struct nvintf_itemid_t
{
    //! NV System ID - identifies system (ZStack, BLE, App, OAD...)
    uint8_t systemID;
    //! NV Item ID
    uint16_t itemID;
    //! NV Item sub ID
    uint16_t subID;
} NVINTF_itemID_t;

//! Function pointer definition for the NVINTF_initNV() function
typedef uint8_t (*NVINTF_initNV)(void *param);

//! Function pointer definition for the NVINTF_compactNV() function
typedef uint8_t (*NVINTF_compactNV)(uint16_t minBytes);

//! Function pointer definition for the NVINTF_createItem() function
typedef uint8_t (*NVINTF_createItem)(NVINTF_itemID_t id,
                                     uint32_t length,
                                     void *buffer );

//! Function pointer definition for the NVINTF_deleteItem() function
typedef uint8_t (*NVINTF_deleteItem)(NVINTF_itemID_t id);

//! Function pointer definition for the NVINTF_readItem() function
typedef uint8_t (*NVINTF_readItem)(NVINTF_itemID_t id,
                                   uint16_t offset,
                                   uint16_t length,
                                   void *buffer );

//! Function pointer definition for the NVINTF_writeItem() function
typedef uint8_t (*NVINTF_writeItem)(NVINTF_itemID_t id,
                                    uint16_t length,
                                    void *buffer );

//! Function pointer definition for the NVINTF_writeItemEx() function
typedef uint8_t (*NVINTF_writeItemEx)(NVINTF_itemID_t id,
                                      uint16_t offset,
                                      uint16_t length,
                                      void *buffer );

//! Function pointer definition for the NVINTF_getItemLen() function
typedef uint32_t (*NVINTF_getItemLen)(NVINTF_itemID_t id);

//! Structure of NV API function pointers
typedef struct nvintf_nvfuncts_t
{
    //! Initialization function
    NVINTF_initNV initNV;
    //! Compact NV function
    NVINTF_compactNV compactNV;
    //! Create item function
    NVINTF_createItem createItem;
    //! Delete NV item function
    NVINTF_deleteItem deleteItem;
    //! Read item function
    NVINTF_readItem readItem;
    //! Write item function
    NVINTF_writeItem writeItem;
    //! Write existing item function
    NVINTF_writeItemEx writeItemEx;
    //! Get item length function
    NVINTF_getItemLen getItemLen;
} NVINTF_nvFuncts_t;

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* NVINTF_H */

