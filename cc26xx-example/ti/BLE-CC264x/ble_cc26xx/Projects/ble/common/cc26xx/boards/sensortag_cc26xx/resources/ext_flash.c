/**
  @file  ext_flash.c
  @brief Experimental base loader storage implementation for W25X20CL

  <!--
  Copyright 2014 - 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED ``AS IS'' WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
  -->
*/
#ifndef TI_DRIVERS_SPI_INCLUDED
#include <driverlib/gpio.h>
#endif
#include "bsp.h"
#include "bsp_spi.h"
#include "ext_flash.h"

/* 
 * Implementation for WinBond W25X20CL Flash 
 *
 */

/* Instruction codes */

#define BLS_CODE_PROGRAM          0x02 /**< Page Program */
#define BLS_CODE_READ             0x03 /**< Read Data */
#define BLS_CODE_READ_STATUS      0x05 /**< Read Status Register */
#define BLS_CODE_WRITE_ENABLE     0x06 /**< Write Enable */
#define BLS_CODE_SECTOR_ERASE     0x20 /**< Sector Erase */
#define BLS_CODE_MDID             0x90 /**< Manufacturer Device ID */

#define BLS_CODE_DP               0xB9 /**< Power down */
#define BLS_CODE_RDP              0xAB /**< Power standby */

/* Erase instructions */

#define BLS_CODE_ERASE_4K         0x20 /**< Sector Erase */
#define BLS_CODE_ERASE_32K        0x52
#define BLS_CODE_ERASE_64K        0xD8
#define BLS_CODE_ERASE_ALL        0xC7 /**< Mass Erase */

/* Bitmasks of the status register */

#define BLS_STATUS_SRWD_BM        0x80
#define BLS_STATUS_BP_BM          0x0C
#define BLS_STATUS_WEL_BM         0x02
#define BLS_STATUS_WIP_BM         0x01

#define BLS_STATUS_BIT_BUSY       0x01 /**< Busy bit of the status register */

/* Part specific constants */

#define BLS_MANUFACTURER_ID       0xEF
#define BLS_DEVICE_ID             0x11

#define BLS_PROGRAM_PAGE_SIZE     256
#define BLS_ERASE_SECTOR_SIZE     4096

static int extFlashWaitReady(void);
static int extFlashWaitPowerDown(void);

/**
 * Clear external flash CSN line
 */
static void extFlashSelect(void)
{
#ifndef TI_DRIVERS_SPI_INCLUDED
  GPIOPinWrite(BSP_FLASH_CS, 0);
#else
  bspSpiFlashSelect(true);
#endif
}

/**
 * Set external flash CSN line
 */
static void extFlashDeselect(void)
{
#ifndef TI_DRIVERS_SPI_INCLUDED
  GPIOPinWrite(BSP_FLASH_CS, 1);
#else
  bspSpiFlashSelect(false);
#endif
}

/******************************************************************************
* @fn       extFlashPowerDown
*
* @brief    Put the device in power save mode. No access to data; only
*           the status register is accessible.
*
* @param    none
*
* @return   Returns true SPI transactions succeed
******************************************************************************/
static bool bspFlashExtPowerDown(void)
{
  uint8_t cmd;
  bool success;
  
  cmd = BLS_CODE_DP;
  extFlashSelect();
  success = bspSpiWrite(&cmd,sizeof(cmd)) == 0;
  extFlashDeselect();
  
  return success;
}

/******************************************************************************
* @fn       extFlashPowerStandby
*
* @brief    Take device out of power save mode and prepare it for normal operation
*
* @param    none
*
* @return   Returns true if command successfully written
******************************************************************************/
static bool extFlashPowerStandby(void)
{
  uint8_t cmd;
  bool success;
  
  cmd = BLS_CODE_RDP;
  extFlashSelect();
  success = bspSpiWrite(&cmd,sizeof(cmd)) == 0;
  extFlashDeselect();
  
  if (success)
    success = extFlashWaitReady() == 0;
  
  return success;
}

