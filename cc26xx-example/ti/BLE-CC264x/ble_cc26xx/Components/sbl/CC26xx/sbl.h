/**************************************************************************************************
  Filename:       sbl.h
  Revised:        $Date: 2015-07-23 11:48:29 -0700 (Thu, 23 Jul 2015) $
  Revision:       $Revision: 44402 $

  Description:    This file contains top level SBL API for CC26xx

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

#ifndef SBL_H
#define SBL_H

#ifdef __cplusplus
extern "C"
{
#endif
  
/*********************************************************************
 * INCLUDES
 */
  
#include <stdint.h>
  
/*********************************************************************
*  EXTERNAL VARIABLES
*/

/*********************************************************************
 * CONSTANTS
 */
  
//! \brief External Device SBL default interface
#define SBL_DEV_INTERFACE_UART          0 //!< External device SBL set for UART
#define SBL_DEV_INTERFACE_SPI           1 //!< External device SBL set for SPI
#define SBL_DEV_INTERFACE_UNDEFINED     9 //!< External device SBL undefined
  
//! \brief SBL Image location types
#define SBL_IMAGE_TYPE_INT              0 //!< Image located in internal flash
#define SBL_IMAGE_TYPE_EXT              1 //!< Image located in external flash
  
//! \brief Response codes for SBL
#define SBL_SUCCESS                     0
#define SBL_DEV_ACK                     1
#define SBL_DEV_NACK                    2
#define SBL_FAILURE                     3
  
//! \brief Target and Device specific defines
#define SBL_MAX_TRANSFER                252
#define SBL_PAGE_SIZE                   4096
  
/*********************************************************************
 * TYPEDEFS
 */

typedef struct 
{
  uint8_t       targetInterface;          //!< SBL_DEV_INTERFACE_[UART,SPI]
  uint8_t       localInterfaceID;         //!< Device Interface (i.e. CC2650_UART0, CC2650_SPI0, etc.)
  uint32_t      resetPinID;               //!< Local PIN ID connected to target RST PIN
  uint32_t      blPinID;                  //!< Local PIN ID connected to target BL PIN
} SBL_Params;
  
typedef struct 
{
  uint8_t       imgType;                //!< SBL_IMAGE_TYPE_[INT,EXT]
  uint32_t      imgInfoLocAddr;         //!< Address of image info header
  uint32_t      imgLocAddr;             //!< Local address of image to use for SBL
  uint32_t      imgTargetAddr;          //!< Target address to write image
} SBL_Image;

/*********************************************************************
 * FUNCTIONS
 */

extern void SBL_initParams(SBL_Params *params);

extern uint8_t SBL_open(SBL_Params *params);

extern uint8_t SBL_openTarget(void);

extern uint8_t SBL_resetTarget(uint32_t rstPinID, uint32_t blPinID);

extern uint8_t SBL_eraseFlash(uint32_t addr, uint32_t size);

extern uint8_t SBL_writeImage(SBL_Image *image);

extern uint8_t SBL_close(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SBL_H */
