/*
 * Copyright (c) 2014, Texas Instruments Incorporated
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

/*
 *  ======== SENSORTAG_CC26xx.c ========
 *  This file is responsible for setting up the board specific items for the
 *  SRF06EB_CC2650 with (CC2650EM) board.
 *
 *  The following defines are used to determine which TI-RTOS peripheral drivers
 *  to include:
 *     TI_DRIVERS_GPIO_INCLUDED
 *     TI_DRIVERS_I2C_INCLUDED
 *     TI_DRIVERS_SPI_INCLUDED
 *     TI_DRIVERS_SPI_INCLUDED
 *     TI_DRIVERS_UART_INCLUDED
 *     TI_DRIVERS_UART_DMA_INCLUDED
 *     TI_DRIVERS_WATCHDOG_INCLUDED
 *     TI_DRIVERS_LCD_INCLUDED
 *  These defines are created when a useModule is done on the driver in the
 *  application's .cfg file. The actual #define is in the application
 *  generated header file that is brought in via the xdc/cfg/global.h.
 *  For example the following in the .cfg file
 *     var GPIO = xdc.useModule('ti.drivers.GPIO');
 *  Generates the following
 *     #define TI_DRIVERS_GPIO_INCLUDED 1
 *  If there is no useModule of ti.drivers.GPIO, the constant is set to 0.
 *
 *  Note: a useModule is generated in the .cfg file via the graphical
 *  configuration tool when the "Add xxx to my configuration" is checked
 *  or "Use xxx" is selected.
 */

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>

#include <driverlib/udma.h>
#include <driverlib/osc.h>
#include <driverlib/cpu.h>
#include <driverlib/pwr_ctrl.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/aon_event.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>

#include <ti/sysbios/family/arm/cc26xx/Power.h>
#include <ti/sysbios/family/arm/cc26xx/PowerCC2650.h>

#if !(defined(MODULE_CC26XX_7X7) || defined(MODULE_CC26XX_5X5) || defined(MODULE_CC26XX_4X4))
	#error "Please specify project wide definition of package: MODULE_CC26XX_7X7 / MODULE_CC26XX_5X5 / MODULE_CC26XX_4X4..."
#endif

/* Make sure to include drivers for GPIO and SPI if LCD driver is included */
#if defined(TI_DRIVERS_LCD_INCLUDED)
#ifndef TI_DRIVERS_GPIO_INCLUDED
#define TI_DRIVERS_GPIO_INCLUDED
#endif
#ifndef TI_DRIVERS_SPI_INCLUDED
#define TI_DRIVERS_SPI_INCLUDED
#endif
#endif // TI_DRIVERS_LCD_INCLUDED

/* workaround to avoid macro expansion of ccs to ‘1’ within a nested #include. Bug number: 320943 */
#ifdef ccs
#undef ccs
#include <xdc/cfg/global.h>
#define ccs 1
#else
#include <xdc/cfg/global.h>
#endif

#include "SENSORTAG_CC2650.h"

#if (TI_DRIVERS_UART_DMA_INCLUDED || TI_DRIVERS_SPI_INCLUDED)
#include <ti/drivers/UDMA.h>

/* Udma objects */
Udma_Object UdmaObjects[SENSORTAG_CC2650_UDMACOUNT];

/* UDMA configuration structure */
const Udma_HWAttrs UdmaHWAttrs[SENSORTAG_CC2650_UDMACOUNT] = {
    {UDMA0_BASE, INT_UDMAERR, PERIPH_UDMA},
};

const Udma_Config Udma_config[] = {
    {&UdmaObjects[0], &UdmaHWAttrs[0]},
    {NULL, NULL},
};

#endif /* (TI_DRIVERS_UART_DMA_INCLUDED || TI_DRIVERS_SPI_INCLUDED) */

/*
 *  ======== SENSORTAG_CC2650_initGeneral ========
 */
