//*****************************************************************************
//! @file       bsp.c
//! @brief      Board support package for CC26xx Cortex devices on SensorTag.
//!
//! Revised     $Date: 2014-11-07 04:45:28 -0800 (Fri, 07 Nov 2014) $
//! Revision    $Revision: 14396 $
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/


/**************************************************************************//**
* @addtogroup bsp_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#ifdef SENSORTAG_CC1350
#include "bsp_cc1350.h"
#else
#include "bsp.h"
#endif
#include "bsp_buzzer.h"

#include "inc/hw_sysctl.h"
#include "inc/hw_prcm.h"
#include "inc/hw_aon_rtc.h"
#include "inc/hw_aon_wuc.h"

#include "driverlib/debug.h"
#include "driverlib/prcm.h"
#include "driverlib/interrupt.h"
#include "driverlib/ioc.h"
#include "driverlib/i2c.h"
#include "driverlib/sys_ctrl.h"
#include "driverlib/aon_wuc.h"
#include "driverlib/aon_ioc.h"
#include "driverlib/aux_wuc.h"
#include "driverlib/osc.h"
#include "driverlib/rom.h"
#include "driverlib/pwr_ctrl.h"


/******************************************************************************
* LOCAL VARIABLES
*/

/******************************************************************************
* FUNCTIONS
*/

/**************************************************************************//**
* @brief    This function initializes the CC26xx clocks and I/O for use on
*           SmartRF06EB.
*
*           The function assumes that an external crystal oscillator is
*           available to the CC26xx. The CC26xx system clock is set to the
*           frequency given by input argument \e ui32SysClockSpeed. The I/O
*           system clock is set configured to the same value as the system
*           clock.
*
*           If the value of \e ui32SysClockSpeed is invalid, the system clock
*           is set to the highest allowed value.
*
* @param    ui32SysClockSpeed   is the system clock speed in Hz; it must be one
*                               of the following:
*           \li \b BSP_CLK_SPD_48MHZ
*           \li \b BSP_CLK_SPD_24MHZ
*
* @return   None
******************************************************************************/
void
bspInit(uint32_t ui32SysClockSpeed)
{
    uint32_t ui32SysDiv;

    //
    // Assert input arguments
    //
    ASSERT((ui32SysClockSpeed == BSP_CLK_SPD_48MHZ) ||
           (ui32SysClockSpeed == BSP_CLK_SPD_24MHZ));

    //
    // Disable global interrupts
    //
    bool bIntDisabled = IntMasterDisable();

    //
    // Turn on all power domains
    //
    uint32_t ui32DomainBm = (PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH);
    PRCMPowerDomainOn( ui32DomainBm);

    //
    // Wait for power on domains
    //
    while((PRCMPowerDomainStatus(ui32DomainBm) != PRCM_DOMAIN_POWER_ON))
    {
    }

    //
    // Determine clock divider
    //
    ui32SysDiv = (GET_MCU_CLOCK / ui32SysClockSpeed) - 1;

    // Configure all clock domains to run at full speed    //
    PRCMClockConfigureSet(PRCM_DOMAIN_SYSBUS, ui32SysDiv);
    PRCMClockConfigureSet(PRCM_DOMAIN_CPU, ui32SysDiv);
    PRCMClockConfigureSet(PRCM_DOMAIN_TIMER, ui32SysDiv);
    PRCMClockConfigureSet(PRCM_DOMAIN_SERIAL, ui32SysDiv);
    PRCMClockConfigureSet(PRCM_DOMAIN_PERIPH, ui32SysDiv);

    // Enable GPIO peripheral
    PRCMPeripheralRunEnable(PRCM_PERIPH_GPIO);

    //
    // Apply settings and wait for them to take effect
    //
    PRCMLoadSet();
    while(!PRCMLoadGet());
    
    //
    // Enable GPT0 module - Wait for the clock to be enabled
    //
    PRCMPeripheralRunEnable(PRCM_PERIPH_TIMER0);
    PRCMLoadSet();
    while(!PRCMLoadGet())
    { }
    
    //
    // Keys (input pullup)
    //
    IOCPinTypeGpioInput(BSP_IOID_KEY_LEFT);
    IOCPinTypeGpioInput(BSP_IOID_KEY_RIGHT);
    IOCIOPortPullSet(BSP_IOID_KEY_LEFT, IOC_IOPULL_UP);
    IOCIOPortPullSet(BSP_IOID_KEY_RIGHT, IOC_IOPULL_UP);
    
    //
    // Sensortage discrete interface
    //
    IOCPinTypeGpioInput(BSP_IOID_MPU_INT);
    IOCIOPortPullSet(BSP_IOID_MPU_INT, IOC_IOPULL_DOWN);

    IOCPinTypeGpioInput(BSP_IOID_REED_RELAY_INT);
    IOCIOPortPullSet(BSP_IOID_REED_RELAY_INT, IOC_IOPULL_DOWN);
    
    IOCPinTypeGpioOutput(BSP_IOID_MPU_POWER);
    GPIOPinWrite(BSP_MPU_POWER, 1);

    //
    // Flash interface
    //
    IOCPinTypeGpioOutput(BSP_IOID_FLASH_CS);
    GPIOPinWrite(BSP_FLASH_CS, 1);

    //
    // Devpack interface
    //
    IOCPinTypeGpioOutput(BSP_IOID_DEVPACK_CS);
    GPIOPinWrite(BSP_DEVPACK_CS, 1);

    IOCPinTypeGpioOutput(BSP_IOID_DEVPK_LCD_DISP);
    GPIOPinWrite(BSP_DEVPK_LCD_DISP, 1);

    IOCPinTypeGpioOutput(BSP_IOID_DEVPK_LCD_EXTMODE);
    GPIOPinWrite(BSP_DEVPK_LCD_EXTMODE, 0);

    IOCPinTypeGpioOutput(BSP_IOID_DEVPK_LCD_EXTCOMIN);
    GPIOPinWrite(BSP_DEVPK_LCD_EXTCOMIN, 0);

    //
    // Re-enable interrupt if initially enabled.
    //
    if(!bIntDisabled)
    {
        IntMasterEnable();
    }
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
