/**************************************************************************************************
  @headerfile:    sbl.c
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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#include <xdc/std.h>
#include <stdbool.h>

#include "sbl.h"
#include "sbl_image.h"
#include "sbl_cmd.h"
#include "sbl_tl.h"

#include <ti/drivers/pin/PINCC26XX.h>

#include "Board.h"

/*********************************************************************
 * CONSTANTS
 */

// Indexes for pin configurations in PIN_Config array
#define RST_PIN_IDX             0       
#define BL_PIN_IDX              1

// Imperically found delay count for 1 ms
#define SBL_DELAY_1_MS          4800

//! \brief Default SBL parameters
const SBL_Params SBL_defaultParams = {
    .targetInterface            = SBL_DEV_INTERFACE_UART,
    .localInterfaceID           = CC2650_UART0,        
    .resetPinID                 = Board_DP0, //SensorTag
    .blPinID                    = Board_DP2  //SensorTag
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

//! \brief PIN Config for reset and bl signals without PIN IDs 
static PIN_Config sblPinsCfg[] =
{
    PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static uint32_t rstPIN = (IOID_UNUSED & IOC_IOID_MASK);
static uint32_t blPIN = (IOID_UNUSED & IOC_IOID_MASK);

//! \brief PIN State for reset and boot loader pins
static PIN_State sblPins;

//! \brief PIN Handles for reset and boot loader pins
static PIN_Handle hsblPins = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
     
static void SBL_utilDelay(uint32_t ms);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @fn      SBL_initParams
 *
 * @brief   Initializes SBL parameters 
 *
 * @param   params - SBL parameter structure to be set to SBL default values
 *
 * @return  None.
 */
void SBL_initParams(SBL_Params *params)
{
  *params = SBL_defaultParams;
}

/**
 * @fn      SBL_open
 *
 * @brief   Opens the SBL port for writing images
 *
 * @param   params - SBL parameters to initialize the port with
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_open(SBL_Params *params)
{
  // Assign PIN IDs to reset and bl
  rstPIN = (params->resetPinID & IOC_IOID_MASK);
  blPIN = (params->blPinID & IOC_IOID_MASK);
  
  // Add PIN IDs to PIN Configuration
  sblPinsCfg[RST_PIN_IDX] |= rstPIN;
  sblPinsCfg[BL_PIN_IDX] |= blPIN;
  
  // Initialize SBL Pins
  hsblPins = PIN_open(&sblPins, sblPinsCfg);
  if (hsblPins == NULL)
  {
    return SBL_FAILURE;
  }
  
  // Open SBL Transport Layer
  return SBL_TL_open(params->targetInterface, params->localInterfaceID);
}

/**
 * @fn      SBL_writeImage
 *
 * @brief   Writes image to the target device using SBL and resets device
 *
 * @param   image - parameters that describe the image to be written to target
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_writeImage(SBL_Image *image)
{
  uint32_t imgSize = 0;
  uint32_t bytesWritten = 0;
  uint32_t transLen = 0;
  uint32_t imageAddr = 0;
  uint8_t imgBuf[SBL_MAX_TRANSFER];

  if (SBL_IMG_open() == SBL_SUCCESS)
  {
    // Verify the image is valid
    if (SBL_IMG_isValid(image->imgInfoLocAddr))
    {
      // Get image size
      imgSize = SBL_IMG_getSize(image->imgInfoLocAddr);
      
      // Align size on 4B boundary
      if (imgSize &  0x03)
      {
        imgSize += (4 - (imgSize % 4));
      }
      
      // Image size must be non-zero
      if (imgSize)
      {
        // Erase Flash
        SBL_eraseFlash(image->imgTargetAddr, imgSize);
        
        // Send Download command to target
        SBL_CMD_download(image->imgTargetAddr, imgSize);
        
        // Verify Device Status after download command
        if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
        {
          SBL_IMG_close();

          return SBL_FAILURE;
        }
        
        // Update addr to point to image instead of image header
        imageAddr = image->imgLocAddr;
        
        // Write image to target device
        while(bytesWritten < imgSize)
        {
          // Determine number of bytes in this transfer
          transLen = (SBL_MAX_TRANSFER < (imgSize - bytesWritten))? 
                             SBL_MAX_TRANSFER : (imgSize - bytesWritten);
                             
          // Read bytes of image
          SBL_IMG_read(imageAddr, imgBuf, transLen);                   
          
          // Send data to target
          SBL_CMD_sendData(imgBuf, transLen);

          // Verify Device Status after download command
          if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
          {
            SBL_IMG_close();

            return SBL_FAILURE;
          }
        
          // Update bytes written and image address of next read
          bytesWritten += transLen;
          imageAddr += transLen;
        }
        
        // Write complete
        SBL_IMG_close();
        
        // Send Rest Command
        SBL_CMD_reset();

        return SBL_SUCCESS;
      }
    }

    SBL_IMG_close();
  }
  
  return SBL_FAILURE;
}

/**
 * @fn      SBL_eraseFlash
 *
 * @brief   Erases specific flash sectors of target device
 *
 * @param   addr - address of image to be erased
 * @param   size - total amount of bytes to erase starting from addr
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_eraseFlash(uint32_t addr, uint32_t size)
{
  uint16_t i;
  uint16_t numPages = 0;
  uint32_t numBytes = size;
  uint32_t firstPageAddr = addr;

  
  // Calculate number of flash pages that need to be erased
  if (addr % SBL_PAGE_SIZE)
  {
    // Starting address is not page aligned, add page to erase total and 
    // remove bytes in the starting page from total
    numPages++;
    numBytes -= (SBL_PAGE_SIZE - (addr % SBL_PAGE_SIZE));
    
    // Reset first page address to beginning of page that contains addr
    firstPageAddr -= (addr % SBL_PAGE_SIZE);
  }
  
  // Calculate pages from remaining byte total
  numPages += (numBytes / SBL_PAGE_SIZE);
  numPages += (numBytes % SBL_PAGE_SIZE) ? 1 : 0;
  
  for(i = 0; i < numPages; i++)
  {
    // Erase page
    SBL_CMD_sectorErase(firstPageAddr);
    
    // Verify Device Status after sector erase command
    if (SBL_CMD_getStatus() != SBL_CMD_RET_SUCCESS)
    {
      return SBL_FAILURE;
    }
    
    // Update page address to next page
    firstPageAddr += SBL_PAGE_SIZE;
  }
  
  return SBL_SUCCESS;
}

/**
 * @fn      SBL_close
 *
 * @brief   Closes SBL port
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_close(void)
{
  // Clear SBL PIN IDs
  rstPIN = (IOID_UNUSED & IOC_IOID_MASK); // Set to 0x000000FF
  blPIN = (IOID_UNUSED & IOC_IOID_MASK); // Set to 0x000000FF
  
  // Clear PIN IDs from PIN Configuration
  sblPinsCfg[RST_PIN_IDX] &= ~rstPIN; 
  sblPinsCfg[BL_PIN_IDX] &= ~blPIN;
  
  // Close PIN Handle
  PIN_close(hsblPins);
  hsblPins = NULL;
  
  // Close SBL Transport Layer
  SBL_TL_close();
  
  return SBL_SUCCESS;
}

/**
 * @fn      SBL_openTarget
 *
 * @brief   Forces target device into SBL
 *
 * @param   None.
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_openTarget(void)
{
  // Set BL PIN low and then set Reset PIN low to enter SBL
  PIN_setOutputValue(hsblPins, blPIN, 0);
  PIN_setOutputValue(hsblPins, rstPIN, 0);
    
  SBL_utilDelay(15);
  
  // Release Reset PIN while keeping BL pin low
  PIN_setOutputValue(hsblPins, rstPIN, 1);
  
  // Delay to be tuned
  SBL_utilDelay(150); 
  
  // Release BL Pin now that target should be in SBL mode
  PIN_setOutputValue(hsblPins, blPIN, 1);
  
  // Send initial packet so target can detect baud rate
  if (SBL_TL_uartAutoBaud() == SBL_DEV_ACK)
  {
    return SBL_SUCCESS;
  }
  
  return SBL_FAILURE;
}

/**
 * @fn      SBL_resetTarget
 *
 * @brief   Forces target device to boot from flash image instead of SBL. This
 *          function can be called before SBL_open() or after SBL_open()
 *
 * @param   rstPinID - Board Pin ID of reset PIN
 * @param   blPinID  - Board Pin ID of boot loader PIN
 *
 * @return  uint8_t - SBL_SUCCESS, SBL_FAILURE
 */
uint8_t SBL_resetTarget(uint32_t rstPinID, uint32_t blPinID)
{
  uint8_t openedPins = 0;
  
  if (hsblPins == NULL)
  {
    // Must open pins if SBL_open() has not yet been called
    openedPins = 1;
    
    // Assign PIN IDs to reset and bl
    rstPIN = (rstPinID & IOC_IOID_MASK);
    blPIN = (blPinID & IOC_IOID_MASK);
    
    // Add PIN IDs to PIN Configuration
    sblPinsCfg[RST_PIN_IDX] |= rstPIN;
    sblPinsCfg[BL_PIN_IDX] |= blPIN;
    
    // Initialize SBL Pins
    hsblPins = PIN_open(&sblPins, sblPinsCfg);
    if (hsblPins == NULL)
    {
      return SBL_FAILURE;
    }
  }
  
  // Guarantee that Boot Loader Pin is high during reset toggle
  PIN_setOutputValue(hsblPins, blPIN, 1);
  
  // Set reset PIN low
  PIN_setOutputValue(hsblPins, rstPIN, 0);
  
  SBL_utilDelay(15);
  
  // Release Reset PIN 
  PIN_setOutputValue(hsblPins, rstPIN, 1);

  // Must close Pins if opened in function
  if (openedPins)
  {
    // Clear SBL PIN IDs
    rstPIN = (IOID_UNUSED & IOC_IOID_MASK); // Set to 0x000000FF
    blPIN = (IOID_UNUSED & IOC_IOID_MASK); // Set to 0x000000FF
    
    // Clear PIN IDs from PIN Configuration
    sblPinsCfg[RST_PIN_IDX] &= ~rstPIN; 
    sblPinsCfg[BL_PIN_IDX] &= ~blPIN;
    
    // Close PIN Handle
    PIN_close(hsblPins);
    hsblPins = NULL;
  }
  
  return SBL_SUCCESS;
}

/**
 * @fn      SBL_utilDelay
 *
 * @brief   Simple for loop to burn cycles while target device is resetting
 *
 * @param   ms - Milliseconds to delay
 *
 * @return  None.
 */
void SBL_utilDelay(uint32_t ms)
{
  volatile uint32_t i,j; // Force compiler to not optimize loops
  
  for (i = 0; i < ms; i++)
  {
    // Delay one millisecond
    for (j = SBL_DELAY_1_MS; j > 0; j--);
  }
}
