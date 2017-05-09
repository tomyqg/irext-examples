/*******************************************************************************
  Filename:       hal_gpio_wrapper.h
  Revised:        $Date: 2013-09-27 13:53:32 -0700 (Fri, 27 Sep 2013) $
  Revision:       $Revision: 35474 $

  Description:    This file contains a simplified GPIO mapping for debug.

  Copyright 2013-2015 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
*******************************************************************************/

#ifndef HAL_GPIO_WRAPPER_H
#define HAL_GPIO_WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

/*******************************************************************************
 * MACROS
 */

#ifdef DEBUG_GPIO

#define HAL_GPIO_INIT()                                                        \
  {                                                                            \
    IOCPinTypeGpioOutput( HAL_GPIO_1 );   /* SmartRF06EB LED3    P403.2 */     \
    IOCPinTypeGpioOutput( HAL_GPIO_2 );   /* SmartRF06EB LED4    P403.4 */     \
    IOCPinTypeGpioOutput( HAL_GPIO_3 );   /* SmartRF06EB LED1    P404.20 */    \
    IOCPinTypeGpioOutput( HAL_GPIO_4 );   /* SmartRF06EB LED2    P405.4 */     \
    IOCPinTypeGpioOutput( HAL_GPIO_5 );   /* SmartRF06EB NC      P403.6 */     \
    IOCPinTypeGpioOutput( HAL_GPIO_6 );   /* SmartRF06EB BTN_L   P403.8 */     \
    IOCPinTypeGpioOutput( HAL_GPIO_7 );   /* SmartRF06EB BTN_R   P403.10 */    \
    IOCPinTypeGpioOutput( HAL_GPIO_8 );   /* SmartRF06EB BTN_UP  P403.12 */    \
  }

#define HAL_GPIO_SET(x)                                                        \
  {                                                                            \
    GPIOPinWrite((1<<(x)), 1);                                                 \
  }

#define HAL_GPIO_CLR(x)                                                        \
  {                                                                            \
    GPIOPinWrite((1<<(x)), 0);                                                 \
  }

#else // !DEBUG_GPIO

#define HAL_GPIO_INIT()
#define HAL_GPIO_SET(x)
#define HAL_GPIO_CLR(x)

#endif // DEBUG_GPIO

/*******************************************************************************
 * CONSTANTS
 */

#define HAL_GPIO_1                IOID_7   // GPIO11 SmartRF06EB LED3    P403.2
#define HAL_GPIO_2                IOID_6   // GPI012 SmartRF06EB LED4    P403.4
#define HAL_GPIO_3                IOID_25  // GPIO24 SmartRF06EB LED1    P404.20
#define HAL_GPIO_4                IOID_27  // GPIO22 SmartRF06EB LED2    P405.4
#define HAL_GPIO_5                IOID_1   // GPIO17 SmartRF06EB NC      P403.6
#define HAL_GPIO_6                IOID_15  // GPIO3  SmartRF06EB BTN_L   P403.8
#define HAL_GPIO_7                IOID_18  // GPIO31 SmartRF06EB BTN_R   P403.10
#define HAL_GPIO_8                IOID_19  // GPIO30 SmartRF06EB BTN_UP  P403.12

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_WRAPPER_H */

