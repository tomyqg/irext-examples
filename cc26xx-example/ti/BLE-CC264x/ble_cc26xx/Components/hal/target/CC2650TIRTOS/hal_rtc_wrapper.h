/*******************************************************************************
  Filename:       hal_rtc_wrapper.h
  Revised:        $Date: 2013-09-24 15:23:54 -0700 (Tue, 24 Sep 2013) $
  Revision:       $Revision: 35436 $

  Description:    This file contains the HAL RTC Wrapper types, constants,
                  API's etc.

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

#ifndef HAL_RTC_WRAPPER_H
#define HAL_RTC_WRAPPER_H

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

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef void (*rtcIntCBack_t)( void );

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * Functions
 */

extern void   halInitRTC( void );

extern void   halSetRtcTimerEvent(uint32 rtcChan, uint32 compVal  );

extern void   halClearRtcTimerEvent( uint32 rtcChan );

extern uint32 halReadRtcTimer( void );

extern void   halSetRtcIntFlag( uint8 rtcChan );

extern uint8  halGetRtcIntFlag( uint8 rtcChan );

extern void   halClrRtcIntFlag( uint8 rtcChan );

extern uint32 halRtcTimerDelta( uint32 time1, uint32 time2 );

extern uint32 halRtcTimerCompare( uint32 time1, uint32 time2 );

extern void   halRtcRegIntCBack( uint8 rtcChan, rtcIntCBack_t rtcCback );

extern void   RTC_ISR( void );

#ifdef __cplusplus
}
#endif

#endif /* HAL_RTC_WRAPPER_H */
