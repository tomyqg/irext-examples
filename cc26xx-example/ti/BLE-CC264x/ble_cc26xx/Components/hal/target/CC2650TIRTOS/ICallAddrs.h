/**
  @file  ICallAddrs.h
  @brief Stack image address information specific to build

  <!--
  Copyright 2013 - 2014 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED ``AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
#ifndef ICALLADDRS_H
#define ICALLADDRS_H

#include <ICall.h>

#ifndef USE_DEFAULT_USER_CFG

#include "hal_types.h"
#include "bleUserConfig.h"

extern bleUserCfg_t user0Cfg;

#define USER0_CFG                      &user0Cfg

#else

#define USER0_CFG                      NULL

#endif // USE_DEFAULT_USER_CFG

/**
 * An entry function address of an external image.
 * Note that it is not necessary to define this macro
 * in this header file. The macro is internally used
 * to expand @ref ICALL_ADDR_MAPS and also used
 * for the sample DummyStack code.
 */
#ifndef ICALL_STACK0_ADDR
#define ICALL_STACK0_ADDR              0x00008000ul
#endif

/**
 * Initializer for an array of @ref ICall_RemoteTaskEntry.
 * Each element of the array corresponds to an entry function
 * of an external image.
 * The function address must be an odd address for CC2650
 * so that call will be made in Thumb mode
 */
#define ICALL_ADDR_MAPS \
{ \
  (ICall_RemoteTaskEntry) (ICALL_STACK0_ADDR + 1) \
}

/**
 * Initializer for an array of thread priorities.
 * Each element of the array corresponds to TI-RTOS specific
 * thread priority value given to a thread to be created
 * per the entry function defined in @ref ICALL_ADDR_MAPS
 * initializer, in the same sequence.
 */
#define ICALL_TASK_PRIORITIES { 5 }

/**
 * Initializer for an array of thread stack sizes.
 * Each element of the array corresponds to stack depth
 * allocated to a thread to be created per the entry function
 * defined in @ref ICALL_ADDR_MAPS initializer, in the same sequence.
 */
#define ICALL_TASK_STACK_SIZES { 800 }

/**
 * Initializer for custom initialization parameters.
 * Each element of the array corresponds to initialization parameter
 * (a pointer) specific to the image to be passed to the entry function
 * defined in @ref ICALL_ADDR_MAPS initializer.
 */
#define ICALL_CUSTOM_INIT_PARAMS { USER0_CFG }

#endif /* ICALLADDRS_H */
