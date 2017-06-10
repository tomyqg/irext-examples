//******************************************************************************
//! \file           npi_tl.h
//! \brief          NPI Transport Layer API
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
#ifndef NPI_TL_H
#define NPI_TL_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************
  
#include "inc/npi_data.h"
#include "hal_types.h"

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(NPI_USE_UART)
#define transportOpen NPITLUART_openTransport
#define transportClose NPITLUART_closeTransport
#define transportRead NPITLUART_readTransport
#define transportWrite NPITLUART_writeTransport
#define transportStopTransfer NPITLUART_stopTransfer
#define transportRemRdyEvent NPITLUART_handleRemRdyEvent
#elif defined(NPI_USE_SPI)
#define transportOpen NPITLSPI_openTransport
#define transportClose NPITLSPI_closeTransport
#define transportRead NPITLSPI_readTransport
#define transportWrite NPITLSPI_writeTransport
#define transportStopTransfer NPITLSPI_stopTransfer
#define transportRemRdyEvent NPITLSPI_handleRemRdyEvent
#endif //NPI_USE_UART

// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief      Typedef for call back function mechanism to notify NPI Task that
//!             an NPI transaction has occurred
typedef void (*npiRtosCB_t)(uint16_t sizeRx, uint16_t sizeTx);

//! \brief      Typedef for call back function mechanism to notify NPI Task that
//!             an Remote Ready edge has occurred
typedef void (*npiMrdyRtosCB_t)(uint8_t state);

//! \brief      Struct for transport layer call backs
typedef struct
{
  npiMrdyRtosCB_t remRdyCB;
  npiRtosCB_t     transCompleteCB;      
} npiTLCallBacks;

typedef struct 
{
  uint16_t              npiTLBufSize;   //!< Buffer size of Tx/Rx Transport layer buffers
  uint32_t              mrdyPinID;      //!< Pin ID Mrdy (only with Power Saving enabled)
  uint32_t              srdyPinID;      //!< Pin ID Srdy (only with Power Saving enabled)
  uint8_t               portType;       //!< NPI_SERIAL_TYPE_[UART,SPI]
  uint8_t               portBoardID;    //!< Board ID for HW, i.e. CC2650_UART0
  npiInterfaceParams    portParams;     //!< Params to initialize NPI port
  npiTLCallBacks        npiCallBacks;   //!< Call backs to NPI Task
} NPITL_Params;

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device. Note that based on project defines, either the
//!             UART, or SPI driver can be used.
//!
//! \param[in]  params - Transport Layer parameters
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_openTL(NPITL_Params *params);

// -----------------------------------------------------------------------------
//! \brief      This routine closes the transport layer
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_closeTL(void);

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer based on len,
//!             and places it into the buffer.
//!
//! \param[out] buf - Pointer to buffer to place read data.
//! \param[in]  len - Number of bytes to read.
//!
//! \return     uint16_t - the number of bytes read from transport
// -----------------------------------------------------------------------------
uint16_t NPITL_readTL(uint8_t *buf, uint16_t len);

// -----------------------------------------------------------------------------
//! \brief      This routine writes data from the buffer to the transport layer.
//!
//! \param[in]  buf - Pointer to buffer to write data from.
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint16_t - NPI Error Code value
// -----------------------------------------------------------------------------
uint8_t NPITL_writeTL(uint8_t *buf, uint16_t len);

// -----------------------------------------------------------------------------
//! \brief      This routine is used to handle an Rem RDY edge from the app
//!             context. Certain operations such as UART_read() cannot be
//!             performed from the actual hwi handler
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_handleRemRdyEvent(void);

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size receive buffer.
//!
//! \return     uint16_t - max size of the receive buffer
// -----------------------------------------------------------------------------
uint16_t NPITL_getMaxRxBufSize(void);

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size transmit buffer.
//!
//! \return     uint16_t - max size of the transmit buffer
// -----------------------------------------------------------------------------
uint16_t NPITL_getMaxTxBufSize(void);

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unread in RxBuf
//!
//! \return     uint16_t - number of unread bytes
// -----------------------------------------------------------------------------
uint16_t NPITL_getRxBufLen(void);

// -----------------------------------------------------------------------------
//! \brief      This routine returns the state of transmission on NPI
//!
//! \return     bool - state of NPI transmission - 1 - active, 0 - not active
// -----------------------------------------------------------------------------
bool NPITL_checkNpiBusy(void);

/*******************************************************************************
 */

#ifdef __cplusplus
}
#endif

#endif /* NPI_TL_H */
