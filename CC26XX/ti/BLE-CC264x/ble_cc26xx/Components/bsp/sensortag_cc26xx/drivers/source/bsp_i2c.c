/******************************************************************************
*  Filename:       bsp_i2c.c
*  Revised:        $Date: 2015-01-15 04:37:10 -0800 (Thu, 15 Jan 2015) $
*  Revision:       $Revision: 14822 $
*
*  Description:    Common API for I2C access. Driverlib implementation.
*
*  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
*
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

#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ioc.h"
#include "inc/hw_prcm.h"

#include "driverlib/interrupt.h"
#include "driverlib/ioc.h"
#include "driverlib/prcm.h"
#include "driverlib/sys_ctrl.h"
#include "driverlib/i2c.h"

#include "bsp.h"
#include "bsp_i2c.h"


static bool bspI2cStatus(void);

static uint8_t slaveAddr = 0x00;
static uint8_t interface = 0xFF;


/**************************************************************************
* @brief    This function initializes the I2C interface
*
* @return   None
******************************************************************************/
void bspI2cInit(void)
{
  // The I2C peripheral must be enabled
  PRCMPeripheralRunEnable(PRCM_PERIPH_I2C0);
  PRCMLoadSet();
  while(!PRCMLoadGet());
  
  // Reset the I2C controller
  HWREG(PRCM_BASE + PRCM_O_RESETI2C) = PRCM_RESETI2C_I2C;
  
  // Enable and initialize the I2C master module
  I2CMasterInitExpClk(I2C0_BASE, SysCtrlPeripheralClockGet(PRCM_PERIPH_I2C0, SYSCTRL_SYSBUS_ON), true);
}


//
//  Burst write to an I2C device
//
bool bspI2cWrite(uint8_t *data, uint8_t len)
{
  uint32_t i;
  bool fSuccess;
  
  // Write slave address
  I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddr, false);
  
  // Write first byte
  I2CMasterDataPut(I2C0_BASE, data[0]);
  
  // Check if another master has access
  while(I2CMasterBusBusy(I2C0_BASE));
  
  // Assert RUN + START
  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(I2CMasterBusy(I2C0_BASE));
  fSuccess = bspI2cStatus();
  
  for (i = 1; i<len && fSuccess; i++)
  {
    // Write next byte
    I2CMasterDataPut(I2C0_BASE, data[i]);
    if (i<len-1)
    {
      // Clear START
      I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
      while(I2CMasterBusy(I2C0_BASE));
      fSuccess = bspI2cStatus();
    }
  }
  
  // Assert stop
  if (fSuccess)
  {
    // Assert STOP
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C0_BASE));
    fSuccess = bspI2cStatus();
    while(I2CMasterBusBusy(I2C0_BASE));
  }
  
  return fSuccess;
}

//
//  Single write to on I2C device
//
bool bspI2cWriteSingle(uint8_t data)
{
  bool fSuccess;
  
  // Write slave address
  I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddr, false);
  
  // Write first byte
  I2CMasterDataPut(I2C0_BASE, data);
  
  // Check if another master has access
  while(I2CMasterBusBusy(I2C0_BASE));
  
  // Assert RUN + START + STOP
  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(I2CMasterBusy(I2C0_BASE));
  fSuccess = bspI2cStatus();

  return fSuccess;
}


//
//  Burst read from an I2C device
//
bool bspI2cRead(uint8_t *data, uint8_t len)
{
  uint8_t i;
  bool success;
  
  // Set slave address
  I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddr, true);

  // Check if another master has access
  while(I2CMasterBusBusy(I2C0_BASE));

  // Assert RUN + START + ACK
  I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_START);
  
  i = 0;
  success = true;
  while ( i < (len - 1) && success)
  {
    while(I2CMasterBusy(I2C0_BASE));
    success = bspI2cStatus();
    if (success)
    {
      data[i] = I2CMasterDataGet(I2C0_BASE);
      I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      i++;
    }
  }
  
  if (success)
  {
    while(I2CMasterBusy(I2C0_BASE));
    success = bspI2cStatus();
    if (success)
    {
      data[len-1] = I2CMasterDataGet(I2C0_BASE);
      I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
      while(I2CMasterBusBusy(I2C0_BASE));
    }
  }
  
  return success;
}

