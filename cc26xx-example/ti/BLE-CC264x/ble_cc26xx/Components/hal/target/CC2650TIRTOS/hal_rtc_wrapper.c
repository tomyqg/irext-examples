/*******************************************************************************
  Filename:       hal_rtc_wrapper.c
  Revised:        $Date: 2013-09-24 15:23:54 -0700 (Tue, 24 Sep 2013) $
  Revision:       $Revision: 35436 $

  Description:    This file provides various wrapper routines for the Always
                  On (AON) Real Time Clock (RTC).

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

/*******************************************************************************
 * INCLUDES
 */

#include "hal_mcu.h"
#include "hal_rtc_wrapper.h"
#include <inc/hw_memmap.h>
#include <inc/hw_aon_rtc.h>
#include <driverlib/aon_rtc.h>
#include <driverlib/aon_event.h>
#include "ICall.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define NUM_RTC_CHANS                  3

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// flag to indicate wake was due to RTC interrupt
static uint8 rtcIntOccurred[ NUM_RTC_CHANS ];

// RTC Interrupt Channel Callbacks
static rtcIntCBack_t rtcIntCback[NUM_RTC_CHANS];

/*******************************************************************************
 * PROTOTYPES
 */

uint8 halGetRtcIntFlag( uint8 rtcChan );
void  halSetRtcIntFlag( uint8 rtcChan );
void  halClrRtcIntFlag( uint8 rtcChan );

/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
 * @fn          halInitRTC
 *
 * @brief       This routine is used to initialize the Real Time Clock.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halInitRTC( void )
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION( cs );

  // disable the RTC interrupt
  ICall_disableInt( INT_AON_RTC );

  // turn off the RTC
  AONRTCDisable();

  // clear the RTC
  AONRTCReset();

  // disable/clear the RTC channel/event
  AONRTCChannelDisable( AON_RTC_CH0 );
  AONRTCEventClear( AON_RTC_CH0 );

  // disable/clear the RTC channel/event
  AONRTCChannelDisable( AON_RTC_CH1 );
  AONRTCEventClear( AON_RTC_CH1 );

  // disable/clear the RTC channel/event
  AONRTCChannelDisable( AON_RTC_CH2 );
  AONRTCEventClear( AON_RTC_CH2 );

  // clear flag that indicates a RTC interrupt has occurred
  halClrRtcIntFlag( AON_RTC_CH0 );
  halClrRtcIntFlag( AON_RTC_CH1 );
  halClrRtcIntFlag( AON_RTC_CH2 );

  // clear callback pointers
  halRtcRegIntCBack( AON_RTC_CH0, NULL );
  halRtcRegIntCBack( AON_RTC_CH1, NULL );
  halRtcRegIntCBack( AON_RTC_CH2, NULL );

  // Make sure there is no delay when event happens
  AONRTCDelayConfig( AON_RTC_CONFIG_DELAY_NODELAY );

  // Set the combined delayed event as a union of channel 0 and channel 1
  // Note: This is all that's needed if only an interrupt is required. If the
  //       entire CM3 core is shutdown, then AON_EVENT_MCU_WUx has to be used
  //       to associate a core wakeup event with the RTC channel.
  AONRTCCombinedEventConfig( AON_RTC_CH0 | AON_RTC_CH1 );

  // Set the combined event as wake up source, and setup wakeup event
  // Note: This is required when using DeepSleep with uLDO!
  AONEventMcuWakeUpSet( AON_EVENT_MCU_WU0, AON_EVENT_RTC0 );
  AONEventMcuWakeUpSet( AON_EVENT_MCU_WU1, AON_EVENT_RTC1 );

  // enable the RTC
  AONRTCEnable();

  // enable the RTC interrupt
  ICall_enableInt( INT_AON_RTC );

  HAL_EXIT_CRITICAL_SECTION( cs );

  return;
}


