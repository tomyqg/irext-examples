//*****************************************************************************
//! @file       bsp.h
//! @brief      Board support package header file for CC26xx on SmartRF06EB.
//!
//! Revised     $Date: 2014-03-07 10:33:11 +0100 (fr, 07 mar 2014) $
//! Revision    $Revision: 12329 $
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
#ifndef __BSP_H__
#define __BSP_H__


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
* INCLUDES
*/
#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_sysctl.h>          // Access to the GET_MCU_CLOCK define
#include <inc/hw_ioc.h>
#include <driverlib/ioc.h>
#include <driverlib/gpio.h>


/******************************************************************************
* DEFINES
*/
// Clock speed defines
#define BSP_CLK_SPD_48MHZ       48000000UL
#define BSP_CLK_SPD_24MHZ       24000000UL
#define BSP_CLK_SPD_12MHZ       12000000UL
#define BSP_CLK_SPD_6MHZ        6000000UL
#define BSP_CLK_SPD_3MHZ        3000000UL
#define BSP_CLK_SPD_1_5MHZ      1500000UL
#define BSP_CLK_SPD_750KHZ      750000UL
#define BSP_CLK_SPD_375KHZ      375000UL
#define BSP_CLK_SPD_187_5KHZ    187500UL

//! Default system clock speed
#define BSP_SYS_CLK_SPD         BSP_CLK_SPD_48MHZ

// Board LED defines
#define BSP_IOID_LED_1          IOID_10
#define BSP_LED_1               (1 << BSP_IOID_LED_1)

#define BSP_IOID_LED_2          IOID_15
#define BSP_LED_2               (1 << BSP_IOID_LED_2)
  
#define BSP_LED_ALL             (BSP_LED_1 | BSP_LED_2)

  // Board buzzer defines
#define BSP_IOID_BUZZER         IOID_21
#define BSP_BUZZER              (1 << BSP_IOID_BUZZER)
  
// Board sensor power control defines
#define BSP_IOID_MPU_POWER       IOID_12
#define BSP_MPU_POWER           (1 << BSP_IOID_MPU_POWER)
  
#define BSP_IOID_MIC_POWER       IOID_13
#define BSP_MIC_POWER           (1 << BSP_IOID_MIC_POWER)

// Board key defines
#define BSP_IOID_KEY_LEFT       IOID_0
#define BSP_IOID_KEY_RIGHT      IOID_4
#define BSP_KEY_LEFT            (1 << BSP_IOID_KEY_LEFT)
#define BSP_KEY_RIGHT           (1 << BSP_IOID_KEY_RIGHT)
#define BSP_KEY_DIR_ALL         (BSP_KEY_LEFT |   BSP_KEY_RIGHT )
#define BSP_KEY_ALL             (BSP_KEY_DIR_ALL) //!< Bitmask of all keys

// Board flash defines
#define BSP_IOID_FLASH_CS       IOID_14
#define BSP_FLASH_CS            (1 << BSP_IOID_FLASH_CS)

// Board reed relay defines
#define BSP_IOID_REED_RELAY_INT IOID_1
#define BSP_REED_RELAY_INT      (1 << BSP_IOID_REED_RELAY_INT)

// Board sensor interface
#define BSP_IOID_MPU_INT        IOID_7
#define BSP_MPU_INT             (1 << BSP_IOID_MPU_INT)

#define BSP_IOD_SDA             IOID_5
#define BSP_IOD_SCL             IOID_6

#define BSP_IOD_SDA_HP          IOID_8
#define BSP_IOD_SCL_HP          IOID_9
  
// Board Light Skin interface
#define BSP_LIGHT_IOID_WHITE    IOID_27
#define BSP_LIGHT_WHITE         (1 << BSP_LIGHT_IOID_WHITE)
#define BSP_LIGHT_IOID_GREEN    IOID_23
#define BSP_LIGHT_GREEN         (1 << BSP_LIGHT_IOID_GREEN)
#define BSP_LIGHT_IOID_BLUE     IOID_24
#define BSP_LIGHT_BLUE          (1 << BSP_LIGHT_IOID_BLUE)
#define BSP_LIGHT_IOID_RED      IOID_25
#define BSP_LIGHT_RED           (1 << BSP_LIGHT_IOID_RED)
  
#define BSP_LIGHT_DIR_ALL       ( BSP_LIGHT_WHITE | BSP_LIGHT_RED | BSP_LIGHT_BLUE | BSP_LIGHT_GREEN)
#define BSP_LIGHT_ALL           ( BSP_LIGHT_DIR_ALL )

/******************************************************************************
* FUNCTION PROTOTYPES
*/
extern void bspInit(uint32_t ui32SysClockSpeed);
extern void bspSleep(void);

/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __BSP_H__ */
