/*******************************************************************************
  Filename:       npi.c
  Revised:        $Date: 2008-06-11 14:30:47 -0700 (Wed, 11 Jun 2008) $
  Revision:       $Revision: 17210 $

  Description:    This file contains the Network Processor Interface (NPI),
                  which abstracts the physical link between the Application
                  Processor (AP) and the Network Processor (NP). The NPI
                  serves as the HAL's client for the SPI and UART drivers, and
                  provides API and callback services for its client.

  Copyright 2006-2015 Texas Instruments Incorporated. All rights reserved.

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

#include "hal_types.h"
#include "inc/npi_ble.h"
#include "npi.h"
#include "OSAL.h"
#include "bcomdef.h"

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
static uint8_t uartAppTaskId = INVALID_TASK_ID;

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
 * @fn          NPI_InitTransport
 *
 * @brief       This routine initializes the transport layer.
 *
 * input parameters
 *
 * @param       npiCallback - pointer to callback
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the number of bytes written to transport.
 */
void NPI_InitTransport(npiCBack_t npiCallback)
{
  //Do something
}



/*******************************************************************************
 * @fn          NPI_RxBufLen
 *
 * @brief       This routine returns the number of bytes in the receive buffer.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the number of bytes in the receive buffer.
 */
uint16 NPI_RxBufLen( void )
{
  return 0;
}

/*******************************************************************************
 * @fn          NPI_ReadTransport
 *
 * @brief       This routine reads data from the transport layer based on len,
 *              and places it into the buffer.
 *
 * input parameters
 *
 * @param       buf - Pointer to buffer to place read data.
 * @param       len - Number of bytes to read.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the number of bytes read from transport.
 */
uint16 NPI_ReadTransport( uint8 *buf, uint16 len )
{
  return 0;
}

/*******************************************************************************
 * @fn          NPI_WriteTransport
 *
 * @brief       This routine writes data from the buffer to the transport layer.
 *
 * input parameters
 *
 * @param       buf - Pointer to buffer to write data from.
 * @param       len - Number of bytes to write.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the number of bytes written to transport.
 */
uint16 NPI_WriteTransport( uint8 *buf, uint16 len )
{
  npiPkt_t *pMsg;

  // check if the Task Id is valid
  if ( uartAppTaskId != INVALID_TASK_ID )
  {
    //Create an OSAL message to copy the buf
    pMsg = (npiPkt_t *)osal_msg_allocate(sizeof(npiPkt_t) + len);

    if (pMsg)
    {
      pMsg->hdr.event = *buf; //Has the event status code in first byte of payload
      pMsg->hdr.status = 0xFF;
      pMsg->pktLen = len;
      pMsg->pData  = (uint8 *)(pMsg + 1);

      osal_memcpy(pMsg->pData, buf, len);

      // Send message using uartAppTaskId
      osal_msg_send(uartAppTaskId, (uint8 *)pMsg); //If there is no space, app has to queue it

      return len;
    }
  }

  return 0;
}

/*******************************************************************************
 * @fn          NPI_GetMaxRxBufSize
 *
 * @brief       This routine returns the max size receive buffer.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the max size of the receive buffer.
 */
uint16 NPI_GetMaxRxBufSize( void )
{
  return( NPI_UART_RX_BUF_SIZE );
}


/*******************************************************************************
 * @fn          NPI_GetMaxTxBufSize
 *
 * @brief       This routine returns the max size transmit buffer.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Returns the max size of the transmit buffer.
 */
uint16 NPI_GetMaxTxBufSize( void )
{
  return( NPI_UART_TX_BUF_SIZE );
}

/*******************************************************************************
 * @fn          NPI_RegisterTask
 *
 * @brief       This routine sends the taskID for the UART task to NPI
 *
 * input parameters
 *
 * @param       taskID - for the UART app task
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void NPI_RegisterTask( uint8_t taskId )
{
  uartAppTaskId = taskId;
  return;
}

/*******************************************************************************
 ******************************************************************************/
