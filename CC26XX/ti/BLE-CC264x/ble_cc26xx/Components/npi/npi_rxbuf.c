//******************************************************************************
//! \file           npi_rxbuf.c
//! \brief          NPI RX Buffer and utilities
//
//   Revised        $Date: 2015-07-28 14:31:44 -0700 (Tue, 28 Jul 2015) $
//   Revision:      $Revision: 44419 $
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

#include "Board.h"
#include "hal_types.h"
#include "inc/npi_config.h"
#include "inc/npi_tl.h"

// ****************************************************************************
// defines
// ****************************************************************************
#define NPIRXBUF_RXHEAD_INC(x)   RxBufHead += x;               \
                                 RxBufHead %= NPI_TL_BUF_SIZE;

#define NPIRXBUF_RXTAIL_INC(x)   RxBufTail += x;               \
                                 RxBufTail %= NPI_TL_BUF_SIZE;

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//Receive Buffer for all NPI messages
static uint8 RxBuf[NPI_TL_BUF_SIZE];
static uint16 RxBufHead = 0;
static uint16 RxBufTail = 0;

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      NPIRxBuf_Read
//!
//! \param[in]  len - number of bytes to read out of circular buffer
//!
//! \return     uint16 - number of bytes read
// -----------------------------------------------------------------------------
uint16 NPIRxBuf_Read(uint16 len)
{
    uint16 partialLen = 0;

    // Need to make two reads due to wrap around of circular buffer
    if ((len + RxBufTail) > NPI_TL_BUF_SIZE)
    {
        partialLen = NPI_TL_BUF_SIZE - RxBufTail;
        NPITL_readTL(&RxBuf[RxBufTail], partialLen);
        len -= partialLen;
        RxBufTail = 0;
    }

    // Read remainder of data from Transport Layer
    NPITL_readTL(&RxBuf[RxBufTail], len);
    NPIRXBUF_RXTAIL_INC(len);

    // Return len to original size
    len += partialLen;

    return len;
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unparsed in RxBuf
//!
//! \return     uint16 - number of bytes that are unparsed in RxBuf
// -----------------------------------------------------------------------------
uint16 NPIRxBuf_GetRxBufCount(void)
{
    return ((RxBufTail - RxBufHead) + NPI_TL_BUF_SIZE) % NPI_TL_BUF_SIZE;
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are available in RxBuf
//!
//! \return     uint16 - number of bytes that are available in RxBuf
// -----------------------------------------------------------------------------
uint16 NPIRxBuf_GetRxBufAvail(void)
{
    return (NPI_TL_BUF_SIZE - NPIRxBuf_GetRxBufCount());
}

// -----------------------------------------------------------------------------
//! \brief      NPIRxBuf_ReadFromRxBuf
//!
//! \param[in]  buf - pointer to byte array in which to copy bytes read
//! \param[in]  len - number of bytes to read out of circular buffer
//!
//! \return     uint16 - numjer of bytes read
// -----------------------------------------------------------------------------
uint16 NPIRxBuf_ReadFromRxBuf(uint8_t *buf, uint16 len)
{
	uint16_t idx;
    
    for (idx = 0; idx < len; idx++)
    {
        *buf++ = RxBuf[RxBufHead];
        NPIRXBUF_RXHEAD_INC(1)
    }

    return len;
}
