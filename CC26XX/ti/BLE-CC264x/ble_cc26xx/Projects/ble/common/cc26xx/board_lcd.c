/*******************************************************************************
  Filename:       board_lcd.c
  Revised:        $Date: 2014-03-10 07:29:12 -0700 (Mon, 10 Mar 2014) $
  Revision:       $Revision: 37597 $

  Description:    This file contains the interface to the SRF06EB LCD Service.

  Copyright 2014 - 2015 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */
#include <ti/drivers/lcd/LCDDogm1286.h>

#include "board_lcd.h"
#include "Board.h"

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// LCD parameter
static LCD_Handle lcdHandle = NULL;

#ifdef TI_DRIVERS_LCD_INCLUDED
// LCD pin table
PIN_Config LCDPinTable[] = {
    Board_3V3_EN     | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL,  // Enable 3V3 domain. Need to be high for LCD to work.
    PIN_TERMINATE                                                           // Terminate list
};

// LCD pin state
PIN_State LCDPinState;

// LCD pin handle
PIN_Handle hLCDPins;
#endif //TI_DRIVERS_LCD_INCLUDED

Char lcdBuffer0[LCD_BYTES] = {0};

LCD_Buffer lcdBuffers[] = {
      {lcdBuffer0, LCD_BYTES, NULL},
  };
  
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Board_openLCD
 *
 * @brief   Open LCD peripheral on SRF06EB.
 *
 * @param   none
 *
 * @return  void
 */
void Board_openLCD(void)
{
#ifdef TI_DRIVERS_LCD_INCLUDED
  //Enable the 3V3 domain for the LCD
  hLCDPins = PIN_open(&LCDPinState, LCDPinTable);
    
  LCD_Params lcdParams;
  
  LCD_Params_init(&lcdParams);
  
  // Open LCD peripheral
  lcdHandle = LCD_open(&lcdBuffers[0], 1, &lcdParams);
  
  if ( lcdHandle )
  {
    LCD_bufferClear(lcdHandle, 0);
    LCD_update(lcdHandle, 0);
  }
#endif
}

/*********************************************************************
 * @fn      Board_writeString
 *
 * @brief   Write a string on the LCD display.
 *
 * @param   str - string to print
 * @param   line - line (page) to write (0-7)
 *
 * @return  void
 */
void Board_writeString(char *str, uint8_t line)
{
  if (lcdHandle != NULL)
  {
    LCD_bufferClearPage(lcdHandle, 0, (LCD_Page)line);
    LCD_bufferPrintString(lcdHandle, 0, str, 0, (LCD_Page)line);
    LCD_update(lcdHandle, 0);
  }
}

/*********************************************************************
 * @fn      Board_writeStringValue
 *
 * @brief   Write a string and value on the LCD display.
 *
 * @param   str - string to print
 * @param   value - value to print
 * @param   format - base of the value to print (2,8,16 etc)
 * @param   line - line (page) to write (0-7)
 *
 * @return  void
 */
void Board_writeStringValue(char *str, uint32_t value, uint8_t format,
                            uint8_t line)
{
  if (lcdHandle != NULL)
  {
    // Write string and 32-bit number
    LCD_writeLine(lcdHandle, 0, str, value, format, line);
  }
}

/*********************************************************************
*********************************************************************/
