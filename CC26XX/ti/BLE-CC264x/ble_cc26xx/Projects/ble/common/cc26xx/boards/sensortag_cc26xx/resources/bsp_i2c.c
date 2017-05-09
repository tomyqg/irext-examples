/******************************************************************************
*  Filename:       bsp_i2c.c
*  Revised:        $Date: $
*  Revision:       $Revision: $
*
*  Description:    Layer added on top of RTOS driver for backward 
*                  compatibility with non RTOS I2C driver.
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
*
******************************************************************************/
#ifdef TI_DRIVERS_I2C_INCLUDED

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/family/arm/cc26xx/Power.h>

#include <ti/drivers/i2c/I2CCC26XX.h>

#include <driverlib/prcm.h>

#include "Board.h"
#include "bsp_i2c.h"


//*****************************************************************************
// I2C setup

extern I2CCC26XX_HWAttrs i2cCC26xxHWAttrs[];
  
static uint8_t slaveAddr;
static uint8_t interface;
static I2C_Handle I2Chandle;
static I2C_Params I2CParams;
static Semaphore_Struct mutex;
static I2C_Transaction masterTransaction;
#ifdef POWER_SAVING
static bool checkI2cConstraint;
#endif

//
//  Burst write to an I2C device
//
bool bspI2cWrite(uint8_t *data, uint8_t len)
{
  masterTransaction.writeCount   = len;
  masterTransaction.writeBuf     = data;
  masterTransaction.readCount    = 0;
  masterTransaction.readBuf      = NULL;
  masterTransaction.slaveAddress = slaveAddr;    
  
  return I2C_transfer(I2Chandle, &masterTransaction) == TRUE;
}

//
//  Single write to an I2C device
//
bool bspI2cWriteSingle(uint8_t data)
{
  uint8_t d;
  
  // This is not optimal but is an implementation
  d = data;
  
  return bspI2cWrite(&d, 1);
}


//
//  Burst read from an I2C device
//
bool bspI2cRead(uint8_t *data, uint8_t len)
{
  masterTransaction.writeCount   = 0;
  masterTransaction.writeBuf     = NULL;
  masterTransaction.readCount    = len;
  masterTransaction.readBuf      = data;
  masterTransaction.slaveAddress = slaveAddr;    
  
  return I2C_transfer(I2Chandle, &masterTransaction) == TRUE;
}

//
//  Write and read in one operation
//
bool bspI2cWriteRead(uint8_t *wdata, uint8_t wlen, uint8_t *rdata, uint8_t rlen)
{
  masterTransaction.writeCount   = wlen;
  masterTransaction.writeBuf     = wdata;
  masterTransaction.readCount    = rlen;
  masterTransaction.readBuf      = rdata;
  masterTransaction.slaveAddress = slaveAddr;    
  
  return I2C_transfer(I2Chandle, &masterTransaction) == TRUE;
}


void bspI2cSelect(uint8_t newInterface, uint8_t address)
{
  /* Acquire I2C resource */
  Semaphore_pend(Semaphore_handle(&mutex),BIOS_WAIT_FOREVER);

#ifdef POWER_SAVING
  if (!checkI2cConstraint)
  {
    /* Prevent the system from entering standby while using I2C. */
    Power_setConstraint(Power_SB_DISALLOW);
    checkI2cConstraint = true;
  }
#endif
  
  slaveAddr = address;
  
  if (newInterface != interface) 
  {
    interface = newInterface;
    
    I2C_close(I2Chandle);
    
    if (interface == BSP_I2C_INTERFACE_0)
    {
      i2cCC26xxHWAttrs[CC2650_I2C0].sdaPin = Board_I2C0_SDA0;
      i2cCC26xxHWAttrs[CC2650_I2C0].sclPin = Board_I2C0_SCL0;
      
      // Secondary I2C as GPIO
      IOCPinTypeGpioInput(Board_I2C0_SDA1);
      IOCPinTypeGpioInput(Board_I2C0_SCL1);
      IOCIOPortPullSet(Board_I2C0_SDA1, IOC_NO_IOPULL);
      IOCIOPortPullSet(Board_I2C0_SCL1, IOC_NO_IOPULL);   
    }
    else if (interface == BSP_I2C_INTERFACE_1)
    {
      i2cCC26xxHWAttrs[CC2650_I2C0].sdaPin = Board_I2C0_SDA1;
      i2cCC26xxHWAttrs[CC2650_I2C0].sclPin = Board_I2C0_SCL1;
      
      // Primary I2C as GPIO
      IOCPinTypeGpioInput(Board_I2C0_SDA0);
      IOCPinTypeGpioInput(Board_I2C0_SCL0);
      IOCIOPortPullSet(Board_I2C0_SDA0, IOC_NO_IOPULL);
      IOCIOPortPullSet(Board_I2C0_SCL0, IOC_NO_IOPULL);
    }
    I2Chandle = I2C_open(Board_I2C, &I2CParams);
  }
}

void bspI2cDeselect(void)
{
#ifdef POWER_SAVING
  /* Allow the system to enter standby */
  if (checkI2cConstraint)
  {
    Power_releaseConstraint(Power_SB_DISALLOW);
    checkI2cConstraint = false;
  }
#endif
  /* Release I2C resource */
  Semaphore_post(Semaphore_handle(&mutex));  
}


void bspI2cInit(void)
{
  Semaphore_Params semParamsMutex;
  
  // Create protection semaphore
  Semaphore_Params_init(&semParamsMutex);
  semParamsMutex.mode = Semaphore_Mode_BINARY;
  Semaphore_construct(&mutex, 1, &semParamsMutex);
  
  // Reset the I2C controller
  HWREG(PRCM_BASE + PRCM_O_RESETI2C) = PRCM_RESETI2C_I2C;
    
  I2C_init();
  I2C_Params_init(&I2CParams);
  I2CParams.bitRate = I2C_400kHz;
  I2Chandle = I2C_open(Board_I2C, &I2CParams);
  
  // Initialize local variables
  slaveAddr = 0xFF;
  interface = BSP_I2C_INTERFACE_0;
#ifdef POWER_SAVING
  checkI2cConstraint = false;
#endif

  /*
  if (I2Chandle == NULL) {
    while(1) {
      // wait here for ever
    }
  }
  */
}

void bspI2cReset(void)
{
  I2C_close(I2Chandle);
  I2Chandle = I2C_open(Board_I2C, &I2CParams);
}

#endif
