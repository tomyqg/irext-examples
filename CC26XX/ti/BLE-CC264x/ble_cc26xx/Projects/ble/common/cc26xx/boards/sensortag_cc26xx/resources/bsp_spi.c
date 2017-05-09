/******************************************************************************
*  Filename:       bsp_spi.c
*  Revised:        $Date$
*  Revision:       $Revision$
*
*  Description:    Layer added on top of RTOS driver for backward 
*                  compatibility with non RTOS SPI driver.
*
*  Copyright (C) 2014 - 2015 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
#ifdef TI_DRIVERS_SPI_INCLUDED

#include <ti/sysbios/family/arm/cc26xx/Power.h>

#include <ti/drivers/spi/SPICC26XXDMA.h>
#include <ti/drivers/dma/UDMACC26XX.h>

#include <driverlib/ssi.h>

#include "Board.h"
#include "bsp_spi.h"
#include "string.h"

//*****************************************************************************
// SPI setup
static SPI_Handle spiHandle = NULL;
static SPI_Params spiParams;

//
//  Write to an SPI device
//
int bspSpiWrite(const uint8_t *buf, size_t length)
{
  SPI_Transaction masterTransaction;
  bool success;
  
  masterTransaction.count  = length;
  masterTransaction.txBuf  = (void*)buf;
  masterTransaction.arg    = NULL;
  masterTransaction.rxBuf  = NULL;
  
  success = SPI_transfer(spiHandle, &masterTransaction);
  
  return success ? 0 : -1;
}

//
//  Read from an SPI device
//
int bspSpiRead(uint8_t *buf, size_t length)
{
  SPI_Transaction masterTransaction;
  bool success;
  
  masterTransaction.count  = length;
  masterTransaction.txBuf  = NULL;
  masterTransaction.arg    = NULL;
  masterTransaction.rxBuf  = buf;
  
  success = SPI_transfer(spiHandle, &masterTransaction);
  
  return success ? 0 : -1;
}

//
//  Write and read from an SPI device
//
int bspSpiWriteRead(uint8_t *buf, uint8_t wlen, uint8_t rlen)
{
  SPI_Transaction masterTransaction;
  bool success;
  
  masterTransaction.count  = wlen + rlen;
  masterTransaction.txBuf  = buf;
  masterTransaction.arg    = NULL;
  masterTransaction.rxBuf  = buf;
  
  success = SPI_transfer(spiHandle, &masterTransaction);
  if (success)
  {
    memcpy(buf,buf+wlen,rlen);
  }
  
  return success ? 0 : -1;
}

//
//  Initialize the SPI communication
//
void bspSpiOpen(void)
{
  if (spiHandle == NULL)
  {
    /*  Configure SPI as master, 4 mHz bit rate*/
    SPI_Params_init(&spiParams);
    spiParams.bitRate = 4000000;
    // spiParams.frameFormat  = SPI_POL1_PHA1; 
    spiParams.mode         = SPI_MASTER;
    spiParams.transferMode = SPI_MODE_BLOCKING; 

    /* Attempt to open SPI. */
    spiHandle = SPI_open(Board_SPI0, &spiParams);
    
    if (spiHandle == NULL) 
    {
      while(1) {
        // wait here forever
      }
    }
  }
}


/* See bsp_spi.h file for description */
void bspSpiFlush(void)
{
    /* make sure SPI hardware module is done  */
    while(SSIBusy(((SPICC26XX_HWAttrs*)spiHandle->hwAttrs)->baseAddr))
    { };
}

void bspSpiFlashSelect(bool select)
{
  if (select)
  {
    PIN_setOutputValue(hGpioPin, Board_SPI_FLASH_CS, 0);
  }
  else
  {
    PIN_setOutputValue(hGpioPin, Board_SPI_FLASH_CS, 1);
  }
}

void bspSpiLcdSelect(bool select)
{
  if (select)
  {
    PIN_setOutputValue(hGpioPin, Board_SPI_DEVPK_CS, 0);
  }
  else
  {
    PIN_setOutputValue(hGpioPin, Board_SPI_DEVPK_CS, 1);
  }
}

/* See bsp_spi.h file for description */
void bspSpiClose(void)
{
  if (spiHandle != NULL)
  {
    SPI_close(spiHandle);
    spiHandle = NULL;
  }
}

#endif
