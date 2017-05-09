/*
 * Copyright (c) 2014 - 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** ============================================================================
 *  @file       SENSORTAG_CC2650.h
 *
 *  @brief      SENSORTAG_CC2650 Board Specific APIs (CC2650)
 *
 *  The SENSORTAG_CC2650 header file should be included in an application as follows:
 *  @code
 *  #include <SENSORTAG_CC2650.h>
 *  @endcode
 *
 *  ============================================================================
 */
#ifndef __SENSORTAG_CC2650_H
#define __SENSORTAG_CC2650_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>
#include <driverlib/ioc.h>

#if defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h> 
    #define PIN2IOID(pin) (31 - __CLZ(pin))
#endif

#if defined(__TI_COMPILER_VERSION__)
    #define PIN2IOID(pin) (31 - __clz(pin))
#endif

/* LEDs on SENSORTAG_CC2650 are active high. */
#define SENSORTAG_CC2650_LED_OFF (0)
#define SENSORTAG_CC2650_LED_ON  (~0)

/* MPU_POWER on SENSORTAG_CC2650 is active high. */
#define SENSORTAG_CC2650_MPU_POWER_OFF (0)
#define SENSORTAG_CC2650_MPU_POWER_ON  (~0)

/* MIC_POWER on SENSORTAG_CC2650 is active high. */
#define SENSORTAG_CC2650_MIC_POWER_OFF (0)
#define SENSORTAG_CC2650_MIC_POWER_ON  (~0)

/* BUZZER on SENSORTAG_CC2650 is active high. */
#define SENSORTAG_CC2650_BUZZER_OFF (0)
#define SENSORTAG_CC2650_BUZZER_ON  (~0)

/* FLASH_CS  on SENSORTAG_CC2650 is active low. */
#define SENSORTAG_CC2650_FLASH_CS_OFF (~0)
#define SENSORTAG_CC2650_FLASH_CS_ON  (0)

  /* IO pin definitions */
#define BSP_IOID_BUTTON2          IOID_0
#define BSP_IOID_TMP_RDY          IOID_1
#define BSP_IOID_AUDIO_DI         IOID_2
#define BSP_IOID_REED_INT         IOID_3
#define BSP_IOID_BUTTON1          IOID_4
#define BSP_IOID_SDA              IOID_5
#define BSP_IOID_SCL              IOID_6
#define BSP_IOID_MPU_INT          IOID_7
#define BSP_IOID_SDA_HP           IOID_8
#define BSP_IOID_SCL_HP           IOID_9
#define BSP_IOID_LED1             IOID_10
#define BSP_IOID_IO7_SCLK         IOID_11 // Skin + Flash
#define BSP_IOID_MPU_POWER        IOID_12  
#define BSP_IOID_MIC_POWER        IOID_13
#define BSP_IOID_FLASH_CS         IOID_14 // Flash
#define BSP_IOID_LED2             IOID_15
#define BSP_IOID_IO12_TDO         IOID_16 // Skin
#define BSP_IOID_IO8_TDI          IOID_17 // Skin
#define BSP_IOID_IO9_UARTRX_MISO  IOID_18 // Skin + Flash
#define BSP_IOID_IO10_UARTTX_MOSI IOID_19 // Skin + Flash
#define BSP_IOID_IO11_CSN         IOID_20 // Skin + Flash
#define BSP_IOID_BUZZER_EN        IOID_21
#define BSP_IOID_IO6_PWM          IOID_22 // Skin
#define BSP_IOID_IO2              IOID_23 // Skin
#define BSP_IOID_IO1              IOID_24 // Skin
#define BSP_IOID_IO0              IOID_25 // Skin
#define BSP_IOID_VDD              IOID_26
#define BSP_IOID_IO3              IOID_27 // Skin
#define BSP_IOID_IO4              IOID_28 // Skin
#define BSP_IOID_IO5              IOID_29 // Skin
#define BSP_IOID_SKIN_ID          IOID_30 // Skin

/* SPI0 pin default pin mapping */
#define SENSORTAG_CC2650_SPI0_CSN  BSP_IOID_FLASH_CS
#define SENSORTAG_CC2650_SPI0_CLK  BSP_IOID_IO7_SCLK
#define SENSORTAG_CC2650_SPI0_MOSI BSP_IOID_IO10_UARTTX_MOSI
#define SENSORTAG_CC2650_SPI0_MISO BSP_IOID_IO9_UARTRX_MISO
  
/* I2C pin default pin mapping */
#define SENSORTAG_CC2650_I2C0_SCL  BSP_IOID_SCL
#define SENSORTAG_CC2650_I2C0_SDA  BSP_IOID_SDA

#define SENSORTAG_CC2650_I2C1_SCL  BSP_IOID_SCL_HP
#define SENSORTAG_CC2650_I2C1_SDA  BSP_IOID_SDA_HP

  /* GPIO_Callbacks structure for GPIO interrupts */
extern const GPIO_Callbacks SENSORTAG_CC2650_gpioPortCallbacks;


