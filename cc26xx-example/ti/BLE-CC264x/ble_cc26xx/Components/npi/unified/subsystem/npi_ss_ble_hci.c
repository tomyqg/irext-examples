//******************************************************************************
//! \file           npi_ss_ble_hci.c
//! \brief          NPI BLE HCI Subsystem
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

// ****************************************************************************
// includes
// ****************************************************************************

#include <inc/npi_data.h>
#include <inc/npi_task.h>
#include <inc/npi_ble.h>
#include <subsystem/npi_ss_ble_hci.h>

// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

extern ICall_EntityID npiAppEntityID;

//*****************************************************************************
// function prototypes
//*****************************************************************************

void NPISS_BLE_HCI_msgFromHost(_npiFrame_t *pNPIMsg);

void NPISS_BLE_HCI_msgFromICall(uint8_t *pGenMsg);

// -----------------------------------------------------------------------------
//! \brief      NPI BLE Subsystem initialization function
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPISS_BLE_HCI_init(void)
{
    // Send BLE Stack the NPI Task Entity ID
    NPI_RegisterTask(npiAppEntityID);  
  
    // Register for messages from Host with RPC_SYS_BLE ssID
    NPITask_regSSFromHostCB(RPC_SYS_BLE_HCI,NPISS_BLE_HCI_msgFromHost);
    
    // Register for messages from ICall
    NPITask_regSSFromICallCB(ICALL_SERVICE_CLASS_BLE,
                             NPISS_BLE_HCI_msgFromICall);
}

// -----------------------------------------------------------------------------
//! \brief      Call back function provided to NPI Task. All incoming NPI 
//!             received by NPI Task with the subsystem ID of this subsystem
//!             will be sent to this call back through the NPI routing system
//!
//!             *** This function MUST free pNPIMsg
//!
//! \param[in]  pNPIMsg    Pointer to a "framed" NPI message
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPISS_BLE_HCI_msgFromHost(_npiFrame_t *pNPIMsg)
{
    // Free NPI Frame after use
    NPITask_freeFrame(pNPIMsg);
}

// -----------------------------------------------------------------------------
//! \brief      Call back function provided to NPI Task. All incoming ICall 
//!             messages received by NPI Task from the src ID provided to NPI
//!             will be sent to this call back through the NPI routing system
//!
//!             *** This function MUST free pGenMsg
//!
//! \param[in]  pGenMsg    Pointer to a generic Icall msg
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPISS_BLE_HCI_msgFromICall(uint8_t *pGenMsg)
{
    // Free ICall Msg after use
    ICall_free(pGenMsg);
}