/*******************************************************************************
 * @fn          halSetRtcTimerEvent
 *
 * @brief       This function sets the CC26xx RTC compare value on the given
 *              RTC channel to generate an RTC interrupt event.
 *
 *              Note: This assumes you don't need t oset mode.
 *
 * input parameters
 *
 * @param       rtcChan - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 * @param       compVal - RTC Compare value.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halSetRtcTimerEvent(uint32 rtcChan, uint32 compVal  )
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION( cs );

  // clear the RTC channel
  AONRTCChannelDisable( rtcChan );

  // clear the RTC event
  AONRTCEventClear( rtcChan );

  if ( rtcChan == AON_RTC_CH1 )
  {
    // ensure the channel is setup for compare
    AONRTCModeCh1Set( AON_RTC_MODE_CH1_COMPARE );
  }
  else if ( rtcChan == AON_RTC_CH2 )
  {
    // ensure the channel is setup for non-continuous
    AONRTCModeCh2Set( AON_RTC_MODE_CH2_NORMALCOMPARE );
  }

  // clear flag that indicates a RTC interrupt has occurred
  halClrRtcIntFlag( rtcChan );

  // configure channel to generate an event for the determined interval
  AONRTCCompareValueSet( rtcChan, compVal );

  // enable event generation for channel
  AONRTCChannelEnable( rtcChan );

  HAL_EXIT_CRITICAL_SECTION( cs );

  return;
}


/*******************************************************************************
 * @fn          halClearRtcTimerEvent
 *
 * @brief       This function clears the CC26xx RTC timer interrupt event.
 *
 * input parameters
 *
 * @param       rtcChan - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halClearRtcTimerEvent( uint32 rtcChan )
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION( cs );

  // disable event generation for channel 0
  AONRTCChannelDisable( rtcChan );

  // clear the RTC event
  AONRTCEventClear( rtcChan );

  // clear flag that indicates a RTC interrupt has occurred
  halClrRtcIntFlag( rtcChan );

  HAL_EXIT_CRITICAL_SECTION( cs );

  return;
}


/*******************************************************************************
 * @fn          halReadRtcTimer
 *
 * @brief       This function reads the CC26xx RTC and returns 16 bits of sec
 *              and 16 bits of subsec.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      A snapshot of RTC sec:subsec.
 */
uint32 halReadRtcTimer( void )
{
  uint32        secTime;
  uint32        fracTime;
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION( cs );

  // latch the RTC timer to get the fractional part
  // Note: Check if the frac part rolled over by comparing a re-read of SEC.
  do
  {
    // latch second and fractional part of RTC
    secTime = AONRTCSecGet();

    // read fractional part
    fracTime = AONRTCFractionGet();

  } while( AONRTCSecGet() != secTime );

  // read fractional part
  fracTime = AONRTCFractionGet();

  HAL_EXIT_CRITICAL_SECTION( cs );

  // return sec:subsec
  return( (secTime<<16) | (fracTime>>16) );
}


/*******************************************************************************
 * @fn          halGetRtcIntFlag
 *
 * @brief       This function reads the state of the RTC interrupt flag for
 *              a given channel.
 *
 * input parameters
 *
 * @param       rtcChan - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      The RTC interrupt flag for the given channel.
 */
uint8 halGetRtcIntFlag( uint8 rtcChan )
{
  // return the RTC Interrupt flag for the given channel
  if ( rtcChan == AON_RTC_CH0 )
  {
    return( rtcIntOccurred[0] );
  }
  else if ( rtcChan == AON_RTC_CH1 )
  {
    return( rtcIntOccurred[1] );
  }
  else // AON_RTC_CH2
  {
    return( rtcIntOccurred[2] );
  }
}


/*******************************************************************************
 * @fn          halSetRtcIntFlag
 *
 * @brief       This function sets the state of the RTC interrupt flag for
 *              a given channel.
 *
 * input parameters
 *
 * @param       rtcChan - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halSetRtcIntFlag( uint8 rtcChan )
{
  // set the RTC Interrupt flag for the given channel
  if ( rtcChan == AON_RTC_CH0 )
  {
    rtcIntOccurred[0] = TRUE;
  }
  else if ( rtcChan == AON_RTC_CH1 )
  {
    rtcIntOccurred[1] = TRUE;
  }
  else // AON_RTC_CH2
  {
    rtcIntOccurred[2] = TRUE;
  }

  return;
}


/*******************************************************************************
 * @fn          halClrRtcIntFlag
 *
 * @brief       This function clears the state of the RTC interrupt flag for
 *              a given channel.
 *
 * input parameters
 *
 * @param       rtcChan - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halClrRtcIntFlag( uint8 rtcChan )
{
  // clear the RTC Interrupt flag for the given channel
  if ( rtcChan == AON_RTC_CH0 )
  {
    rtcIntOccurred[0] = FALSE;
  }
  else if ( rtcChan == AON_RTC_CH1 )
  {
    rtcIntOccurred[1] = FALSE;
  }
  else // AON_RTC_CH2
  {
    rtcIntOccurred[2] = FALSE;
  }

  return;
}


/*******************************************************************************
 * @fn          halRtcTimerDelta
 *
 * @brief       This function returns the delta of two RTC timer values. It is
 *              intended that time1 is the first timestamp, and time2 is the
 *              second timestamp. The result is the same format as the RTC
 *              timer (i.e. sec: subsec).
 *
 * input parameters
 *
 * @param       time1 - First timestamp of RTC.
 * @param       time2 - Second timestamp of RTC.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      The delta of time1 and time2 as a RTC sec:subsec.
 */
