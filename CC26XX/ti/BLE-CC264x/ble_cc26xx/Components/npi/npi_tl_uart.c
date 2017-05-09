//******************************************************************************
//! \file           npi_tl_uart.c
//! \brief          NPI Transport Layer Module for UART
//
//   Revised        $Date: 2015-05-01 12:55:54 -0700 (Fri, 01 May 2015) $
//   Revision:      $Revision: 43630 $
//
//  Copyright 2015 Texas Instruments Incorporated. All rights reserved.
//
// IMPORTANT: Your use of this Software is limited to those specific rights
// granted under the terms of a software license agreement between the user
// who downloaded the software, his/her employer (which must be your employer)
// and Texas Instruments Incorporated (the "License").  You may not use this
// Software unless you agree to abide by the terms of the License. The License
// limits your use, and you acknowledge, that the Software may not be modified,
// copied or distributed unless used solely and exclusively in conjunction with
// a Texas Instruments radio frequency device, which is integrated into
// your product.  Other than for the foregoing purpose, you may not use,
// reproduce, copy, prepare derivative works of, modify, distribute, perform,
// display or sell this Software and/or its documentation for any purpose.
//
//  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
//  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,l
//  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
//  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
//  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
//  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
//  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
//  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
//  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
//  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
//  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
//
//  Should you have any questions regarding your right to use this Software,
//  contact Texas Instruments Incorporated at www.TI.com.
//******************************************************************************

// ****************************************************************************
// includes
// ****************************************************************************
#include <string.h>
#include <xdc/std.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "ICall.h"
#include "Board.h"
#include "hal_types.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>

#include "inc/npi_config.h"
#include "inc/npi_tl_uart.h"
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

#include "_hal_types.h"

// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************
//! \brief UART Handle for UART Driver
static UART_Handle uartHandle;

//! \brief UART ISR Rx Buffer
static Char isrRxBuf[UART_ISR_BUF_SIZE];

//! \brief NPI TL call back function for the end of a UART transaction
static npiCB_t npiTransmitCB = NULL;

#ifdef POWER_SAVING
//! \brief Flag signalling receive in progress
static uint8 RxActive = FALSE;

//! \brief Flag signalling transmit in progress
static uint8 TxActive = FALSE;

//! \brief Value of MRDY NPI TL pin
static uint8 mrdy_flag = 1;
#endif //POWER_SAVING

//! \brief Pointer to NPI TL TX Buffer
static Char* TransportRxBuf;

//! \brief Length of bytes received
static uint16 TransportRxLen = 0;

//! \brief Pointer to NPI TL RX Buffer
static Char* TransportTxBuf;

//! \brief Length of bytes to send from NPI TL Tx Buffer
static uint16 TransportTxLen = 0;

//! \brief UART Object. Initialized in board specific files
extern UARTCC26XX_Object uartCC26XXObjects[];

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief UART ISR function. Invoked upon specific threshold of UART RX FIFO size
static uint16 NPITLUART_readIsrBuf(size_t size);

//! \brief UART Callback invoked after UART write completion
static void NPITLUART_writeCallBack(UART_Handle handle, void *ptr, size_t size);

//! \brief UART Callback invoked after readsize has been read or timeout
static void NPITLUART_readCallBack(UART_Handle handle, void *ptr, size_t size);

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device.
//!
//! \param[in]  tRxBuf - pointer to NPI TL Tx Buffer
//! \param[in]  tTxBuf - pointer to NPI TL Rx Buffer
//! \param[in]  npiCBack - NPI TL call back function to be invoked at the end of 
//!             a UART transaction                     
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLUART_initializeTransport(Char *tRxBuf, Char *tTxBuf, npiCB_t npiCBack)
{
    UART_Params params;

    TransportRxBuf = tRxBuf;
    TransportTxBuf = tTxBuf;
    npiTransmitCB = npiCBack;

    // Configure UART parameters.
    UART_Params_init(&params);
    params.baudRate = NPI_UART_BR;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.dataLength = UART_LEN_8;
    params.stopBits = UART_STOP_ONE;
    params.readMode = UART_MODE_CALLBACK;
    params.writeMode = UART_MODE_CALLBACK;
    params.readEcho = UART_ECHO_OFF;

    params.readCallback = NPITLUART_readCallBack;
    params.writeCallback = NPITLUART_writeCallBack;

    // Open / power on the UART.
    uartHandle = UART_open(Board_UART, &params);
    // Enable Partial Reads on all subsequent UART_read()
    UART_control(uartHandle, UARTCC26XX_CMD_RETURN_PARTIAL_ENABLE,  NULL);

#ifndef POWER_SAVING
    // This call will start repeated Uart Reads when Power Savings is disabled
    NPITLUART_readTransport();
#endif //!POWER_SAVING
    
    return;
}

#ifdef POWER_SAVING
// -----------------------------------------------------------------------------
//! \brief      This routine stops any pending reads
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLUART_stopTransfer(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    mrdy_flag = 1;

    // If we have no bytes in FIFO yet we must assume there was nothing to read
    // or that the FIFO has already been read for this UART_read()
    // In either case UART_readCancel will call the read CB function and it will
    // invoke npiTransmitCB with the appropriate number of bytes read
    if (!UARTCharsAvail(((UARTCC26XX_HWAttrs const *)(uartHandle->hwAttrs))->baseAddr))
    {
        RxActive = FALSE;
        UART_readCancel(uartHandle);
    }

    ICall_leaveCriticalSection(key);
    return;
}
#endif //POWER_SAVING

