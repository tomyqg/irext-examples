/**************************************************************************************************
  Filename:       hal_mcu.h
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2015 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

#ifndef HAL_MCU_H
#define HAL_MCU_H



/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>

#include "hal_defs.h"
#include "hal_types.h"
#include <inc/hw_nvic.h>
#include <inc/hw_ints.h>
#include <inc/hw_types.h>

/* configuration register definitions */
#include <inc/hw_gpio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>

#include <driverlib/systick.h>
#include <driverlib/interrupt.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/flash.h>
#include <driverlib/ioc.h>


/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Abstraction
 * ------------------------------------------------------------------------------------------------
 */

/* ---------------------- IAR Compiler ---------------------- */
#if defined (__IAR_SYSTEMS_ICC__)
#define HAL_COMPILER_IAR
#define HAL_MCU_LITTLE_ENDIAN()   __LITTLE_ENDIAN__

/* ---------------------- Keil Compiler ---------------------- */
#elif defined (__KEIL__)
#define HAL_COMPILER_KEIL
#define HAL_MCU_LITTLE_ENDIAN()   0


/* ------------------ Unrecognized Compiler ------------------ */
#elif defined (ccs) || defined __TI_COMPILER_VERSION__ || defined (__GNUC__)
#define HAL_MCU_LITTLE_ENDIAN()   1
//do nothing for now
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                       Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */

#ifdef USE_ICALL
#include <ICall.h>

typedef ICall_CSState halIntState_t;

/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()     ICall_enableMInt()

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()    ICall_disableMInt()

/* Enter critical section */
#define HAL_ENTER_CRITICAL_SECTION(x) st(x = ICall_enterCriticalSection();)

/* Exit critical section */
#define HAL_EXIT_CRITICAL_SECTION(x) ICall_leaveCriticalSection(x)

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT()    \
{                                    \
  ICall_enableInt(INT_RFCORERTX);    \
}

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() \
{                                       \
  ICall_enableInt(INT_RFCOREERR);       \
}

/* Note that check of whether interrupts are enabled or not is not supported
 * by any random operating system.
 * Hence, the call to HAL_INTERRUPTS_ARE_ENABLED() itself must not be made
 * from the beginning.
 */
#define HAL_INTERRUPTS_ARE_ENABLED() FALSE

#elif defined OSAL_PORT2TIRTOS

#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Task.h>

typedef int halIntState_t;

/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()                 \
  do { Hwi_enable(); Task_enable(); } while (0)

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()                \
  do { Task_disable(); Hwi_disable(); } while (0)

/* Enter critical section */
#define HAL_ENTER_CRITICAL_SECTION(x)                   \
  do { extern void zipEnterCriticalSection(void);       \
    (void) x; zipEnterCriticalSection(); } while (0)

/* Exit critical section */
#define HAL_EXIT_CRITICAL_SECTION(x)                    \
  do { extern void zipExitCriticalSection(void);        \
    (void) x; zipExitCriticalSection(); } while (0)

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT() Hwi_enableInterrupt(INT_RFCORERTX)

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() Hwi_enableInterrupt(INT_RFCOREERR)

/* Note that check of whether interrupts are enabled or not is not supported
 * by any random operating system.
 * Hence, the call to HAL_INTERRUPTS_ARE_ENABLED() itself must not be made
 * from the beginning.
 */
#define HAL_INTERRUPTS_ARE_ENABLED() FALSE

#else /* OSAL_PORT2TIRTOS */

typedef bool halIntState_t;

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT()    \
{                                    \
  IntEnable(INT_RFCORERTX);          \
}

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() \
{                                       \
  IntEnable(INT_RFCOREERR);             \
}

/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()     IntMasterEnable()

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()    IntMasterDisable()

static bool halIntsAreEnabled(void)
{
  bool status = !IntMasterDisable();
  if (status)
  {
    IntMasterEnable();
  }
  return status;
}

#define HAL_INTERRUPTS_ARE_ENABLED() halIntsAreEnabled()

#define HAL_ENTER_CRITICAL_SECTION(x)  \
  do { (x) = !IntMasterDisable(); } while (0)

#define HAL_EXIT_CRITICAL_SECTION(x) \
  do { if (x) { (void) IntMasterEnable(); } } while (0)

#endif /* USE_ICALL */

#define HAL_NON_ISR_ENTER_CRITICAL_SECTION(x)  HAL_ENTER_CRITICAL_SECTION(x)
#define HAL_NON_ISR_EXIT_CRITICAL_SECTION(x)   HAL_EXIT_CRITICAL_SECTION(x)

/* Hal Critical statement definition */
#define HAL_CRITICAL_STATEMENT(x)       st( halIntState_t s; HAL_ENTER_CRITICAL_SECTION(s); x; HAL_EXIT_CRITICAL_SECTION(s); )

/* Enable Key/button interrupts */
#define HAL_ENABLE_PUSH_BUTTON_PORT_INTERRUPTS()  \
{                                                 \
  ICall_enableInt(INT_GPIOC);                           \
  ICall_enableInt(INT_GPIOA);                           \
}

/* Disable Key/button interrupts */
#define HAL_DISABLE_PUSH_BUTTON_PORT_INTERRUPTS()  \
{                                                  \
  ICall_enableInt(INT_GPIOC);                           \
  ICall_enableInt(INT_GPIOA);                           \
}

/* ------------------------------------------------------------------------------------------------
 *                                        Reset Macro
 * ------------------------------------------------------------------------------------------------
 */
// Perform the equivalent of a PIN Reset (hard reset).
// The cc26xx system has not been design to handle soft reset.
// Making a soft reset can make the system unstable. 
// All soft reset needs to be replace by Hard reset.

#define HAL_SYSTEM_RESET()  (HWREG( AON_SYSCTL_BASE + AON_SYSCTL_O_RESETCTL ) |= AON_SYSCTL_RESETCTL_SYSRESET_M);

/* ------------------------------------------------------------------------------------------------
 *                                        Sleep common code stubs
 * ------------------------------------------------------------------------------------------------
 */
#define CLEAR_SLEEP_MODE()
#define ALLOW_SLEEP_MODE()


/* ------------------------------------------------------------------------------------------------
 *                                        Dummy for this platform
 * ------------------------------------------------------------------------------------------------
 */
#define HAL_AES_ENTER_WORKAROUND() 
#define HAL_AES_EXIT_WORKAROUND() 
/**************************************************************************************************
 */
#endif