/**
 * Verify the flash part.
 * @return True when successful.
 */
static bool extFlashVerifyPart(void)
{
  const uint8_t wbuf[] = { BLS_CODE_MDID, 0xFF, 0xFF, 0x00 };
  uint8_t rbuf[2];

  extFlashSelect();

  int ret = bspSpiWrite(wbuf, sizeof(wbuf));
  if (ret)
  {
    extFlashDeselect();
    return false;
  }

  ret = bspSpiRead(rbuf, sizeof(rbuf));
  extFlashDeselect();

  if (ret || rbuf[0] != BLS_MANUFACTURER_ID || rbuf[1] != BLS_DEVICE_ID)
  {
    return false;
  }
  return true;
}

/**
 * Wait till previous erase/program operation completes.
 * @return Zero when successful.
 */
static int extFlashWaitReady(void)
{
  const uint8_t wbuf[1] = { BLS_CODE_READ_STATUS };
  int ret;

  /* Throw away all garbages */
  extFlashSelect();
  bspSpiFlush();
  extFlashDeselect();

  for (;;)
  {
    uint8_t buf;
    /* Note that this temporary implementation is not
     * energy efficient.
     * Thread could have yielded while waiting for flash
     * erase/program to complete.
     */
    extFlashSelect();

    bspSpiWrite(wbuf, sizeof(wbuf));
    ret = bspSpiRead(&buf,sizeof(buf));
    
    extFlashDeselect();
    
    if (ret)
    {
      /* Error */
      return -2;
    }
    if (!(buf & BLS_STATUS_BIT_BUSY))
    {
      /* Now ready */
      break;
    }
  }
  
  return 0;
}

/**
 * Wait until the part has entered power down (status register is 0xFF)
 * @return Zero when successful.
 */
static int extFlashWaitPowerDown(void)
{
  const uint8_t wbuf[1] = { BLS_CODE_READ_STATUS };
  int ret;

  /* Throw away all garbages */
  bspSpiFlush();

  for (;;)
  {
    uint8_t buf;
    /* Note that this temporary implementation is not
     * energy efficient.
     * Thread could have yielded while waiting for flash
     * erase/program to complete.
     */
    extFlashSelect();

    bspSpiWrite(wbuf, sizeof(wbuf));
    ret = bspSpiRead(&buf,sizeof(buf));
    
    extFlashDeselect();
    
    if (ret)
    {
      /* Error */
      return -2;
    }
    if (buf == 0xFF)
    {
      /* Now ready */
      break;
    }
  }
  
  return 0;
}

/**
 * Enable write.
 * @return Zero when successful.
 */
static int extFlashWriteEnable(void)
{
  const uint8_t wbuf[] = { BLS_CODE_WRITE_ENABLE };

  extFlashSelect();
  int ret = bspSpiWrite(wbuf,sizeof(wbuf));
  extFlashDeselect();

  if (ret)
  {
    return -3;
  }
  return 0;
}

/* See ext_flash.h file for description */
bool extFlashOpen(void)
{
#ifndef TI_DRIVERS_SPI_INCLUDED
  bspSpiOpen();
  /* GPIO pin configuration */
  IOCPinTypeGpioOutput(BSP_IOID_FLASH_CS);
#endif
  /* Default output to clear chip select */
  extFlashDeselect();

  /* Put the part is standby mode */
  extFlashPowerStandby();
  
  return extFlashVerifyPart();
}

/* See ext_flash.h file for description */
void extFlashClose(void)
{
  // Put the part in low power mode
  bspFlashExtPowerDown();
#ifndef TI_DRIVERS_SPI_INCLUDED
  bspSpiClose();
#endif
}