uint32 halRtcTimerDelta( uint32 time1, uint32 time2 )
{
  uint32 sec1    = time1 >> 16;
  uint32 subsec1 = time1 & 0xFFFF;
  uint32 sec2    = time2 >> 16;
  uint32 subsec2 = time2 & 0xFFFF;
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION( cs );

  // check if time2 sec time later than time1
  if ( sec2 > sec1 )
  {
    // check time2 subsec time same or later than time
    if ( subsec2 >= subsec1 )
    {
      // take the difference for both sec and subsec
      sec1 = sec2 - sec1;
      subsec1 = subsec2 - subsec1;
    }
    else // subsec2 < subsec1
    {
      // Note: sec1 equals sec2 means we've wrapped (>18.2 hours)!
      //ASSERT( sec1 != sec2 );

      // reduce sec delta by one to account for round up of subsec
      sec1 = (sec2 - sec1) - 1;
      subsec1 = (0x10000 - subsec1) + subsec2;
    }
  }
  else // sec2 <= sec1; handle wrap
  {
    // determine time1 sec to wraparound
    sec1 = 0x10000 - sec1;

    // check if time1 subsec affects number of time1 sec to wraparound
    if ( subsec1 > 0 )
    {
      // reduce time 1 sec to wraparound by one due to time1 subsec roundup
      sec1--;

      // determine time1 subsec to wraparound
      subsec1 = 0x10000 - subsec1;
    }

    // combine time1 sec to wraparound with time2 sec
    sec1 += sec2;

    // combine time1 subsec to wraparound with time2 subsec
    subsec1 += subsec2;

    // check if subsec rolled over
    if ( subsec1 > 0xFFFF )
    {
      // adjust sec due to subsec rollover
      sec1++;

      // correct subsec due to rollover
      subsec1 -= 0x10000;
    }
  }

  HAL_EXIT_CRITICAL_SECTION( cs );

  // combine delta sec and subsec
  return( (sec1 << 16) | (subsec1 & 0xFFFF) );
}


/*******************************************************************************
 * @fn          halRtcTimerCompare
 *
 * @brief       This function determines if the first time parameter is greater
 *              than the second time parameter, taking timer counter wrap into
 *              account. If so, TRUE is returned.
 *
 * input parameters
 *
 * @param       time1 - First timestamp of RTC.
 * @param       time2 - Second timestamp of RTC.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE:  When first parameter is greater than second.
 *              FALSE: When first parameter is less than or equal to
 *                     the second.
 */
uint32 halRtcTimerCompare( uint32 time1, uint32 time2 )
{
  return( TRUE );
}


/*******************************************************************************
 * @fn          halRtcRegIntCBack
 *
 * @brief       This function determines if the first time parameter is greater
 *              than the second time parameter, taking timer counter wrap into
 *              account. If so, TRUE is returned.
 *
 * input parameters
 *
 * @param       rtcChan  - AON_RTC_CH0, AON_RTC_CH1, or AON_RTC_CH2
 * @param       rtcCback - Callback function.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void halRtcRegIntCBack( uint8 rtcChan, rtcIntCBack_t rtcCback )
{
  // return RTC interrupt flag
  if ( rtcChan == AON_RTC_CH0 )
  {
    rtcIntCback[0] = rtcCback;
  }
  else if ( rtcChan == AON_RTC_CH1 )
  {
    rtcIntCback[1] = rtcCback;
  }
  else // AON_RTC_CH2
  {
    rtcIntCback[2] = rtcCback;
  }

  return;
}


/*******************************************************************************
 * @fn          Real Time Clock ISR
 *
 * @brief       This ISR handles the RTC interrupt for Channel 0 only.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void RTC_ISR( void )
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION(cs);

  CLEAR_SLEEP_MODE();

  // check if a RTC Channel 0 interrupt occurred
  if ( AONRTCEventGet( AON_RTC_CH0 ) )
  {
    halSetRtcIntFlag( AON_RTC_CH0 );

    // make the callback for this channel
    if (rtcIntCback[0]) (rtcIntCback[0])();

    halClearRtcTimerEvent( AON_RTC_CH0 );
  }

  if ( AONRTCEventGet( AON_RTC_CH1 ) )
  {
    halSetRtcIntFlag( AON_RTC_CH1 );

    // make the callback for this channel
    if (rtcIntCback[1]) (rtcIntCback[1])();

    halClearRtcTimerEvent( AON_RTC_CH1 );
  }

  if ( AONRTCEventGet( AON_RTC_CH2 ) )
  {
    halSetRtcIntFlag( AON_RTC_CH2 );

    // make the callback for this channel
    if (rtcIntCback[2]) (rtcIntCback[2])();

    halClearRtcTimerEvent( AON_RTC_CH2 );
  }

  HAL_EXIT_CRITICAL_SECTION(cs);

  return;
}

/*******************************************************************************
 */



