//******************************************************************************
//! \file           npi_util.h
//! \brief          NPI Utilities
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
#ifndef NPI_UTIL_H
#define NPI_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************
  
#ifdef USE_ICALL
#include "ICall.h"
#else
#include <stdlib.h>
#endif //USE_ICALL
  
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Semaphore.h>
  
// ****************************************************************************
// defines
// ****************************************************************************
  
#ifdef USE_ICALL
#define NPIUTIL_MALLOC(size)    ICall_malloc(size)
#define NPIUTIL_FREE(pMsg)      ICall_free(pMsg)
#else
#define NPIUTIL_MALLOC(size)    malloc(size)
#define NPIUTIL_FREE(pMsg)      free(pMsg)
#endif
  
// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief Keys used to enter and exit critical sections
typedef struct _npiCSKey_t
{
    uint_least16_t hwikey;
    uint_least16_t taskkey;  
} _npiCSKey_t;

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      Critical section entrance. Disables Tasks and HWI
//!
//! \return     _npiCSKey_t 	CS Key used to later exit CS
// -----------------------------------------------------------------------------
extern _npiCSKey_t NPIUtil_EnterCS(void);

// -----------------------------------------------------------------------------
//! \brief      Critical section exit. Enables Tasks and HWI
//!
//! \param		key 	key obtained with corresponding call to EnterCS()
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIUtil_ExitCS(_npiCSKey_t key);

// -----------------------------------------------------------------------------
//! \brief   Initialize an RTOS queue to hold messages to be processed.
//!
//! \param   pQueue - pointer to queue instance structure.
//!
//! \return  A queue handle.
// -----------------------------------------------------------------------------
extern Queue_Handle NPIUtil_constructQueue(Queue_Struct *pQueue);

// -----------------------------------------------------------------------------
//! \brief   Creates a queue node and puts the node in RTOS queue.
//!
//! \param   msgQueue - queue handle.
//! \param   sem - thread's event processing semaphore that queue is
//!                associated with.
//! \param   pMsg - pointer to message to be queued
//!
//! \return  TRUE if message was queued, FALSE otherwise.
// -----------------------------------------------------------------------------
extern uint8_t NPIUtil_enqueueMsg(Queue_Handle msgQueue, Semaphore_Handle sem,
                                  uint8_t *pMsg);

// -----------------------------------------------------------------------------
//! \brief   Dequeues the message from the RTOS queue.
//!
//! \param   msgQueue - queue handle.
//!
//! \return  pointer to dequeued message, NULL otherwise.
// -----------------------------------------------------------------------------
extern uint8_t * NPIUtil_dequeueMsg(Queue_Handle msgQueue);

#ifdef __cplusplus
}
#endif

#endif /* NPI_UTIL_H */
