//******************************************************************************
//! \file           npi_tl_spi.c
//! \brief          NPI Transport Layer Module for SPI
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
#include <string.h>
#include <xdc/std.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "Board.h"
#include "hal_types.h"
#include "ICall.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>

#include "inc/npi_tl.h"
#include "inc/npi_tl_spi.h"
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26XXDMA.h>

// ****************************************************************************
// defines
// ****************************************************************************

#define SPI_TX_FIELD_LEN                        NPI_SPI_HDR_LEN
#define SPI_TX_HDR_LEN                          3
#define SPI_TX_ZERO_PAD_INDEX                   0        
#define SPI_TX_SOF_INDEX                        1
#define SPI_TX_LEN_INDEX                        2

//! \brief NPI SPI Message Indexes and Constants
//
#define NPI_SPI_MSG_LEN_LSB_IDX                     0x01
#define NPI_SPI_MSG_LEN_MSB_IDX                     0x02
#define NPI_SPI_MSG_HDR_LEN                         0x05
#define NPI_SPI_MSG_HDR_NOSOF_LEN                   0x04
#define NPI_SPI_MSG_SOF                             0xFE
#define NPI_SPI_MSG_SOF_IDX                         0x00
#define ZERO_PAD                                    1

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//! \brief Handle for SPI object used by SPI driver
static SPI_Handle spiHandle;

//! \brief Structure that defines parameters of one SPI transaction
static SPI_Transaction spiTransaction;

//! \brief NPI TL call back function for the end of a SPI transaction
static npiCB_t npiTransmitCB = NULL;

//! \brief Length of bytes to send from NPI TL Tx Buffer
static uint16_t npiTxBufLen = 0;

//! \brief SPI Object. Initialized in board specific files
extern SPICC26XX_Object spiCC26XXDMAObjects[];

//! \brief NPI Transport Layer Buffer variables defined in npi_tl.c
extern uint8_t * npiRxBuf;
extern uint8_t * npiTxBuf;
extern uint16_t npiBufSize;

//! \brief Flag signalling receive in progress
static uint8 RxActive = FALSE;

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief Call back function invoked at the end of a SPI transaction
static void NPITLSPI_CallBack(SPI_Handle handle, SPI_Transaction  *objTransaction);

//! \brief Calculate FCS field of SPI Transaction frame
static uint8_t NPITLSPI_calcFCS(uint8_t *buf, uint16_t len);

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device.
//!
//! \param[in]  portID      ID value for board specific SPI port
//! \param[in]  portParams  Parameters used to initialize SPI port
//! \param[in]  npiCBack    Transport Layer call back function    
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLSPI_openTransport(uint8_t portID, SPI_Params *portParams,
                            npiCB_t npiCBack)
{
    npiTransmitCB = npiCBack;

    // Add call backs SPI parameters.
    portParams->transferMode = SPI_MODE_CALLBACK;
    portParams->transferCallbackFxn = NPITLSPI_CallBack;

    // Attempt to open SPI
    spiHandle = SPI_open(portID, portParams);
}

// -----------------------------------------------------------------------------
//! \brief      This routine closes Transport Layer port           
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLSPI_closeTransport(void)
{
    SPI_close(spiHandle);
}

// -----------------------------------------------------------------------------
//! \brief      This routine stops any pending reads
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLSPI_stopTransfer(void)
{
    SPI_transferCancel(spiHandle);
}

