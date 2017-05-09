//******************************************************************************
//! \file           npi_tl.c
//! \brief          NPI Transport Layer API
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

// ****************************************************************************
// includes
// ****************************************************************************
#include <string.h>
#include <xdc/std.h>
#include <ti/sysbios/family/arm/cc26xx/Power.h>
#include <ti/sysbios/family/arm/cc26xx/PowerCC2650.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "Board.h"
#include "ICall.h"
#include "hal_types.h"
#include "inc/npi_tl.h"
#include "inc/npi_config.h"

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(NPI_USE_SPI)
#include "inc/npi_tl_spi.h"
#elif defined(NPI_USE_UART)
#include "inc/npi_tl_uart.h"
#else
#error Must define an underlying serial bus for NPI
#endif


// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//! \brief Flag for low power mode
static volatile bool npiPMSetConstraint = FALSE;

//! \brief Flag for ongoing NPI TX
static volatile bool npiTxActive = FALSE;

//! \brief The packet that was being sent when HWI of MRDY going low was received
static volatile uint32 mrdyPktStamp = 0;

//! \brief Packets transmitted counter
static uint32 txPktCount = 0;

//! \brief NPI Transport Layer receive buffer
static Char npiRxBuf[NPI_TL_BUF_SIZE];

//! \brief Index to last byte written into NPI Transport Layer receive buffer
static uint16_t npiRxBufTail = 0;

//! \brief Index to first byte to be read from NPI Transport Layer receive buffer
static uint16_t npiRxBufHead = 0;

//! \brief NPI Transport Layer transmit buffer
static Char npiTxBuf[NPI_TL_BUF_SIZE];

//! \brief Number of bytes in NPI Transport Layer transmit buffer
static uint16_t npiTxBufLen = 0;

//! \brief Call back function in NPI Task for transmit complete
static npiRtosCB_t taskTxCB = NULL;

//! \brief Call back function in NPI Task for receive complete
static npiRtosCB_t taskRxCB = NULL;

//! \brief The remainder of any message that is fragmented
static uint8 *msgFrag = NULL;

//! \brief The length of the remaining message fragment
static uint16 msgFragLen = 0;

#ifdef POWER_SAVING
//! \brief Call back function in NPI Task for MRDY hardware interrupt
static npiMrdyRtosCB_t taskMrdyCB = NULL;

