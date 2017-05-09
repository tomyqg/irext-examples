//******************************************************************************
//! \file           npi_task.h
//! \brief          NPI is a TI RTOS Application Thread that provides a
//! \brief          common Network Processor Interface framework.
//
//   Revised        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
//   Revision:      $Revision: 44375 $
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
#ifndef _NPITASK_H
#define _NPITASK_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************
#include "npi_data.h"

// ****************************************************************************
// defines
// ****************************************************************************


// ****************************************************************************
// typedefs
// ****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      Typedef for call back function mechanism to reroute incoming NPI
//!             messages.
//!             NOTE: Definer MUST copy contents to local buffer.  NPI task will
//!             free this memory.
//!             NOTE: The contained message buffer does NOT include any "framing"
//!             bytes, ie. SOF, FCS etc.
//! \param[in]  pMsg   Pointer to "unframed" message buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
typedef void (*npiIncomingEventCBack_t)(uint8_t *pMsg);

//! \brief      numeration used to determine the type of NPI Rerouting requested
//!             by the application.
//!
//!             NONE: no rerouting despite callback non-null registered callback
//!                   func.
//!             ECHO: Sends a copy of the NPI message to both the registered
//!                   application callback as well as to the stack task.
//!             INTERCEPT: Sends the NPI message to the via the registered callback
//!                        and NOT to the stack task.
//!
typedef enum NPIEventRerouteType { NONE,
                                   ECHO,
                                   INTERCEPT } NPI_IncomingNPIEventRerouteType;



//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      NPI task creation function
//!
//! \return     void
// -----------------------------------------------------------------------------
Void NPITask_createTask(uint32_t stackID);

// -----------------------------------------------------------------------------
//! \brief      Register callback function to reroute incoming (from host)
//!             NPI messages.
//!
//! \param[in]  appRxCB   Callback function.
//! \param[in]  reRouteType Type of re-routing requested
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITask_registerIncomingRXEventAppCB(npiIncomingEventCBack_t appRxCB,
                                                 NPI_IncomingNPIEventRerouteType reRouteType);

// -----------------------------------------------------------------------------
//! \brief      Register callback function to reroute outgoing (from stack)
//!             NPI messages.
//!
//! \param[in]  appTxCB   Callback function.
//! \param[in]  reRouteType Type of re-routing requested
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITask_registerIncomingTXEventAppCB(npiIncomingEventCBack_t appTxCB,
                                                 NPI_IncomingNPIEventRerouteType reRouteType);

// -----------------------------------------------------------------------------
//! \brief      API for application task to send a message to the Host.
//!             NOTE: It's assumed all message traffic to the stack will use
//!             other (ICALL) APIs/Interfaces.
//!
//! \param[in]  pMsg    Pointer to "unframed" message buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITask_sendToHost(uint8_t *pMsg);


#ifdef __cplusplus
{
#endif // extern "C"

#endif // end of _NPITASK_H definition