Void SENSORTAG_CC2650_initGeneral(Void)
{
  /* force power on AUX - this will be released when entering sleep mode */
  AONWUCAuxWakeupEvent(AONWUC_AUX_WAKEUP);
  
  /* enable the AUX oscillator clock */
  AUXWUCClockEnable(AUX_WUC_OSCCTRL_CLOCK);
  while(AUXWUCClockStatus(AUX_WUC_OSCCTRL_CLOCK) != AUX_WUC_CLOCK_READY)
  { }
  
  /* This application will not be using the AUX domain out of boot
  * and we will leave out clock for optimal power conservation */
  AONWUCAuxPowerDownConfig(AONWUC_NO_CLOCK);
  
  /*
  * Source the LF clock from the low frequency XTAL_OSC.
  * HF and MF are sourced from the high frequency RC_OSC.
  */
  OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_XOSC_LF);
  OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_RCOSC_HF);
  
  /*
  * Check if already sourcing the HF clock from RC_OSC.
  * If a switch of the clock source is not required, then the call to ROM
  * will loop forever.
  */
  if(OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_RCOSC_HF)
  {
    OSCHfSourceSwitch();
  }
  
  /* enable DCDC */
  PowerCtrlSourceSet(PWRCTRL_PWRSRC_DCDC);
  
  /* make sure AON accesses are in sync and enable powerdown on AUX */
  SysCtrlAonSync();
  AUXWUCPowerCtrl(AUX_WUC_POWER_DOWN);
}

#if TI_DRIVERS_GPIO_INCLUDED
#include <ti/drivers/GPIO.h>

/* Callback functions for the GPIO interrupt */
void gpioButton1Fxn(void);
void gpioButton2Fxn(void);
void gpioRelayFxn(void);
void gpioMpuFxn(void);

/* GPIO configuration structure */
const GPIO_HWAttrs gpioHWAttrs[SENSORTAG_CC2650_GPIOCOUNT] = {
    {GPIO_PIN_10, GPIO_OUTPUT}, /* SENSORTAG_CC2650_LED1 */
    {GPIO_PIN_15, GPIO_OUTPUT}, /* SENSORTAG_CC2650_LED2 */
    {GPIO_PIN_0,  GPIO_INPUT},  /* SENSORTAG_CC2650_BUTTON2 */
    {GPIO_PIN_4,  GPIO_INPUT},  /* SENSORTAG_CC2650_BUTTON1 */
    {GPIO_PIN_3,  GPIO_INPUT},  /* SENSORTAG_CC2650_RELAY */
    {GPIO_PIN_21, GPIO_OUTPUT}, /* SENSORTAG_CC2650_BUZZER_EN */
    {GPIO_PIN_17, GPIO_INPUT},  /* SENSORTAG_CC2650_UART_RX */
    {GPIO_PIN_16, GPIO_OUTPUT}, /* SENSORTAG_CC2650_UART_TX */
    {GPIO_PIN_12, GPIO_OUTPUT}, /* SENSORTAG_CC2650_MPU_POWER */
    {GPIO_PIN_14, GPIO_OUTPUT}, /* SENSORTAG_CC2650_FLASH_CS */
    {GPIO_PIN_7,  GPIO_INPUT},  /* SENSORTAG_CC2650_MPU_INT */
    {GPIO_PIN_1,  GPIO_INPUT},  /* SENSORTAG_CC2650_TMP_RDY */
    {GPIO_PIN_13, GPIO_OUTPUT}, /* SENSORTAG_CC2650_MIC_POWER */
};

/* Memory for the GPIO module to construct a Hwi */
Hwi_Struct callbackHwi;

/* GPIO callback structure to set callbacks for GPIO interrupts. NB! Indexed by GPIO pin number! */
const GPIO_Callbacks SENSORTAG_CC2650_gpioPortCallbacks = {
	INT_EDGE_DETECT, &callbackHwi,
	{
    gpioButton2Fxn, NULL, NULL, gpioRelayFxn, gpioButton1Fxn, NULL, NULL, gpioMpuFxn,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
  }
};

const GPIO_Config GPIO_config[SENSORTAG_CC2650_GPIOCOUNT+1] = {
    {&gpioHWAttrs[0]},
    {&gpioHWAttrs[1]},
    {&gpioHWAttrs[2]},
    {&gpioHWAttrs[3]},
    {&gpioHWAttrs[4]},
    {&gpioHWAttrs[5]},
    {&gpioHWAttrs[6]},
    {&gpioHWAttrs[7]},
    {&gpioHWAttrs[8]},
    {&gpioHWAttrs[9]},
    {&gpioHWAttrs[10]},
    {&gpioHWAttrs[11]},
    {&gpioHWAttrs[11]},
    {NULL},
};

