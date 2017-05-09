/**
  @file  bsp_spi.h
  @brief SPI-bus abstraction

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
#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Initialize SPI interface
   *
   * @return none
   */
  extern void bspSpiOpen(void);

  /**
   * Close SPI interface
   *
   * @return True when successful.
   */
  extern void bspSpiClose(void);

  /**
   * Clear data from SPI interface
   *
   * @return none
   */
  extern void bspSpiFlush(void);

  /**
   * Read from an SPI device
   *
   * @return 0 when successful.
   */
  extern int bspSpiRead(uint8_t *buf, size_t length);

  /**
   * Write to an SPI device
   *
   * @return 0 when successful.
   */
  extern int bspSpiWrite(const uint8_t *buf, size_t length);

  /**
   * Write and read to/from an SPI device in the same transaction
   *
   * @return 0 when successful.
   */
  extern  int bspSpiWriteRead(uint8_t *buf, uint8_t wlen, uint8_t rlen);
  
  /**
   * Select the external flash device (chip select)
   *
   * @return 0 when successful.
   */
  void bspSpiFlashSelect(bool select);
  
#ifdef __cplusplus
}
#endif

#endif /* BSP_SPI_H */
