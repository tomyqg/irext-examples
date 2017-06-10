/**************************************************************************************************
  Filename:       time_clock.c
  Revised:        $Date: 2014-11-04 14:27:46 -0800 (Tue, 04 Nov 2014) $
  Revision:       $Revision: 40983 $

  Description:    Time clock display and timekeeping.

  Copyright 2014 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include <string.h>

#include "bcomdef.h"
#include "board_lcd.h"
#include "Board.h"

#include "UTC_Clock.h"
#include "time_clock.h"

#include <ti/drivers/lcd/LCDDogm1286.h>
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Month string
static const char timeMonthStr[12][3] =
{
  {'J', 'a', 'n'},
  {'F', 'e', 'b'},
  {'M', 'a', 'r'},
  {'A', 'p', 'r'},
  {'M', 'a', 'y'},
  {'J', 'u', 'n'},
  {'J', 'u', 'l'},
  {'A', 'u', 'g'},
  {'S', 'e', 'p'},
  {'O', 'c', 't'},
  {'N', 'o', 'v'},
  {'D', 'e', 'c'}
};
 
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static char *num2Str(char *pStr, uint8_t num);
static char *year2Str(char *pStr, uint16_t year);

/*********************************************************************
 * @fn      Time_clockInit()
 *
 * @brief   Initialize the Time clock.
 *
 * @return  none
 */
void Time_clockInit(void)
{
  // Start the UTC clock.
  UTC_init();
}

/*********************************************************************
 * @fn      Time_clockDisplay()
 *
 * @brief   Write the clock time to the display.
 *
 * @return  none
 */
void Time_clockDisplay(void)
{
  char displayBuf[20];
  char *p = displayBuf;
  UTCTimeStruct time;
  
  memset(displayBuf, 0x00, 20);
  
  // Get time structure from UTC.
  UTC_convertUTCTime(&time, UTC_getClock());
  
  // Display is in the format:
  // HH:MM MmmDD YYYY
  
  p = num2Str(p, time.hour);
  *p++ = ':';
  p = num2Str(p, time.minutes);
  *p++ = ' ';

  *p++ = timeMonthStr[time.month][0];  
  *p++ = timeMonthStr[time.month][1];
  *p++ = timeMonthStr[time.month][2];

  p = num2Str(p, time.day + 1);
  *p++ = ' ';
  
  p = year2Str(p, time.year);
  
  LCD_WRITE_STRING(displayBuf, LCD_PAGE2);  
}

/*********************************************************************
 * @fn      Time_clockSet()
 *
 * @brief   Set the clock. 
 *
 * @param   pData - Pointer to a Date Time characteristic structure
 *
 * @return  none
 */
void Time_clockSet(uint8_t *pData)
{
  UTCTimeStruct time;
  
  // Parse time service structure to UTC time structure.
  time.year = BUILD_UINT16(pData[0], pData[1]);
  if (time.year == 0)
  {
    time.year = 2000;
  }
  pData += 2;
  time.month = *pData++;
  if (time.month > 0)
  {
   // time.month--;
  }
  time.day = *pData++;
  if (time.day > 0)
  {
  //  time.day--;
  }
  time.hour = *pData++;
  time.minutes = *pData++;
  time.seconds = *pData;
  
  // Update UTC time.
  UTC_setClock(UTC_convertUTCSecs(&time));
}

/*********************************************************************
 * @fn      num2Str()
 *
 * @brief   Convert unsigned int 0-99 to decimal digit string.
 *
 * @return  pointer to string
 */
static char *num2Str(char *pStr, uint8_t num)
{
  *pStr++ = (num / 10) + '0';
  *pStr++ = (num % 10) + '0';
  
  return pStr;
}

/*********************************************************************
 * @fn      num2Str()
 *
 * @brief   Convert a year [9999-0000] to decimal digit string.
 *          Note: this assumes the device's longevity will not surpass
 *          year 9999.
 *
 * @return  pointer to string
 */
static char *year2Str(char *pStr, uint16_t year)
{
  //thousands
  *pStr++ = ((year / 1000) % 10) + '0';
  //hundreds
  *pStr++ = ((year / 100) % 10) + '0';
  //tens
  *pStr++ = ((year / 10) % 10) + '0';
  //units
  *pStr++ = (year & 10) + '0';
  
  return pStr;
}


/*********************************************************************
*********************************************************************/
