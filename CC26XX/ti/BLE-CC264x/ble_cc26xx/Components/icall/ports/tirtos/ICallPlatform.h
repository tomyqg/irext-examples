/**
  @file  ICallPlatform.h
  @brief Platform specific function interfaces required for ICall implementation

  <!--
  Copyright 2013 - 2015 Texas Instruments Incorporated. All rights reserved.

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
#ifndef ICALLPLATFORM_H
#define ICALLPLATFORM_H

#include <stdint.h>

#include "ICall.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Functions in platform independent ICall module that can be used
 * by platform dependent functions.
 */
extern ICall_CSState ICall_enterCSImpl(void);
extern void ICall_leaveCSImpl(ICall_CSState key);
extern void *ICall_mallocImpl(uint_fast16_t size);
extern void ICall_freeImpl(void *ptr);

#ifdef ICALL_HOOK_ABORT_FUNC
extern void ICALL_HOOK_ABORT_FUNC();
#else /* ICALL_HOOK_ABORT_FUNC */
/* Note that customer can use their own assert handler */
#include <stdlib.h>
/**
 * Abort function definition.
 * Note that at compile time, this macro can be overridden
 * to point to another function of void fn(void) type.
 */
#if defined (__IAR_SYSTEMS_ICC__)
#pragma diag_suppress=Pe111
#endif // __IAR_SYSTEMS_ICC__
#define ICALL_HOOK_ABORT_FUNC() abort()
#endif /* ICALL_HOOK_ABORT */

/**
 * @internal
 * Updates power activity counter
 *
 * @param args  arguments
 * @return return value of ICall_pwrUpdateActivityCounter()
 */
extern ICall_Errno
ICallPlatform_pwrUpdActivityCounter(ICall_PwrUpdActivityCounterArgs *args);

/**
 * @internal
 * Registers power state transition notify function
 *
 * @param args  arguments
 * @return return values of ICall_pwrRegisterNotify()
 */
extern ICall_Errno
ICallPlatform_pwrRegisterNotify(ICall_PwrRegisterNotifyArgs *args);

/**
 * @internal
 * Configures power activity counter action
 *
 * @param args arguments
 * @return return value of ICall_pwrConfigACAction()
 */
extern ICall_Errno
ICallPlatform_pwrConfigACAction(ICall_PwrBitmapArgs *args);

/**
 * @internal
 * Sets power constraints and dependencies
 *
 * @param args arguments
 * @return return value of ICall_pwrRequire()
 */
extern ICall_Errno
ICallPlatform_pwrRequire(ICall_PwrBitmapArgs *args);

/**
 * @internal
 * Releases power constraints and dependencies
 *
 * @param args arguments
 * @return return value of ICall_pwrDispense()
 */
extern ICall_Errno
ICallPlatform_pwrDispense(ICall_PwrBitmapArgs *args);

/**
 * @internal
 * Checks whether HF XOSC is stable.
 *
 * @return ICALL_ERRNO_SUCCESS
 */
extern ICall_Errno
ICallPlatform_pwrIsStableXOSCHF(ICall_GetBoolArgs* args);

/**
 * @internal
 * Switches clock source to HF XOSC.
 *
 * @return ICALL_ERRNO_SUCCESS
 */
extern ICall_Errno
ICallPlatform_pwrSwitchXOSCHF(ICall_FuncArgsHdr* args);

/**
 * @internal
 * Get the estimated crystal oscillator startup time
 *
 * @return ICALL_ERRNO_SUCCESS
 */
extern ICall_Errno
ICallPlatform_pwrGetXOSCStartupTime(ICall_PwrGetXOSCStartupTimeArgs * args);

/**
 * @internal
 * Retrieves power transition state.
 *
 * @return @ref ICALL_ERRNO_SUCCESS
 */
extern ICall_Errno
ICallPlatform_pwrGetTransitionState(ICall_PwrGetTransitionStateArgs *args);

#ifdef __cplusplus
}
#endif

#endif /* ICALLPLATFORM_H */
