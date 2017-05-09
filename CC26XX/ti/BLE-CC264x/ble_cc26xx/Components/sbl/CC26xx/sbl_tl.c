/**************************************************************************************************
  @headerfile:    sbl_tl.c
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    This file contains transport layer code for SBL on CC26xx

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
#include <string.h>

#include "sbl.h"
#include "sbl_tl.h"

#include <ti/drivers/UART.h>

// Needed for UART driver re-open work-around
#include <driverlib/uart.h>
#include <ti/drivers/uart/UARTCC26XX.h>

/*********************************************************************
 * CONSTANTS
 */

enum {
  SBL_HDR_LEN_IDX = 0,
  SBL_HDR_CKS_IDX,   
  SBL_HDR_SIZE,
};

#define SBL_ACK_SIZE            2

#define DEVICE_ACK              0xCC
#define DEVICE_NACK             0x33

#define SBL_UART_BR             115200

const uint8_t NACK[] =          { 0x00, 0x33 };
const uint8_t ACK[] =           { 0x00, 0xcc };
const uint8_t BAUD[] =          { 0x55, 0x55 };

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

//! \brief UART Handle for UART Driver
static UART_Handle sblUartHandle;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static uint8_t SBL_TL_CKS(uint8_t cmd, uint8_t *pData, uint16_t len);

static uint8_t SBL_TL_sendACK(uint8_t ack);

static uint8_t SBL_TL_getRspACK(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @fn      SBL_TL_open
 *
 * @brief   Open device port for communication with the target device. Currently
 *          only supports UART                
 *
 * @param   pType - SBL_DEV_INTERFACE_[UART,SPI]
 * @param   pID - local serial interface ID (i.e. CC2650_UART0)
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_TL_open(uint8_t pType, uint8_t pID)
{
  UART_Params params;
  
  // Currently only support over UART
  if (pType == SBL_DEV_INTERFACE_UART)
  {
    // Configure UART parameters.
    // No parity bit, One stop bit set by default
    UART_Params_init(&params);
    params.baudRate = SBL_UART_BR;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.readEcho = UART_ECHO_OFF;
    
    // Open UART port for sending SBL commands
    sblUartHandle = UART_open(pID, &params);
    if (sblUartHandle == NULL)
    {
      return SBL_FAILURE;
    }

    // Work around for UART driver re-open bug. Sometimes there is a mystery byte already
    // read prior to sending any commands to the target device
    UARTCharGetNonBlocking(((UARTCC26XX_HWAttrs const *)(sblUartHandle->hwAttrs))->baseAddr);
    
    return SBL_SUCCESS;
  }

  return SBL_FAILURE;
}

/**
 * @fn      SBL_TL_getRspACK
 *
 * @brief   Get ACK/NACK response from the target device
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_DEV_ACK, SBL_DEV_NACK, or SBL_FAILURE if neither ACK/NACK
 */
uint8_t SBL_TL_getRspACK(void)
{
  uint8_t rsp[SBL_ACK_SIZE];
  
  UART_read(sblUartHandle, rsp, sizeof(rsp));
  
  if (memcmp(rsp, ACK, sizeof(rsp)))
  {
    return SBL_DEV_ACK;
  }
  else if (memcmp(rsp, NACK, sizeof(rsp)))
  {
    return SBL_DEV_NACK;
  }
  else
  {
    return SBL_FAILURE;
  }
}

/**
 * @fn      SBL_TL_getRsp
 *
 * @brief   Get response message from the target device
 *
 * @param   pData - pointer to byte array to store data
 * @param   maxSize - size of byte array pointed to by pData
 * @param   len - will be set to the length of data written to pData
 *
 * @return  uint8_t - SBL_SUCCESS or SBL_FAILURE
 */
uint8_t SBL_TL_getRsp(uint8_t *pData, uint16_t maxSize, uint16_t *len)
{
  uint8_t hdr[SBL_HDR_SIZE];
  
  // Read Response header
  UART_read(sblUartHandle, hdr, sizeof(hdr));
  
  // Check if length of incoming response is too long
  if (maxSize < (hdr[SBL_HDR_LEN_IDX] - sizeof(hdr)))
  {
    return SBL_FAILURE;
  }

  // Read Response Payload
  UART_read(sblUartHandle, pData,hdr[SBL_HDR_LEN_IDX] - sizeof(hdr));
  
  // Verify Checksum
  if (hdr[SBL_HDR_CKS_IDX] != SBL_TL_CKS(0, pData, hdr[SBL_HDR_LEN_IDX] - sizeof(hdr)))
  {  
    return SBL_FAILURE;
  }

  // Set length parameter to length of payload data
  *len = hdr[SBL_HDR_LEN_IDX];
  
  // Respond with ACK
  return SBL_TL_sendACK(DEVICE_ACK);
}

/**
 * @fn      SBL_TL_sendCmd
 *
 * @brief   Sends a SBL command to target device
 *
 * @param   cmd - command ID
 * @param   pData - pointer to command payload
 * @param   len - length of command payload
 *
 * @return  uint8_t - SBL_SUCCESS
 */
uint8_t SBL_TL_sendCmd(uint8_t cmd, uint8_t *pData, uint16_t len)
{
  uint8_t hdr[SBL_HDR_SIZE + 1]; // Header + CMD byte
  
  // Initialize Header
  hdr[SBL_HDR_LEN_IDX] = len + sizeof(hdr);             // Length
  hdr[SBL_HDR_CKS_IDX] = SBL_TL_CKS(cmd, pData, len);   // Checksum
  hdr[2] = cmd;                                         // Command

  // Send Header
  UART_write(sblUartHandle, hdr, sizeof(hdr));
  
  // Send Packet
  if (len)
  {
    UART_write(sblUartHandle, pData, len);
  }

  return SBL_TL_getRspACK();
}

/**
 * @fn      SBL_TL_sendACK
 *
 * @brief   Send ACK/NACK response to the target device
 *
 * @param   ack - ACK/NACK value
 *
 * @return  uint8_t - SBL_SUCCESS
 */
uint8_t SBL_TL_sendACK(uint8_t ack)
{
  uint8_t rsp[2];
  
  // Initialize response 
  rsp[0] = 0x00; // Must write zero byte first for response
  rsp[1] = ack;
  
  UART_write(sblUartHandle, rsp, sizeof(rsp));
  
  return SBL_SUCCESS;
}

/**
 * @fn      SBL_TL_sendACK
 *
 * @brief   Send baud packet to allow target to auto detect 
 *          baud rate of SBL transmissions
 *
 * @param   ack - ACK/NACK value
 *
 * @return  uint8_t - SBL_SUCCESS
 */
uint8_t SBL_TL_uartAutoBaud(void)
{
  UART_write(sblUartHandle, BAUD, sizeof(BAUD));
  
  return SBL_TL_getRspACK();
}

/**
 * @fn      SBL_TL_close
 *
 * @brief   Close interface to target device
 *
 * @param   None.
 *
 * @return  None.
 */
void SBL_TL_close(void)
{
  UART_close(sblUartHandle);
} 

/**
 * @fn      SBL_TL_CKS
 *
 * @brief   Calculates checksum over a byte array
 *
 * @param   cmd - optional command byte from sending messages
 * @param   pData - pointer to byte array to store data
 * @param   len - length of byte array pointed to by pData
 *
 * @return  uint8_t - checksum
 */
uint8_t SBL_TL_CKS(uint8_t cmd, uint8_t *pData, uint16_t len)
{
  uint8_t checksum = cmd;
  uint16_t i;
  
  for(i = 0; i < len; i++)
  {
    checksum += pData[i];
  }
  
  return checksum;
}