//! \brief PIN Config for Mrdy and Srdy signals
static PIN_Config npiHandshakePinsCfg[] =
{
    MRDY_PIN | PIN_GPIO_OUTPUT_DIS | PIN_INPUT_EN | PIN_PULLUP,
    SRDY_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

//! \brief PIN State for Mrdy and Srdy signals
static PIN_State npiHandshakePins;

//! \brief PIN Handles for Mrdy and Srdy signals
static PIN_Handle hNpiHandshakePins;

//! \brief No way to detect whether positive or negative edge with PIN Driver
//!             Use a flag to keep track of state
static uint8 mrdy_state;
#endif

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief Call back function provided to underlying serial interface to be
//              invoked upon the completion of a transmission
static void NPITL_transmissionCallBack(uint16 Rxlen, uint16 Txlen);

#ifdef POWER_SAVING
//! \brief HWI interrupt function for MRDY
static void NPITL_MRDYPinHwiFxn(PIN_Handle hPin, PIN_Id pinId);
#endif

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device. Note that based on project defines, either the
//!             UART, or SPI driver can be used.
//!
//! \param[in]  npiCBTx - Call back function for TX complete event
//! \param[in]  npiCBRx - Call back function for RX event
//! \param[in]  npiCBMrdy - Call back function for MRDY event
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_initTL(npiRtosCB_t npiCBTx, npiRtosCB_t npiCBRx, npiRtosCB_t npiCBMrdy)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
    
    taskTxCB = npiCBTx;
    taskRxCB = npiCBRx;
#ifdef POWER_SAVING
    taskMrdyCB = npiCBMrdy;
#endif

    transportInit(npiRxBuf,npiTxBuf, NPITL_transmissionCallBack);

#ifdef POWER_SAVING
    SRDY_DISABLE();

    // Initialize SRDY/MRDY. Enable int after callback registered
    hNpiHandshakePins = PIN_open(&npiHandshakePins, npiHandshakePinsCfg);
    PIN_registerIntCb(hNpiHandshakePins, NPITL_MRDYPinHwiFxn);
    PIN_setConfig(hNpiHandshakePins, PIN_BM_IRQ, MRDY_PIN | PIN_IRQ_BOTHEDGES);

    // Enable wakeup
    PIN_setConfig(hNpiHandshakePins, PINCC26XX_BM_WAKEUP, MRDY_PIN | PINCC26XX_WAKEUP_NEGEDGE);
    
    mrdy_state = PIN_getInputValue(MRDY_PIN);
#endif // ! POWER_SAVING

    ICall_leaveCriticalSection(key);
    
    return;
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the state of transmission on NPI
//!
//! \return     bool - state of NPI transmission - 1 - active, 0 - not active
// -----------------------------------------------------------------------------
bool NPITL_checkNpiBusy(void)
{
#ifdef POWER_SAVING

    return !PIN_getOutputValue(SRDY_PIN);
#else
    return npiTxActive;
#endif
}

#ifdef POWER_SAVING

// -----------------------------------------------------------------------------
//! \brief      This routine is used to set constraints on power manager
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_setPM(void)
{
    if( npiPMSetConstraint )
    {
        return;
    }
    // set constraints for Standby and idle mode
    Power_setConstraint(Power_SB_DISALLOW);
    Power_setConstraint(Power_IDLE_PD_DISALLOW);
    npiPMSetConstraint = TRUE;
}
#endif

#ifdef POWER_SAVING
// -----------------------------------------------------------------------------
//! \brief      This routine is used to release constraints on power manager
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_relPM(void)
{
    if ( ! npiPMSetConstraint )
    {
        return;
    }
    // release constraints for Standby and idle mode
    Power_releaseConstraint(Power_SB_DISALLOW);
    Power_releaseConstraint(Power_IDLE_PD_DISALLOW);
    npiPMSetConstraint = FALSE;
}
#endif

#ifdef POWER_SAVING
// -----------------------------------------------------------------------------
//! \brief      This routine is used to handle an MRDY transition from a task
//!             context. Certain operations such as UART_read() cannot be
//!             performed from a HWI context
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_handleMrdyEvent(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
  
    // Check to make sure this event is not occurring during the next packet
    // transmission
    if ( PIN_getInputValue(MRDY_PIN) == 0 || 
        (npiTxActive && mrdyPktStamp == txPktCount ) )
    {
        transportMrdyEvent();
        SRDY_ENABLE();
    }
    
    ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This is a HWI function handler for the MRDY pin. Some MRDY
//!             functionality can execute from this HWI context. Others
//!             must be executed from task context hence the taskMrdyCB()
//!
//! \param[in]  hPin - PIN Handle
//! \param[in]  pinId - ID of pin that triggered HWI
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_MRDYPinHwiFxn(PIN_Handle hPin, PIN_Id pinId)
{
    // The pin driver does not currently support returning whether the int
    // was neg or pos edge so we must use a variable to keep track of state. 
    // If the physical state of the pin was used then a very quick toggle of
    // of MRDY could be missed.
    mrdy_state ^= 1;
    
    if (mrdy_state == 0)
    {
        mrdyPktStamp = txPktCount;
        NPITL_setPM();
        if ( taskMrdyCB )
        {
            taskMrdyCB();
        }
    }
    else
    {
        transportStopTransfer();
    }
    
    // Check the physical state of the pin to see if it matches the variable 
    // state. If not then edge has been missed
    if (mrdy_state != PIN_getInputValue(MRDY_PIN))
    {
        mrdy_state = PIN_getInputValue(MRDY_PIN);
        
        if (mrdy_state == 0)
        {
            mrdyPktStamp = txPktCount;
            NPITL_setPM();

            if (taskMrdyCB)
            {
                taskMrdyCB();
            }
        }
        else
        {
            transportStopTransfer();
        }
    }
}
#endif

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on the completion of one transmission
//!             to/from the host MCU. Any bytes receives will be [0,Rxlen) in
//!             npiRxBuf.
//!             If bytes were receives or transmitted, this function notifies
//!             the NPI task via registered call backs
//!
//! \param[in]  Rxlen   - lenth of the data received
//! \param[in]  Txlen   - length of the data transferred
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_transmissionCallBack(uint16 Rxlen, uint16 Txlen)
{
    npiRxBufHead = 0;
    npiRxBufTail = Rxlen;

    if(Rxlen)
    {
        if ( taskRxCB )
        {
            taskRxCB(Rxlen);
        }
    }
    if(Txlen)
    {
        npiTxActive = FALSE;
        // Only perform call back if NPI Task has been registered
        // and if there is not another fragment to send of this message
        if ( taskTxCB && !msgFragLen )
        {
            taskTxCB(Txlen);
        }
    }

#ifdef POWER_SAVING
    // Reset mrdy state in case of missed HWI
    mrdy_state = PIN_getInputValue(MRDY_PIN);
    
    NPITL_relPM();
    SRDY_DISABLE();
#endif
    
    // If there is another fragment to send, begin write without notifying 
    // higher level tasks
    if ( msgFragLen )
    {
        NPITL_writeTL(msgFrag,msgFragLen);
    }
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer based on len,
//!             and places it into the buffer.
//!
//! \param[in]  buf - Pointer to buffer to place read data.
//! \param[in]  len - Number of bytes to read.
//!
//! \return     uint16 - the number of bytes read from transport
// -----------------------------------------------------------------------------
uint16 NPITL_readTL(uint8 *buf, uint16 len)
{
    // Only copy the lowest number between len and bytes remaining in buffer
    len = (len > NPITL_getRxBufLen()) ? NPITL_getRxBufLen() : len;

    memcpy(buf, &npiRxBuf[npiRxBufHead], len);
    npiRxBufHead += len;
    return len;
}

// -----------------------------------------------------------------------------
//! \brief      This routine writes data from the buffer to the transport layer.
//!
//! \param[in]  buf - Pointer to buffer to write data from.
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint16 - the number of bytes written to transport
// -----------------------------------------------------------------------------
uint16 NPITL_writeTL(uint8 *buf, uint16 len)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();
    
    // Writes are atomic at transport layer
    if ( NPITL_checkNpiBusy() )
    {
        ICall_leaveCriticalSection(key);
        return 0;
    }
    
    // If len of message is greater than fragment size
    // then message must be sent over the span of multiple 
    // fragments
    if ( len > NPI_MAX_FRAG_SIZE )
    {
      msgFrag = buf + NPI_MAX_FRAG_SIZE;
      msgFragLen = len - NPI_MAX_FRAG_SIZE;
      len = NPI_MAX_FRAG_SIZE;
    }
    else
    {
      msgFrag = NULL;
      msgFragLen = 0;
    }

    memcpy(npiTxBuf, buf, len);
    npiTxBufLen = len;
    npiTxActive = TRUE;
    txPktCount++;

    len = transportWrite(npiTxBufLen);

#ifdef POWER_SAVING
    SRDY_ENABLE();
#endif
    
    ICall_leaveCriticalSection(key);

    return len;
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size receive buffer.
//!
//! \return     uint16 - max size of the receive buffer
// -----------------------------------------------------------------------------
uint16 NPITL_getMaxRxBufSize(void)
{
    return(NPI_TL_BUF_SIZE);
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size transmit buffer.
//!
//! \return     uint16 - max size of the transmit buffer
// -----------------------------------------------------------------------------
uint16 NPITL_getMaxTxBufSize(void)
{
    return(NPI_TL_BUF_SIZE);
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unread in RxBuf
//!
//! \return     uint16 - number of unread bytes
// -----------------------------------------------------------------------------
uint16 NPITL_getRxBufLen(void)
{
    return ((npiRxBufTail - npiRxBufHead) + NPI_TL_BUF_SIZE) % NPI_TL_BUF_SIZE;
}