/*
 *  ======== SENSORTAG_CC2650_initGPIO ========
 */
Void SENSORTAG_CC2650_initGPIO(Void)
{
	Bits32 pin;
  
	/* Power up the GPIO module. */
	GPIO_open();

  /* Setup the LED1 (red) GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_LED1].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_OUTPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_OUT);
  
  /* Setup the LED2 (green) GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_LED2].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_OUTPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_OUT);

  /* Setup the Button1 GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_BUTTON1].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_INPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_IN);
  IOCIOPortPullSet(PIN2IOID(pin), IOC_IOPULL_UP);
  
  /* Setup the Button2 GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_BUTTON2].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_INPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_IN);
  IOCIOPortPullSet(PIN2IOID(pin), IOC_IOPULL_UP);

  /* Setup the Reed relay GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_REED_INT].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_INPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_IN);
  IOCIOPortPullSet(PIN2IOID(pin), IOC_IOPULL_DOWN);

  /* Setup the FLASH_CS pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_FLASH_CS].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_OUTPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_OUT);
  IOCIOPortPullSet(PIN2IOID(pin), IOC_NO_IOPULL);

  /* Setup the BUZZER_EN pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_BUZZER_EN].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_OUTPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_OUT);

  /* Setup the MPU_POWER GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_MPU_POWER].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_OUTPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_OUT);

  /* Setup the MPU_INT GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_MPU_INT].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_INPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_IN);
  IOCIOPortPullSet(PIN2IOID(pin), IOC_IOPULL_DOWN);

  /* Setup the TMP_RDY GPIO pin. */
	pin = gpioHWAttrs[SENSORTAG_CC2650_TMP_RDY].pin;
  IOCPortConfigureSet(PIN2IOID(pin), IOC_PORT_GPIO, IOC_STD_INPUT);
  GPIODirModeSet(pin, GPIO_DIR_MODE_IN);

  /* Once GPIO_init is called, GPIO_config cannot be changed */
  GPIO_init();
  
  /* Set some of the IO pins */
  GPIO_write(SENSORTAG_CC2650_LED1, SENSORTAG_CC2650_LED_OFF);
  GPIO_write(SENSORTAG_CC2650_LED2, SENSORTAG_CC2650_LED_OFF);
  GPIO_write(SENSORTAG_CC2650_MPU_POWER, SENSORTAG_CC2650_MPU_POWER_ON);
  GPIO_write(SENSORTAG_CC2650_MIC_POWER, SENSORTAG_CC2650_MIC_POWER_OFF);
  GPIO_write(SENSORTAG_CC2650_FLASH_CS, SENSORTAG_CC2650_FLASH_CS_OFF);
}


#endif /* TI_DRIVERS_GPIO_INCLUDED */

#if TI_DRIVERS_I2C_INCLUDED

#include <ti/drivers/I2C.h>
#include "ti/drivers/i2c/I2CCC26XX.h"
#include <driverlib/prcm.h>
#include "bsp_i2c.h"

/* Init function of IOs for SPI */
void I2CCC26XX_ioInit(const I2C_Config* I2C_config, int i2cInterfaceID)
{
  I2CCC26XX_Object* pI2CCC26XX;
  
  /* Initialize the IOs for I2C0 */
  pI2CCC26XX = (I2CCC26XX_Object*)I2C_config[0].object;

  if (i2cInterfaceID == BSP_I2C_INTERFACE_0)
  {
    pI2CCC26XX->i2cScl = SENSORTAG_CC2650_I2C0_SCL;
    pI2CCC26XX->i2cSda = SENSORTAG_CC2650_I2C0_SDA;
  } 
  else if (i2cInterfaceID == BSP_I2C_INTERFACE_1)
  {
    pI2CCC26XX->i2cScl = SENSORTAG_CC2650_I2C1_SCL;
    pI2CCC26XX->i2cSda = SENSORTAG_CC2650_I2C1_SDA;
  }
}

