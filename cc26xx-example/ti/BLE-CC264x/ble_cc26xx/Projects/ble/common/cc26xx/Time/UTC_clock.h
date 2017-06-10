/******************************************************************************
  Filename:       UTC_Clock.h
  Revised:        $Date: 2015-07-22 10:45:09 -0700 (Wed, 22 Jul 2015) $
  Revision:       $Revision: 44392 $

  Description:    UTC Clock types and functions prototypes.


  Copyright 2004 - 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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
******************************************************************************/

#ifndef UTC_CLOCK_H
#define UTC_CLOCK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

// number of seconds since 0 hrs, 0 minutes, 0 seconds, on the
// 1st of January 2000 UTC
typedef uint32_t UTCTime;

// UTC time structs broken down until standard components.
typedef struct
{
  uint8_t seconds;  // 0-59
  uint8_t minutes;  // 0-59
  uint8_t hour;     // 0-23
  uint8_t day;      // 0-30
  uint8_t month;    // 0-11
  uint16_t year;    // 2000+ 
} UTCTimeStruct;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * @fn      UTC_init
 *
 * @brief   Initialize the UTC clock module.  Sets up and starts the
 *          clock instance.
 *
 * @param   None.
 *
 * @return  None.
 */
extern void UTC_init(void);

/*
 * Set the new time.  This will only set the seconds portion
 * of time and doesn't change the factional second counter.
 *     newTime - number of seconds since 0 hrs, 0 minutes,
 *               0 seconds, on the 1st of January 2000 UTC
 */
extern void UTC_setClock( UTCTime newTime );

/*
 * Gets the current time.  This will only return the seconds
 * portion of time and doesn't include the factional second counter.
 *     returns: number of seconds since 0 hrs, 0 minutes,
 *              0 seconds, on the 1st of January 2000 UTC
 */
extern UTCTime UTC_getClock( void );

/*
 * Converts UTCTime to UTCTimeStruct
 *
 * secTime - number of seconds since 0 hrs, 0 minutes,
 *          0 seconds, on the 1st of January 2000 UTC
 * tm - pointer to breakdown struct
 */
extern void UTC_convertUTCTime( UTCTimeStruct *tm, UTCTime secTime );

/*
 * Converts UTCTimeStruct to UTCTime (seconds since 00:00:00 01/01/2000)
 *
 * tm - pointer to UTC time struct
 */
extern UTCTime UTC_convertUTCSecs( UTCTimeStruct *tm );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* UTC_CLOCK_H */
