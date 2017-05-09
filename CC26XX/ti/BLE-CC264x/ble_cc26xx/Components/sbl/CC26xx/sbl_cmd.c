/**************************************************************************************************
  @headerfile:    sbl_cmd.c
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <xdc/std.h>

#include "sbl.h"
#include "sbl_cmd.h"
#include "sbl_tl.h"

/*********************************************************************
 * CONSTANTS
 */

#define SBL_MAX_BYTES_PER_TRANSFER   252
   
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SBL_CMD_uint32MSB(uint32_t src, uint8_t *pDst);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

uint8_t SBL_CMD_ping(void)
{
  return SBL_TL_sendCmd(SBL_CMD_PING, NULL, 0);
}
   
uint8_t SBL_CMD_download(uint32_t addr, uint32_t size)
{
  uint8_t payload[8];
  
  // Check input arguments
  if (size & 0x03)
  {
    // NEED TO CHECK: addr in range of device
    // CHECKING: byte count must be multiple of 4
    return SBL_FAILURE;
  }

  // Initialize payload - MSB order
  SBL_CMD_uint32MSB(addr, &payload[0]);
  SBL_CMD_uint32MSB(size, &payload[4]);

  return SBL_TL_sendCmd(SBL_CMD_DOWNLOAD, payload, sizeof(payload));
}

uint8_t SBL_CMD_getStatus(void)
{
  uint8_t rsp[1];
  uint16_t rspLen = 0;
  
  // Send Get Status command
  if (SBL_TL_sendCmd(SBL_CMD_GET_STATUS, NULL, 0) == SBL_FAILURE)
  {
    return SBL_FAILURE;
  }
  
  // Get the status response from target
  SBL_TL_getRsp(rsp, sizeof(rsp), &rspLen);
  
  return rsp[0];
}

uint8_t SBL_CMD_sendData(uint8_t *pData, uint16_t len)
{
  // Check input arguments
  if (len > SBL_MAX_BYTES_PER_TRANSFER)
  {
    // Length of bytes excess maximum allowed per transfer
    return SBL_FAILURE;
  }

  // Send command
  return SBL_TL_sendCmd(SBL_CMD_SEND_DATA, pData, len);
}

uint8_t SBL_CMD_reset(void)
{
  return SBL_TL_sendCmd(SBL_CMD_RESET, NULL, 0);
}

uint8_t SBL_CMD_sectorErase(uint32_t addr)
{
  uint8_t payload[4];
  
  // Initialize payload - MSB order
  SBL_CMD_uint32MSB(addr, &payload[0]);

  // Send command
  return SBL_TL_sendCmd(SBL_CMD_SECTOR_ERASE, payload, sizeof(payload));
}

uint8_t SBL_CMD_crc32(uint32_t addr, uint32_t size, uint32_t readRepeatCnt)
{
  uint8_t payload[12];

  // Initialize payload - MSB order
  SBL_CMD_uint32MSB(addr, &payload[0]);
  SBL_CMD_uint32MSB(size, &payload[4]);
  SBL_CMD_uint32MSB(readRepeatCnt, &payload[8]);

  return SBL_TL_sendCmd(SBL_CMD_CRC32, payload, sizeof(payload));
}

uint32_t SBL_CMD_getChipID(void)
{
  uint8_t rsp[4];
  uint16_t rspLen = 0;
  uint32_t chipID = 0;
  
  // Send Get Chip ID command
  if (SBL_TL_sendCmd(SBL_CMD_GET_CHIP_ID, NULL, 0) == SBL_FAILURE)
  {
    return SBL_FAILURE;
  }
  
  // Get the status response from target
  SBL_TL_getRsp(rsp, sizeof(rsp), &rspLen);
  
  // Reverse MSB order of response to get Chip ID
  SBL_CMD_uint32MSB(chipID, rsp);
  
  return chipID;
}

uint8_t SBL_CMD_bankErase(void)
{
  return SBL_TL_sendCmd(SBL_CMD_BANK_ERASE, NULL, 0);
}
   
uint8_t SBL_CMD_setCCFG(uint32_t fieldID, uint32_t fieldValue)
{
  uint8_t payload[8];

  // Initialize payload - MSB order
  SBL_CMD_uint32MSB(fieldID, &payload[0]);
  SBL_CMD_uint32MSB(fieldValue, &payload[4]);

  return SBL_TL_sendCmd(SBL_CMD_SET_CCFG, payload, sizeof(payload));
}

void SBL_CMD_uint32MSB(uint32_t src, uint8_t *pDst)
{
  // MSB first
  pDst[0] = (uint8_t)(src >> 24);
  pDst[1] = (uint8_t)(src >> 16);
  pDst[2] = (uint8_t)(src >> 8);
  pDst[3] = (uint8_t)(src >> 0);
}