#ifdef POWER_SAVING
// -----------------------------------------------------------------------------
//! \brief      This routine is called from the application context when MRDY is
//!             de-asserted
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLUART_handleMrdyEvent(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
    
    mrdy_flag = 0;

    // If we haven't already begun reading, now is the time before Master
    //    potentially starts to send data
    // The !TxActive condition is because we will call UART_npiRead() prior to setting
    // TxActive true. There is the possibility that MRDY gets set high which
    // clears RxActive prior to us getting to this event. This will cause us to
    // read twice per transaction which will cause the transaction to never
    // complete
    if ( !RxActive && !TxActive )
    {
        NPITLUART_readTransport();
    }

    // If we have something to write, then the Master has signalled it is ready
    //    to receive. Time to write.
    if ( TxActive )
    {
        // Check to see if transport is successful. If not, reset TxLen to allow
        // another write to be processed
        if ( UART_write(uartHandle, TransportTxBuf, TransportTxLen) == UART_ERROR )
        {
          TxActive = FALSE;
          TransportTxLen = 0;
        }
    }

    ICall_leaveCriticalSection(key);
    
    return;
}
#endif //POWER_SAVING

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on Write completion
//!
//! \param[in]  handle - handle to the UART port
//! \param[in]  ptr    - pointer to data to be transmitted
//! \param[in]  size   - size of the data
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITLUART_writeCallBack(UART_Handle handle, void *ptr, size_t size)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

#ifdef POWER_SAVING
    if ( !RxActive )
    {
        UART_readCancel(uartHandle);
        if ( npiTransmitCB )
        {
            npiTransmitCB(TransportRxLen, TransportTxLen);
        }
    }

    TxActive = FALSE;
    
#else
    if ( npiTransmitCB )
    {
        npiTransmitCB(0, TransportTxLen);
    }
#endif //POWER_SAVING

    ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on Read completion of readSize/receive
//!             timeout
//!
//! \param[in]  handle - handle to the UART port
//! \param[in]  ptr    - pointer to buffer to read data into
//! \param[in]  size   - size of the data
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITLUART_readCallBack(UART_Handle handle, void *ptr, size_t size)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    if (size)
    {
        if (size != NPITLUART_readIsrBuf(size))
        {
            // Buffer overflow imminent. Cancel read and pass to higher layers
            // for handling
#ifdef POWER_SAVING
            RxActive = FALSE;
#endif //POWER_SAVING
            if ( npiTransmitCB )
            {
                npiTransmitCB(NPI_TL_BUF_SIZE, TransportTxLen);
            }
        }
    }

#ifdef POWER_SAVING
    // Read has been cancelled by transport layer, or bus timeout and no bytes in FIFO
    //    - do not invoke another read
    if ( !UARTCharsAvail(((UARTCC26XX_HWAttrs const *)(uartHandle->hwAttrs))->baseAddr) &&
            mrdy_flag )
    {
        RxActive = FALSE;
        
        // If TX has also completed then we are safe to issue call back
        if ( !TxActive && npiTransmitCB )
        {
            npiTransmitCB(TransportRxLen, TransportTxLen);
        }
    }
    else
    {
        UART_read(uartHandle, &isrRxBuf[0], UART_ISR_BUF_SIZE);
    }
#else
    if ( npiTransmitCB )
    {
        npiTransmitCB(size, 0);
    }
    TransportRxLen = 0;
    UART_read(uartHandle, &isrRxBuf[0], UART_ISR_BUF_SIZE);
#endif //POWER_SAVING

    ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer based on len,
//!             and places it into the buffer.
//!
//! \param[in]  size - amount of bytes in UART ISR Rx Buffer
//!
//! \return     uint16 - number of bytes read from transport
// -----------------------------------------------------------------------------
static uint16 NPITLUART_readIsrBuf(size_t size)
{
    uint8_t i = 0;

    // Copy the UART buffer to the application buffer
    // Do not allow overflow of buffer. Instead pass up to NPI module and allow
    // it to handle
    for (; (i < size) && (TransportRxLen < NPI_TL_BUF_SIZE); i++)
    {
        TransportRxBuf[TransportRxLen++] = isrRxBuf[i];
        isrRxBuf[i] = 0;
    }

    return i;
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the UART
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLUART_readTransport(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
    
#ifdef POWER_SAVING
    RxActive = TRUE;
#endif //POWER_SAVING

    TransportRxLen = 0;
    UART_read(uartHandle, &isrRxBuf[0], UART_ISR_BUF_SIZE);
    
    ICall_leaveCriticalSection(key);
}


// -----------------------------------------------------------------------------
//! \brief      This routine writes copies buffer addr to the transport layer.
//!
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint8 - number of bytes written to transport
// -----------------------------------------------------------------------------
uint16 NPITLUART_writeTransport(uint16 len)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
    
    TransportTxLen = len;

#ifdef POWER_SAVING
    TxActive = TRUE;

    // Start reading prior to impending write transaction
    // We can only call UART_write() once MRDY has been signaled from Master
    // device
    NPITLUART_readTransport();
#else
    // Check to see if transport is successful. If not, reset TxLen to allow
    // another write to be processed
    if(UART_write(uartHandle, TransportTxBuf, TransportTxLen) == UART_ERROR )
    {
      TransportTxLen = 0;
    }
#endif //POWER_SAVING
    ICall_leaveCriticalSection(key);
    
    return TransportTxLen;
}