/*!
 *  @def    SENSORTAG_CC2650_GPIOName
 *  @brief  Enum of GPIO names on the SENSORTAG_CC2650 dev board
 */
typedef enum SENSORTAG_CC2650_GPIOName {
    SENSORTAG_CC2650_LED1 = 0,
    SENSORTAG_CC2650_LED2,
    SENSORTAG_CC2650_BUTTON2,
    SENSORTAG_CC2650_BUTTON1,
    SENSORTAG_CC2650_REED_INT,
    SENSORTAG_CC2650_BUZZER_EN,
    SENSORTAG_CC2650_UART_RX,
    SENSORTAG_CC2650_UART_TX,
    SENSORTAG_CC2650_MPU_POWER,
    SENSORTAG_CC2650_FLASH_CS,
    SENSORTAG_CC2650_MPU_INT,
    SENSORTAG_CC2650_TMP_RDY,
    SENSORTAG_CC2650_MIC_POWER,
    SENSORTAG_CC2650_GPIOCOUNT
} SENSORTAG_CC2650_GPIOName;

/*!
 *  @def    SENSORTAG_CC2650_I2CName
 *  @brief  Enum of I2C names on the SENSORTAG_CC2650 dev board
 */
typedef enum SENSORTAG_CC2650_I2CName {
    SENSORTAG_CC2650_I2C0 = 0,
    SENSORTAG_CC2650_I2CCOUNT
} SENSORTAG_CC2650_I2CName;

/*!
 *  @def    SENSORTAG_CC2650_SPIName
 *  @brief  Enum of SPI names on the SENSORTAG_CC2650 dev board
 */
typedef enum SENSORTAG_CC2650_SPIName {
    SENSORTAG_CC2650_SPI0 = 0,
    SENSORTAG_CC2650_SPICOUNT
} SENSORTAG_CC2650_SPIName;

/*!
 *  @def    SENSORTAG_CC2650_UARTName
 *  @brief  Enum of UARTs on the SENSORTAG_CC2650 dev board
 */
typedef enum SENSORTAG_CC2650_UARTName {
    SENSORTAG_CC2650_UART0 = 0,
    SENSORTAG_CC2650_UARTCOUNT
} SENSORTAG_CC2650_UARTName;

/*!
 *  @def    SENSORTAG_CC2650_WatchdogName
 *  @brief  Enum of Watchdogs on the SENSORTAG_CC2650 dev board
 */
typedef enum SENSORTAG_CC2650_WatchdogName {
    SENSORTAG_CC2650_WATCHDOG0 = 0,
    SENSORTAG_CC2650_WATCHDOGCOUNT
} SENSORTAG_CC2650_WatchdogName;

/*!
 *  @brief  Initialize board specific DMA settings
 *
 *  This function creates a hwi in case the DMA controller creates an error
 *  interrupt, enables the DMA and supplies it with a uDMA control table.
 */
extern Void SENSORTAG_CC2650_initDMA(Void);

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings. This include
 *     - Flash wait states based on the process
 *     - Disable clock source to watchdog module
 *     - Enable clock sources for peripherals
 */
extern Void SENSORTAG_CC2650_initGeneral(Void);

/*!
 *  @brief  Initialize board specific GPIO settings
 *
 *  This function initializes the board specific GPIO settings and
 *  then calls the GPIO_init API to initialize the GPIO module.
 *
 *  The GPIOs controlled by the GPIO module are determined by the GPIO_config
 *  variable.
 */
extern Void SENSORTAG_CC2650_initGPIO(Void);

/*!
 *  @brief  Initialize board specific I2C settings
 *
 *  This function initializes the board specific I2C settings and then calls
 *  the I2C_init API to initialize the I2C module.
 *
 *  The I2C peripherals controlled by the I2C module are determined by the
 *  I2C_config variable.
 */
extern Void SENSORTAG_CC2650_initI2C(Void);

/*!
 *  @brief  Assigns pins to I2C controller
 */
extern Void I2CCC26XX_ioInit(const I2C_Config* I2C_config, int i2cInterfaceID);

/*!
 *  @brief  Initialize board specific SPI settings
 *
 *  This function initializes the board specific SPI settings and then calls
 *  the SPI_init API to initialize the SPI module.
 *
 *  The SPI peripherals controlled by the SPI module are determined by the
 *  SPI_config variable.
 */
extern Void SENSORTAG_CC2650_initSPI(Void);

/*!
 *  @brief  Initialize board specific UART settings
 *
 *  This function initializes the board specific UART settings and then calls
 *  the UART_init API to initialize the UART module.
 *
 *  The UART peripherals controlled by the UART module are determined by the
 *  UART_config variable.
 */
extern Void SENSORTAG_CC2650_initUART(Void);

/*!
 *  @brief  Initialize board specific Watchdog settings
 *
 *  This function initializes the board specific Watchdog settings and then
 *  calls the Watchdog_init API to initialize the Watchdog module.
 *
 *  The Watchdog peripherals controlled by the Watchdog module are determined
 *  by the Watchdog_config variable.
 */
extern Void SENSORTAG_CC2650_initWatchdog(Void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSORTAG_CC2650_H */