// -----------------------------------------------------------------------------
//! \brief      This routine is called from the application context when REM RDY
//!             is de-asserted
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLSPI_handleRemRdyEvent(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    // If write has not be set up then a read must occur during this
    // transaction. There is a possibility that after bidirectional 
    // transaction there is an extra MRDY event. This event 
    // could cause a double read (which would clear the RX Buffer) so this 
    // check ignores the MRDY event if Rx is already in progress
    if (!npiTxBufLen && !RxActive)
    {
      NPITLSPI_readTransport();
    }

    ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on transmission completion
//!
//! \param[in]  handle - handle to the SPI port
//! \param[in]  objTransaction    - handle for SPI transmission
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITLSPI_CallBack(SPI_Handle handle, SPI_Transaction *objTransaction)
{
    uint16_t i;
    uint16_t readLen = 0;
    uint16_t storeTxLen = npiTxBufLen;
    
    // Check if a valid packet was found
    // SOF:
    if (npiRxBuf[NPI_SPI_MSG_SOF_IDX] == NPI_SPI_MSG_SOF &&
            objTransaction->count)
    {
        // Length:
        readLen = npiRxBuf[NPI_SPI_MSG_LEN_LSB_IDX];
        readLen += ((uint16)npiRxBuf[NPI_SPI_MSG_LEN_MSB_IDX] << 8);
        readLen += NPI_SPI_MSG_HDR_NOSOF_LEN; // Include the header w/o SOF
        
        // FCS:
        if (npiRxBuf[readLen + 1] == 
              NPITLSPI_calcFCS(&npiRxBuf[NPI_SPI_MSG_LEN_LSB_IDX],readLen))
        {
            // Message is valid. Shift bytes to remove SOF
            for (i = 0 ; i < readLen; i++)
            {
                npiRxBuf[i] = npiRxBuf[i + 1];
            }
        }
        else
        {
            // Invalid FCS. Discard message
            readLen = 0;
        }
    }
    
     //All bytes in TxBuf must be sent by this point
    npiTxBufLen = 0; 
    RxActive = FALSE;  
    
    if (npiTransmitCB)
    {
        npiTransmitCB(readLen,storeTxLen);
    }
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer
//!             and places it into the buffer.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLSPI_readTransport(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    npiTxBufLen = 0;
    RxActive = TRUE;

    // Clear DMA Rx buffer and clear extra Tx buffer bytes to ensure clean buffer 
    //    for next RX/TX
    memset(npiRxBuf, 0, npiBufSize);
    memset(&npiTxBuf[npiTxBufLen], 0, npiBufSize - npiTxBufLen);

    // set up the SPI Transaction
    spiTransaction.txBuf = npiTxBuf;
    spiTransaction.rxBuf = npiRxBuf;
    spiTransaction.count = npiBufSize;
    SPI_transfer(spiHandle, &spiTransaction);

    ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This routine initializes and begins a SPI transaction
//!
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint16_t - number of bytes written to transport
// -----------------------------------------------------------------------------
uint16_t NPITLSPI_writeTransport(uint16_t len)
{
    int16 i = 0;
    ICall_CSState key;
    
    key = ICall_enterCriticalSection();

    // Shift all bytes in TX Buffer up ZERO_PAD indexes. SPI Driver clips off 
    // first byte
    // NPI TL already shifts bytes up 1 index for SOF before calling write()
    for (i = len + 1; i > 0; i--)
    {
        npiTxBuf[i + ZERO_PAD] = npiTxBuf[i];
    }
    
    // Clear zero pad bytes
    memset(npiTxBuf, 0, ZERO_PAD);
    
    npiTxBuf[NPI_SPI_MSG_SOF_IDX + ZERO_PAD] = NPI_SPI_MSG_SOF;
    npiTxBuf[len + ZERO_PAD + 1] = NPITLSPI_calcFCS((uint8_t *)&npiTxBuf[2],len);
    npiTxBufLen = len + ZERO_PAD + 2; // 2 = SOF + FCS 
    
    // Clear DMA Rx buffer and clear extra Tx buffer bytes to ensure clean buffer
    //    for next RX/TX
    memset(npiRxBuf, 0, npiBufSize);
    memset(&npiTxBuf[npiTxBufLen], 0, npiBufSize - npiTxBufLen);

    // set up the SPI Transaction
    spiTransaction.count = npiBufSize;
    spiTransaction.txBuf = npiTxBuf;
    spiTransaction.rxBuf = npiRxBuf;
    
    // Check to see if transport is successful. If not, reset TxBufLen to allow
    // another write to be processed
    if (!SPI_transfer(spiHandle, &spiTransaction))
    {
      npiTxBufLen = 0;
    }

    ICall_leaveCriticalSection(key);

    return npiTxBufLen;
}

// ----------------------------------------------------------------------------
//! \brief      Calculate the FCS of a message buffer by XOR'ing each byte.
//!             Remember to NOT include SOP and FCS fields, so start at the CMD
//!             field.
//!
//! \param[in]  msg_ptr   message pointer
//! \param[in]  len       length of message
//!
//! \return     uint8_t   fcs value 
// ----------------------------------------------------------------------------
uint8_t NPITLSPI_calcFCS(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint8_t fcs = 0;
    
    for (i = 0; i < len; i++)
    {
        fcs ^= buf[i];
    }
    
    return fcs;
}