//
//  Write and read in once operation
//
bool bspI2cWriteRead(uint8_t *wdata, uint8_t wlen, uint8_t *rdata, uint8_t rlen)
{
  uint32_t i;
  bool fSuccess;
  
  // Set slave address for write
  I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddr, false);
  
  // Write first byte
  I2CMasterDataPut(I2C0_BASE, wdata[0]);
  
  // Check if another master has access
  while(I2CMasterBusBusy(I2C0_BASE));
  
  // Assert RUN + START
  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(I2CMasterBusy(I2C0_BASE));
  fSuccess = bspI2cStatus();
  
  for (i = 1; i<wlen && fSuccess; i++)
  {
    // Write next byte
    I2CMasterDataPut(I2C0_BASE, wdata[i]);
    if (i<wlen-1)
    {
      // Clear START
      I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
      while(I2CMasterBusy(I2C0_BASE));
      fSuccess = bspI2cStatus();
    }
  }
  if (!fSuccess)
    return false;

  // Set slave address for read
  I2CMasterSlaveAddrSet(I2C0_BASE, slaveAddr, true);

  // Assert ACK
  I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_START);
  
  i = 0;
  while ( i < (rlen - 1) && fSuccess)
  {
    while(I2CMasterBusy(I2C0_BASE));
    fSuccess = bspI2cStatus();
    if (fSuccess)
    {
      rdata[i] = I2CMasterDataGet(I2C0_BASE);
      I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      i++;
    }
  }
  
  if (fSuccess)
  {
    while(I2CMasterBusy(I2C0_BASE));
    fSuccess = bspI2cStatus();
    if (fSuccess)
    {
      rdata[rlen-1] = I2CMasterDataGet(I2C0_BASE);
      I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
      while(I2CMasterBusBusy(I2C0_BASE));
    }
  }

  return fSuccess;
}


void bspI2cSelect(uint8_t newInterface, uint8_t address)
{
  slaveAddr = address;
  
  if (newInterface != interface) 
  {
    interface = newInterface;
    if (interface == BSP_I2C_INTERFACE_0)
    {
      IOCIOPortPullSet(BSP_IOD_SDA, IOC_NO_IOPULL);
      IOCIOPortPullSet(BSP_IOD_SCL, IOC_NO_IOPULL);
      IOCPinTypeI2c(I2C0_BASE, BSP_IOD_SDA, BSP_IOD_SCL);
      IOCPinTypeGpioInput(BSP_IOD_SDA_HP);
      IOCPinTypeGpioInput(BSP_IOD_SCL_HP);
    }
    else if (interface == BSP_I2C_INTERFACE_1)
    {
      IOCIOPortPullSet(BSP_IOD_SDA_HP, IOC_NO_IOPULL);
      IOCIOPortPullSet(BSP_IOD_SCL_HP, IOC_NO_IOPULL);
      IOCPinTypeI2c(I2C0_BASE, BSP_IOD_SDA_HP, BSP_IOD_SCL_HP);
      IOCPinTypeGpioInput(BSP_IOD_SDA);
      IOCPinTypeGpioInput(BSP_IOD_SCL);
    }
  }
}

void bspI2cDeselect(void)
{
}


static bool bspI2cStatus(void)
{
  uint32_t status;
  
  status = I2CMasterErr(I2C0_BASE);
  if (status & (I2C_MSTAT_DATACK_N_M | I2C_MSTAT_ADRACK_N_M))
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
  
  return status == I2C_MASTER_ERR_NONE;
}