/* I2C objects */
I2CCC26XX_Object i2cCC26XXObjects[SENSORTAG_CC2650_I2CCOUNT];

/* I2C configuration structure, describing which pins are to be used */
const I2CCC26XX_HWAttrs i2cCC26XXHWAttrs[SENSORTAG_CC2650_I2CCOUNT] = {
    {I2C0_BASE, INT_I2C, PERIPH_I2C0},
};

const I2C_Config I2C_config[] = {
    {&I2CCC26XX_fxnTable, &i2cCC26XXObjects[0], &i2cCC26XXHWAttrs[0]},
    {NULL, NULL, NULL}
};

/*
 *  ======== SENSORTAG_CC2650_initI2C ========
 */
Void SENSORTAG_CC2650_initI2C(Void)
{
  /* Initialize drivers */
  I2C_init();
  
  /* Initialize IOs */
  I2CCC26XX_ioInit(I2C_config, BSP_I2C_INTERFACE_0);
}
#endif /* TI_DRIVERS_I2C_INCLUDED */

#if TI_DRIVERS_SPI_INCLUDED
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26XX.h>

/* Init function of IOs for SPI */
static void SPICC26XX_ioInit(const SPI_Config* SPI_config)
{
  /* Initialize the IOs for the SPI0 */
  ((SPICC26XX_Object*)SPI_config[0].object)->spiMiso = SENSORTAG_CC2650_SPI0_MISO;
  ((SPICC26XX_Object*)SPI_config[0].object)->spiMosi = SENSORTAG_CC2650_SPI0_MOSI;
  ((SPICC26XX_Object*)SPI_config[0].object)->spiClk = SENSORTAG_CC2650_SPI0_CLK;
  ((SPICC26XX_Object*)SPI_config[0].object)->spiCsn = IOID_UNUSED; // Let application control CSN
}

/* SPI objects */
SPICC26XX_Object spiCC26XXObjects[SENSORTAG_CC2650_SPICOUNT];

/* SPI configuration structure */
const SPICC26XX_HWAttrs spiCC26XXHWAttrs[SENSORTAG_CC2650_SPICOUNT] = {
  { SSI0_BASE, INT_SSI0, PERIPH_SSI0 }, /* SENSORTAG_CC2650_SPI0 */
};

const SPI_Config SPI_config[] = {
  { &SPICC26XX_fxnTable, &spiCC26XXObjects[0], &spiCC26XXHWAttrs[0] },
  { NULL, NULL, NULL }
};

/*
 *  ======== SENSORTAG_CC2650_initSPI ========
 */
Void SENSORTAG_CC2650_initSPI(Void)
{
    /* Initialize driver */
    SPI_init();
    
    /* Initialize IOs */
    SPICC26XX_ioInit(SPI_config);
}
#endif /* TI_DRIVERS_SPI_INCLUDED */

#if TI_DRIVERS_SPI_INCLUDED
#include <ti/drivers/UDMA.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26XXDMA.h>

/* Init function of IOs for SPI */
void SPICC26XXDma_ioInit(const SPI_Config* SPI_config)
{
  /* Initialize the IOs for the SPI0 */
  ((SPICC26XXDMA_Object*)SPI_config[0].object)->spiMiso = SENSORTAG_CC2650_SPI0_MISO;
  ((SPICC26XXDMA_Object*)SPI_config[0].object)->spiMosi = SENSORTAG_CC2650_SPI0_MOSI;
  ((SPICC26XXDMA_Object*)SPI_config[0].object)->spiClk = SENSORTAG_CC2650_SPI0_CLK;
  ((SPICC26XXDMA_Object*)SPI_config[0].object)->spiCsn = IOID_UNUSED;  
}

/* SPI objects */
SPICC26XXDMA_Object spiCC26XXDMAobjects[SENSORTAG_CC2650_SPICOUNT];