/* See ext_flash.h file for description */
bool extFlashRead(size_t offset, size_t length, uint8_t *buf)
{
  uint8_t wbuf[4];

  /* Wait till previous erase/program operation completes */
  int ret = extFlashWaitReady();
  if (ret)
  {
    return false;
  }

  /* SPI is driven with very low frequency (1MHz < 33MHz fR spec)
   * in this temporary implementation.
   * and hence it is not necessary to use fast read. */
  wbuf[0] = BLS_CODE_READ;
  wbuf[1] = (offset >> 16) & 0xff;
  wbuf[2] = (offset >> 8) & 0xff;
  wbuf[3] = offset & 0xff;

  extFlashSelect();

  if (bspSpiWrite(wbuf, sizeof(wbuf)))
  {
    /* failure */
    extFlashDeselect();
    return false;
  }

  ret = bspSpiRead(buf, length);

  extFlashDeselect();

  return ret == 0;
}

/* See ext_flash.h file for description */
bool extFlashWrite(size_t offset, size_t length, const uint8_t *buf)
{
  uint8_t wbuf[4];

  while (length > 0)
  {
    /* Wait till previous erase/program operation completes */
    int ret = extFlashWaitReady();
    if (ret)
    {
      return false;
    }

    ret = extFlashWriteEnable();
    if (ret)
    {
      return false;
    }

    size_t ilen; /* interim length per instruction */

    ilen = BLS_PROGRAM_PAGE_SIZE - (offset % BLS_PROGRAM_PAGE_SIZE);
    if (length < ilen)
    {
      ilen = length;
    }

    wbuf[0] = BLS_CODE_PROGRAM;
    wbuf[1] = (offset >> 16) & 0xff;
    wbuf[2] = (offset >> 8) & 0xff;
    wbuf[3] = offset & 0xff;

    offset += ilen;
    length -= ilen;

    /* Up to 100ns CS hold time (which is not clear
     * whether it's application only in-between reads)
     * is not imposed here since above instructions
     * should be enough to delay
     * as much. */
    extFlashSelect();

    if (bspSpiWrite(wbuf, sizeof(wbuf)))
    {
      /* failure */
      extFlashDeselect();
      return false;
    }

    if (bspSpiWrite(buf,ilen))
    {
      /* failure */
      extFlashDeselect();
      return false;
    }
    buf += ilen;
    extFlashDeselect();
  }

  return true;
}

/* See ext_flash.h file for description */
bool extFlashErase(size_t offset, size_t length)
{
  /* Note that Block erase might be more efficient when the floor map
   * is well planned for OTA but to simplify for the temporary implementation,
   * sector erase is used blindly. */
  uint8_t wbuf[4];
  size_t i, numsectors;

  wbuf[0] = BLS_CODE_SECTOR_ERASE;

  {
    size_t endoffset = offset + length - 1;
    offset = (offset / BLS_ERASE_SECTOR_SIZE) * BLS_ERASE_SECTOR_SIZE;
    numsectors = (endoffset - offset + BLS_ERASE_SECTOR_SIZE - 1) / BLS_ERASE_SECTOR_SIZE;
  }

  for (i = 0; i < numsectors; i++)
  {
    /* Wait till previous erase/program operation completes */
    int ret = extFlashWaitReady();
    if (ret)
    {
      return false;
    }

    ret = extFlashWriteEnable();
    if (ret)
    {
      return false;
    }

    wbuf[1] = (offset >> 16) & 0xff;
    wbuf[2] = (offset >> 8) & 0xff;
    wbuf[3] = offset & 0xff;

    extFlashSelect();

    if (bspSpiWrite(wbuf, sizeof(wbuf)))
    {
      /* failure */
      extFlashDeselect();
      return false;
    }
    extFlashDeselect();

    offset += BLS_ERASE_SECTOR_SIZE;
  }

  return true;
}

/* See ext_flash.h file for description */
bool extFlashTest(void)
{
  bool ret;
  
  ret = extFlashOpen();
  if (ret)
  {
    extFlashClose();
    
    ret = extFlashWaitPowerDown() == 0;
  }
  
  return ret;
}