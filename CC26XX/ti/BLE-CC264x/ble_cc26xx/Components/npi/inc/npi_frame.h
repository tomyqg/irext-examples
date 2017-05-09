//*****************************************************************************
//! \file           npi_frame.h
//! \brief          This file contains the Network Processor Interface (NPI)
//!                 data frame specific functions definitions.
//
//   Revised        $Date: 2015-01-29 11:51:00 -0800 (Thu, 29 Jan 2015) $
//   Revision:      $Revision: 42121 $
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
//*****************************************************************************
#ifndef NPIFRAME_H
#define NPIFRAME_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************
#include <inc/hw_types.h>
#include "npi_data.h"

// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief typedef for call back function to return a complete NPI message.
//!        The npiFrame module encapsulates the collecting/parsing of the
//!        complete message and returns via this callback the received message.
//!        NOTE: the message buffer does NOT include the framing elements
//!        (i.e. Start of Frame, FCS/CRC or similar).
typedef void (*npiIncomingFrameCBack_t)( uint8_t frameSize, uint8_t *pFrame,
                                         NPIMSG_Type msgType );


//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************
// ----------------------------------------------------------------------------
//! \brief      Initialize Frame module with NPI callbacks.
//!
//! \param[in]  incomingFrameCB   Call back for complete inbound (from host)
//!                               messages
//!
//! \return     void
// ----------------------------------------------------------------------------
extern void NPIFrame_initialize(npiIncomingFrameCBack_t incomingFrameCB);


// ----------------------------------------------------------------------------
//! \brief      Bundles message into Transport Layer frame and NPIMSG_msg_t
//!             container.  A transport layer specific version of this function
//!             must be implemented.
//!
//! \param[in]  pData     Pointer to message buffer.
//!
//! \return     void
// ----------------------------------------------------------------------------
extern NPIMSG_msg_t * NPIFrame_frameMsg(uint8_t *pIncomingMsg);

// ----------------------------------------------------------------------------
//! \brief      Collects serial message buffer.  Called based on events 
//!             received from the transport layer.  When an entire message has 
//!             been successfully received, it is passed back to NPI task via 
//!             the callback function above: npiIncomingFrameCBack_t.
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIFrame_collectFrameData(void);

#ifdef __cplusplus
}
#endif

#endif /* NPIFRAME_H */
