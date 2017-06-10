//******************************************************************************
//! \file           npi_task.h
//! \brief          NPI is a TI RTOS Application Thread that provides a
//! \brief          common Network Processor Interface framework.
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
#ifndef NPI_TASK_H
#define NPI_TASK_H

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

//! \brief Call back function that a subsystem must register with NPI Task to 
//         receive messages from the Host Processor
typedef void (*npiFromHostCBack_t)(_npiFrame_t *pNPIMsg);

//! \brief Call back function that a subsystem must register with NPI Task to 
//         receive forwarded messages from ICall
typedef void (*npiFromICallCBack_t)(uint8_t *pGenMsg);

typedef struct 
{
  uint16_t              stackSize;      //!< Configurable size of stack for NPI Task
  uint16_t              bufSize;        //!< Buffer size of Tx/Rx Transport layer buffers
  uint32_t              mrdyPinID;      //!< Pin ID Mrdy (only with Power Saving enabled)
  uint32_t              srdyPinID;      //!< Pin ID Srdy (only with Power Saving enabled)
  uint8_t               portType;       //!< NPI_SERIAL_TYPE_[UART,SPI]
  uint8_t               portBoardID;    //!< Board ID for HW, i.e. CC2650_UART0
  npiInterfaceParams    portParams;     //!< Params to initialize NPI port
} NPI_Params;

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      Initialize a NPI_Params struct with default values
//!
//! \param[in]  portType  NPI_SERIAL_TYPE_[UART,SPI]
//! \param[in]  params    Pointer to NPI params to be initialized
//!                     
//! \return     uint8_t   Status NPI_SUCCESS, NPI_TASK_INVALID_PARAMS
// -----------------------------------------------------------------------------
extern uint8_t NPITask_Params_init(uint8_t portType, NPI_Params *params);

// -----------------------------------------------------------------------------
//! \brief      Task creation function for NPI
//!
//! \param[in]  params    Pointer to NPI params which will be used to 
//!                       initialize the NPI Task
//!
//! \return     uint8_t   Status NPI_SUCCESS, or NPI_TASK_FAILURE
// -----------------------------------------------------------------------------
extern uint8_t NPITask_open(NPI_Params *params);

// -----------------------------------------------------------------------------
//! \brief      NPI Task close and tear down. Cannot be used with ICall because
//!             ICall service cannot be un-enrolled
//!
//! \return     uint8_t   Status NPI_SUCCESS, or NPI_TASK_FAILURE
// -----------------------------------------------------------------------------
extern uint8_t NPITask_close(void);

// -----------------------------------------------------------------------------
//! \brief      API for application task to send a message to the Host.
//!             NOTE: It's assumed all message traffic to the stack will use
//!             other (ICALL) APIs/Interfaces.
//!
//! \param[in]  pMsg    Pointer to "unframed" message buffer.
//!
//! \return     uint8_t Status NPI_SUCCESS, or NPI_SS_NOT_FOUND
// -----------------------------------------------------------------------------
extern uint8_t NPITask_sendToHost(_npiFrame_t *pMsg);

// -----------------------------------------------------------------------------
//! \brief      API for subsystems to register for NPI messages received with 
//!             the specific ssID. All NPI messages will be passed to callback
//!             provided
//!
//! \param[in]  ssID    The subsystem ID of NPI messages that should be routed
//!                     to pCB
//! \param[in]  pCB     The call back function that will receive NPI messages
//!
//! \return     uint8_t Status NPI_SUCCESS, or NPI_ROUTING_FULL
// -----------------------------------------------------------------------------
extern uint8_t NPITask_regSSFromHostCB(uint8_t ssID, npiFromHostCBack_t pCB);

// -----------------------------------------------------------------------------
//! \brief      API for subsystems to register for ICall messages received from 
//!             the specific source entity ID. All ICall messages will be passed
//!             to the callback provided
//!
//! \param[in]  icallID Source entity ID whose messages should be sent to pCB
//!             pCB     The call back function that will receive ICall messages
//!
//! \return     uint8_t Status NPI_SUCCESS, or NPI_ROUTING_FULL
// -----------------------------------------------------------------------------
extern uint8_t NPITask_regSSFromICallCB(uint8_t icallID, npiFromICallCBack_t pCB);

// -----------------------------------------------------------------------------
//! \brief      API to allocate an NPI frame of a given data length
//!
//! \param[in]  len             Length of data field of frame
//!
//! \return     _npiFrame_t *   Pointer to newly allocated frame
// -----------------------------------------------------------------------------
extern _npiFrame_t * NPITask_mallocFrame(uint16_t len);

// -----------------------------------------------------------------------------
//! \brief      API to de-allocate an NPI frame
//!
//! \param[in]  frame   Pointer to NPI frame to be de-allocated
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPITask_freeFrame(_npiFrame_t *frame);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // end of NPI_TASK_H definition