/* SPI configuration structure, describing which pins are to be used */
const SPICC26XXDMA_HWAttrs spiCC26XXDMAHWAttrs[SENSORTAG_CC2650_SPICOUNT] = {
  {
    SSI0_BASE, INT_SSI0, PERIPH_SSI0, UDMA0_BASE,
    UDMA_CHAN_SSI0_RX, UDMA_CHAN_SSI0_TX,
  },
  {
    SSI1_BASE, INT_SSI1, PERIPH_SSI1, UDMA0_BASE,
    UDMA_CHAN_SSI1_RX, UDMA_CHAN_SSI1_TX,
  }
};

const SPI_Config SPI_config[] = {
  {&SPICC26XXDMA_fxnTable, &spiCC26XXDMAobjects[0], &spiCC26XXDMAHWAttrs[0]},
  {&SPICC26XXDMA_fxnTable, &spiCC26XXDMAobjects[1], &spiCC26XXDMAHWAttrs[1]},
  {NULL, NULL, NULL},
};

/*
*  ======== SENSORTAG_CC2650_initSPI ========
*/
Void SENSORTAG_CC2650_initSPI(Void)
{
  /* Initialize drivers */
	UDMA_init();
  SPI_init();
  
  /* Initialize IOs */
  SPICC26XXDma_ioInit(SPI_config);
}
#endif /* TI_DRIVERS_SPI_INCLUDED */

#if TI_DRIVERS_UART_INCLUDED
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

/* Init function of IOs for SPI */
void UARTCC26XX_ioInit(const UART_Config* UART_config)
{
    uint32_t pin;
    
    /* Initialize the IOs for the UART0 */
    pin = gpioHWAttrs[SENSORTAG_CC2650_UART_TX].pin;
    ((UARTCC26XX_Object*)UART_config[0].object)->uartTx = PIN2IOID(pin);
    pin = gpioHWAttrs[SENSORTAG_CC2650_UART_RX].pin;
    ((UARTCC26XX_Object*)UART_config[0].object)->uartRx = PIN2IOID(pin);
    ((UARTCC26XX_Object*)UART_config[0].object)->uartCts = IOID_UNUSED;
    ((UARTCC26XX_Object*)UART_config[0].object)->uartRts = IOID_UNUSED;
}

/* UART objects */
UARTCC26XX_Object uartCC26XXObjects[SENSORTAG_CC2650_UARTCOUNT];

/* UART configuration structure */
const UARTCC26XX_HWAttrs uartCC26XXHWAttrs[SENSORTAG_CC2650_UARTCOUNT] = {
    {UART0_BASE, INT_UART0, PERIPH_UART0}, /* SENSORTAG_CC2650_UART0 */
};

const UART_Config UART_config[] = {
    { &UARTCC26XX_fxnTable, &uartCC26XXObjects[0], &uartCC26XXHWAttrs[0] },
    { NULL, NULL, NULL }
};

/*
 *  ======== SENSORTAG_CC2650_initUART ========
 */
Void SENSORTAG_CC2650_initUART()
{
    /* Initialize the UART driver */
    UART_init();
    
    /* Initialize the UART IOs */
    UARTCC26XX_ioInit(UART_config);
}
#endif /* TI_DRIVERS_UART_INCLUDED */


#if TI_DRIVERS_WATCHDOG_INCLUDED
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/watchdog/WatchdogCC26XX.h>

/* Watchdog objects */
WatchdogCC26XX_Object watchdogCC26XXObjects[SENSORTAG_CC2650_WATCHDOGCOUNT];

/* Watchdog configuration structure */
const WatchdogCC26XX_HWAttrs watchdogCC26XXHWAttrs[SENSORTAG_CC2650_WATCHDOGCOUNT] = {
    /* SENSORTAG_CC2650_WATCHDOG0 with 1 sec period at default CPU clock freq */
    {WDT_BASE, INT_WATCHDOG, 48000000/2},
};

const Watchdog_Config Watchdog_config[] = {
    {&WatchdogCC26XX_fxnTable, &watchdogCC26XXObjects[0], &watchdogCC26XXHWAttrs[0]},
    {NULL, NULL, NULL},
};

/*
 *  ======== SENSORTAG_CC2650_initWatchdog ========
 */
Void SENSORTAG_CC2650_initWatchdog()
{
    /* Initialize the Watchdog driver */
    Watchdog_init();
}
#endif /* TI_DRIVERS_WATCHDOG_INCLUDED */


