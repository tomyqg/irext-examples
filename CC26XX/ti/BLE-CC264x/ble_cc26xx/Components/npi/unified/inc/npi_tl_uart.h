//******************************************************************************
//! \file           npi_tl_uart.h
//! \brief          NPI Transport Layer Module for UART
//
//  Revised:        $Date: 2015-01-28 17:22:05 -0800 (Wed, 28 Jan 2015) $
//  Revision:       $Revision: 42106 $
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
#ifndef NPI_TL_UART_H
#define NPI_TL_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************

// ****************************************************************************
// defines
// ****************************************************************************

// UART ISR Buffer define
#define UART_ISR_BUF_SIZE 128
  
// ****************************************************************************
// typedefs
// ****************************************************************************
// -----------------------------------------------------------------------------
//! \brief      Typedef for call back function mechanism to notify NPI TL that
//!             an NPI transaction has occurred
//! \param[in]  rxLen     number of bytes received
//! \param[in]  txLen     number of bytes transmitted      
//!
//! \return     void
// ----------------------------------------------------------------------------- 
typedef void (*npiCB_t)(uint16_t rxLen, uint16_t txLen);

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device.
//!
//! \param[in]  portID		ID value for board specific UART port
//! \param[in]  portParams	Parameters used to initialize UART port
//! \param[in]  npiCBack	Transport Layer call back function  
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITLUART_openTransport(uint8_t portID, UART_Params *portParams, 
                             	    npiCB_t npiCBack);

// -----------------------------------------------------------------------------
//! \brief      This routine closes Transport Layer port
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITLUART_closeTransport(void);

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the UART
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITLUART_readTransport(void);

// -----------------------------------------------------------------------------
//! \brief      This routine initializes and begins a UART transfer 
//!
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint16_t - number of bytes written to transport
// -----------------------------------------------------------------------------
extern uint16_t NPITLUART_writeTransport(uint16_t len);

// -----------------------------------------------------------------------------
//! \brief      This routine stops any pending reads
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITLUART_stopTransfer(void);

// -----------------------------------------------------------------------------
//! \brief      This routine is called from the application context when REM RDY
//!             is de-asserted
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITLUART_handleRemRdyEvent(void);

#ifdef __cplusplus
}
#endif

#endif /* NPI_TL_UART_H */
