/**************************************************************************************************
  Filename:       sbl_cmd.h
  Revised:        $Date: 2015-06-22 20:21:02 -0700 (Mon, 22 Jun 2015) $
  Revision:       $Revision: 44191 $

  Description:    This file contains SBL commands

  Copyright 2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef SBL_CMD_H
#define SBL_CMD_H

#ifdef __cplusplus
extern "C"
{
#endif
  
/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */

//!< \brief Serial Bootloader Command IDs for CC26xx
#define SBL_CMD_PING                    0x20
#define SBL_CMD_DOWNLOAD                0x21
#define SBL_CMD_GET_STATUS              0x23
#define SBL_CMD_SEND_DATA               0x24
#define SBL_CMD_RESET                   0x25
#define SBL_CMD_SECTOR_ERASE	          0x26
#define SBL_CMD_CRC32                   0x27
#define SBL_CMD_GET_CHIP_ID             0x28
#define SBL_CMD_MEMORY_READ             0x2A    // currently not supported
#define SBL_CMD_MEMORY_WRITE            0x2B    // currently not supported
#define SBL_CMD_BANK_ERASE		          0x2C
#define SBL_CMD_SET_CCFG                0x2D

//!< \brief Serial Bootloader Response IDs for CC26xx
#define SBL_CMD_RET_SUCCESS             0x40
#define SBL_CMD_RET_UNKNOWN_CMD         0x41
#define SBL_CMD_RET_INVALID_CMD         0x42
#define SBL_CMD_RET_INVALID_ADR         0x43
#define SBL_CMD_RET_FLASH_FAIL          0x44

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

extern uint8_t SBL_CMD_ping(void);   
   
extern uint8_t SBL_CMD_download(uint32_t addr, uint32_t size);

extern uint8_t SBL_CMD_getStatus(void);

extern uint8_t SBL_CMD_sendData(uint8_t *pData, uint16_t len);

extern uint8_t SBL_CMD_reset(void);

extern uint8_t SBL_CMD_sectorErase(uint32_t addr);

extern uint8_t SBL_CMD_crc32(uint32_t addr, uint32_t size, uint32_t readRepeatCnt);

extern uint32_t SBL_CMD_getChipID(void);

extern uint8_t SBL_CMD_bankErase(void);
   
extern uint8_t SBL_CMD_setCCFG(uint32_t fieldID, uint32_t fieldValue);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #define SBL_CMD_H */
