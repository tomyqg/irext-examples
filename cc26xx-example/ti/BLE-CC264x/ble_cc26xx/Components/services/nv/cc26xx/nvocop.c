/*******************************************************************************
  Filename:       nvocop.c
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    This module contains the OSAL simple non-volatile memory 
                  functions.

  Copyright 2009 - 2015 Texas Instruments Incorporated. All rights reserved.

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
*******************************************************************************/

//*****************************************************************************
// Design Overview
//*****************************************************************************
/*
This driver implements a non-volatile (NV) memory system that utilizes 1 page
of on-chip Flash memory. After initialization, this page is ACTIVE
and GPRAM, or Cache RAM, is available for "compaction" when the ACTIVE page does 
not have enough empty space for data write operation. Compaction can occur 'just 
in time' during a data write operation or 'on demand' by application request. The
compaction process IS NOT designed to survive a power cycle before it completes. 
If power is cycled after the page is erased but before all contents have 
been written back to the clean page with the active bit of the page 
header set all data stored in this module will be lost. If a power cycle occurs 
before in compaction before the page is erased, the process is restarted 
during initialization.

Each Flash page has a "page header" which indicates its current state (ERASED,
ACTIVE, or XFER), located at the first byte of the Flash page. The remainder of
the Flash page contains NV data items which are packed together following the
page header. Each NV data item has two parts, (1) a data block which is stored
first (lower memory address), (2) immediately followed by item header (higher
memory address). The item header contains information necessary to traverse the
packed data items, as well as, current status of each data item. A
search for the newest instance of an item is sped up by starting the search at 
the last entry in the page (higher memory address).
*/

/*********************************************************************
 * INCLUDES
 */

#include <string.h>
#include "hal_adc.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "pwrmon.h"
#include "OSAL.h"
#include "osal_snv.h"
#include "hal_assert.h"
#include <driverlib\vims.h>

#include "nvocop.h"

/*********************************************************************
 * CONSTANTS
 */

// Length in bytes of a flash word
#define FLASH_WORD_SIZE            HAL_FLASH_WORD_SIZE

// NV page header size in bytes
#define NV_PAGE_HDR_SIZE           FLASH_WORD_SIZE

// In case pages 0-1 are ever used, define a null page value.
#define NV_PAGE_NULL               0

// In case item Id 0 is ever used, define a null item value.
#define NV_ITEM_NULL               0

// NV page header offset within a page
#define NV_PAGE_HDR_OFFSET         0

// Flag in a length field of an item header to indicate validity
// of the length field
#define NV_INVALID_LEN_MARK        0x8000

// Flag in an ID field of an item header to indicate validity of
// the identifier field
#define NV_INVALID_ID_MARK         0x8000

// Bit difference between active page state indicator value and
// transfer page state indicator value
#define NV_ACTIVE_XFER_DIFF        0x00100000

// active page state indicator value
#define NV_ACTIVE_PAGE_STATE       NV_ACTIVE_XFER_DIFF

// transfer page state indicator value
#define NV_XFER_PAGE_STATE         (NV_ACTIVE_PAGE_STATE ^ NV_ACTIVE_XFER_DIFF)

#define NV_PAGE                    (uint8)((uint32)NV_FLASH >> 12)

// Cache RAM buffer used as a temporary storage buffer during compaction
// This is volatile memory!
#define RAM_BUFFER_ADDRESS         (uint8*)0x11000000
/*********************************************************************
 * MACROS
 */

// Checks whether CC26xx voltage high enough to erase/write NV
#ifdef NV_VOLTAGE_CHECK
#define NV_CHECK_VOLTAGE()  (PWRMON_check(MIN_VDD_FLASH))
#else 
#define NV_CHECK_VOLTAGE()  (TRUE)
#endif //NV_VOLTAGE_CHECK   

/*********************************************************************
 * TYPEDEFS
 */

// NV item header structure
typedef struct
{
  uint16 id;
  uint16 len;
} NvItemHdr_t;
// Note that osalSnvId_t and osalSnvLen_t cannot be bigger than uint16

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

extern uint8* HalFlashGetAddress( uint8 pg, uint16 offset );

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=4096
const uint8 NV_FLASH[FLASH_PAGE_SIZE] @ ".snvSectors";
#elif defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma location = (SNV_FIRST_PAGE << 12);
const uint8 NV_FLASH[FLASH_PAGE_SIZE] = {0x00};
#else
#error "Unknown Compiler! Support for SNV not provided!"
#endif 
 
// active page offset
static uint16 pgOff;

// Flag to indicate that a fatal error occurred while writing to or erasing the
// Flash memory. If flag is set, it's unsafe to attempt another write or erase.
// This flag locks writes to Flash until the next system reset.
static uint8 failF = NVINTF_FAILURE;

// Flag to indicate that a non-fatal error occurred while writing to or erasing
// Flash memory. If flag is set, it's safe to attempt another write or erase.
// This flag is reset by any API calls that cause an erase/write to Flash.
static uint8 failW;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void   setActivePage( void );
static void   setXferPage(void);
static void   erasePage( void );
static void   cleanErasedPage( void );
static uint16 compactPage( void );

static void   writeWord( uint16 offset, uint8 *pBuf, osalSnvLen_t cnt );
static void   writeItem(uint16 offset, osalSnvId_t id, uint16 alignedLen, 
                        uint8 *pBuf );
static uint16 findItemInCache(uint16 offset, osalSnvId_t id);
static uint16 findItem( uint16 offset, osalSnvId_t id );
static void   findOffset( void );

static void   enableCache ( uint8 state );
static uint8  disableCache ( void );

//*****************************************************************************
// API Functions - NV driver
//*****************************************************************************

/*********************************************************************
 * @fn      NV_initNV
 *
 * @brief   Initialize the NV flash pages.
 *
 * @param   param - not used.
 *
 * @return  NVINTF_SUCCESS if initialization succeeds, NVINTF_FAILURE otherwise.
 */
uint8 NVOCOP_initNV( void *param )
{
  failW = failF;
  
#if !defined( NO_OSAL_SNV )
  uint32 pgHdr;
  failF = failW = NVINTF_SUCCESS;

  // Pick active page and clean up erased page if necessary
  HalFlashRead(NV_PAGE, NV_PAGE_HDR_OFFSET, (uint8 *)(&pgHdr), NV_PAGE_HDR_SIZE);

  if ( pgHdr == NV_ACTIVE_PAGE_STATE )
  {
    findOffset();
  }
  else if ( pgHdr == NV_XFER_PAGE_STATE)
  {
    findOffset();
	
    compactPage();
  }
  else
  {
    // Erase this page if it is not erased.
    // This is to ensure that any page that we're in the middle of
    // compacting gets erased.
    erasePage();
		
    setActivePage();
		
    pgOff = NV_PAGE_HDR_SIZE;
  }
  
  return failW;
#else
  
  return NVINTF_FAILURE;
#endif // NO_OSAL_SNV
}

/*********************************************************************
 * @fn      NV_compactNV
 *
 * @brief   Compacts NV if fewer byte are free than minAvail bytes.
 *
 * @param   minAvail - number of free bytes in NV for a compaction to not
 *                     immediately occur.  If set to zero compaction will
 *                     always occur.
 *
 * @return  NVINTF_SUCCESS if successful,
 *          NVINTF_FAILURE if failed
 */
uint8 NVOCOP_compactNV( uint16 minAvail )
{
  uint8 ret = failF;
  
#if !defined( NO_OSAL_SNV )
  uint16 remainder;

  if (ret != NVINTF_SUCCESS)
  {
    return NVINTF_FAILURE;
  }
  
  // Number of bytes left on active page
  remainder = FLASH_PAGE_SIZE - pgOff;

  // Time to do a compaction?
  if( (remainder < minAvail) || (minAvail == 0) )
  {
    // Change the ACTIVE page to XFER mode
    setXferPage();

    // Reset failW
    failW = NVINTF_SUCCESS;
    
    // Compact page.
    remainder = compactPage();

    if (remainder == 0)
    {
      ret = (failW == NVINTF_SUCCESS) ? NVINTF_FAILURE : failW;
    }
  }
#endif // !NO_OSAL_SNV

  return ret;
}

//*****************************************************************************
// API Functions - NV Data Items
//*****************************************************************************

/*********************************************************************
 * @fn      NV_readItem
 *
 * @brief   Read data from NV.
 *
 * @param   nv_id  - Valid NV item Id.
 * @param   len    - Length of data to read.
 * @param   *pBuf  - Data is read into this buffer.
 *
 * @return  NVINTF_SUCCESS if successful.
 *          Otherwise, NVINTF_FAILURE for failure.
 */
uint8 NVOCOP_readItem( NVINTF_itemID_t nv_id, uint16 offset, uint16 len, void *pBuf )
{
#if !defined( NO_OSAL_SNV )
  osalSnvId_t id = nv_id.itemID;
  
  uint16 itemOffset = findItem(pgOff, id);

  if (itemOffset != 0)
  {
    HalFlashRead(NV_PAGE, itemOffset, pBuf, len);

    return NVINTF_SUCCESS;
  }
#endif // !NO_OSAL_SNV

  return NVINTF_FAILURE;
}

/*********************************************************************
 * @fn      NV_writeItem
 *
 * @brief   Write a data item to NV.
 *
 * @param   nv_id - Valid NV item Id.
 * @param   len   - Length of data to write.
 * @param   *pBuf - Data to write.
 *
 * @return  NVINTF_SUCCESS if successful, failure code otherwise.
 */
uint8 NVOCOP_writeItem( NVINTF_itemID_t nv_id, uint16 len, void *pBuf )
{ 
#if !defined( NO_OSAL_SNV )
  uint16 alignedLen;
  osalSnvId_t id = nv_id.itemID;

  // Reset failW
  failW = NVINTF_SUCCESS;
  
  {
    uint16 offset = findItem(pgOff, id);

    if (offset > 0)
    {
      uint8 tmp;
      osalSnvLen_t i;

      for (i = 0; i < len; i++)
      {
        HalFlashRead(NV_PAGE, offset, &tmp, 1);
        if (tmp != ((uint8 *)pBuf)[i])
        {
          break;
        }
        offset++;
      }

      if (i == len)
      {
        // Changed value is the same value as before.
        // Return here instead of re-writing the same value to NV.
        return NVINTF_SUCCESS;
      }
    }
  }

  alignedLen = ((len + FLASH_WORD_SIZE - 1) / FLASH_WORD_SIZE) * FLASH_WORD_SIZE;

  if ( pgOff + alignedLen + FLASH_WORD_SIZE > FLASH_PAGE_SIZE )
  {
    setXferPage();
    compactPage();
  }

  // pBuf shall be referenced beyond its valid length to save code size.
  writeItem(pgOff, id, alignedLen, pBuf);

  // Check if failed
  if (failW != NVINTF_SUCCESS)
  {
    return failW;
  }

  pgOff += alignedLen + FLASH_WORD_SIZE;
  
  return NVINTF_SUCCESS;
#else
  
  return NVINTF_FAILURE;
#endif // !NO_OSAL_SNV
}

//*****************************************************************************
// Local NV Driver Utility Functions
//*****************************************************************************

/*********************************************************************
 * @fn      setActivePage
 *
 * @brief   Set page header active state to be active.
 *
 * @param   none
 *
 * @return  none
 */
static void setActivePage(void)
{
  uint32 pgHdr;

  pgHdr = NV_ACTIVE_PAGE_STATE;

  writeWord( NV_PAGE_HDR_OFFSET, (uint8*) &pgHdr, 1);
}

/*********************************************************************
 * @fn      setXferPage
 *
 * @brief   Set active page header state to be transfer state.
 *
 * @param   none
 *
 * @return  none
 */
static void setXferPage(void)
{
  uint32 pgHdr;

  // erase difference bit between active state and xfer state
  pgHdr = NV_XFER_PAGE_STATE;

  writeWord( NV_PAGE_HDR_OFFSET, (uint8*)&pgHdr, 1 );
}

/*********************************************************************
 * @fn      erasePage
 *
 * @brief   Erases a page in Flash.
 *
 * @param   none
 *
 * @return  none
 */
static void erasePage( void )
{
  if (!NV_CHECK_VOLTAGE())
  {
    // Power monitor indicates low voltage
    failW = NVINTF_FAILURE;
    return;
  }
  else
  {
    halIntState_t cs;
    uint8 state;
    uint32_t err;
    
    HAL_ENTER_CRITICAL_SECTION(cs);
    
    // Disable the cache.
    state = disableCache();

    // Erase the page.
    err = FlashSectorErase( (uint32)HalFlashGetAddress(NV_PAGE, 0));

    // Enable the cache.
    enableCache(state);

      // Page erase failed, further usage is unsafe.
    if (err != FAPI_STATUS_SUCCESS)
    {
      failF = failW = NVINTF_FAILURE;
    }
  
    HAL_EXIT_CRITICAL_SECTION(cs);
  }
}

/*********************************************************************
 * @fn      findOffset
 *
 * @brief   find an offset of an empty space in active page
 *          where to write a new item to.
 *
 * @param   None
 *
 * @return  none
 */
static void findOffset(void)
{
  uint16 offset;
  for (offset = FLASH_PAGE_SIZE - FLASH_WORD_SIZE;
       offset >= NV_PAGE_HDR_SIZE;
       offset -= FLASH_WORD_SIZE)
  {
    uint32 tmp;

    HalFlashRead(NV_PAGE, offset, (uint8 *)&tmp, FLASH_WORD_SIZE);
    if (tmp != 0xFFFFFFFF)
    {
      break;
    }
  }
  pgOff = offset + FLASH_WORD_SIZE;
}

/*********************************************************************
 * @fn      findItem
 *
 * @brief   find a valid item from a designated page and offset
 *
 * @param   offset - offset in the NV page from where to start search up.
 *                   Usually this parameter is set to the empty space offset.
 * @param   id     - NV item ID to search for
 *
 * @return  offset of the item, 0 when not found
 */
static uint16 findItem(uint16 offset, osalSnvId_t id)
{
  offset -= FLASH_WORD_SIZE;

  while (offset >= NV_PAGE_HDR_SIZE)
  {
    NvItemHdr_t hdr;

    HalFlashRead(NV_PAGE, offset, (uint8 *) &hdr, FLASH_WORD_SIZE);

    if (hdr.id == id)
    {
      // item found
      // length field could be corrupt. Mask invalid length mark.
      uint8 len = hdr.len & ~NV_INVALID_LEN_MARK;
      return offset - len;
    }
    else if (hdr.len & NV_INVALID_LEN_MARK)
    {
      offset -= FLASH_WORD_SIZE;
    }
    else
    {
      // valid length field
      if (hdr.len + FLASH_WORD_SIZE <= offset)
      {
        // valid length
        offset -= hdr.len + FLASH_WORD_SIZE;
      }
      else
      {
        // active page is corrupt
        // This could happen if NV initialization failed upon failure to erase
        // page and active page is set to uncleanly erased page.
        HAL_ASSERT_FORCED();
        return 0;
      }
    }
  }
  return 0;
}

/*********************************************************************
 * @fn      findItemInCache
 *
 * @brief   find a valid item from cache
 *
 * @param   ramBuffer - Points buffer to the start of cache ram 0x11000000.
 * @param   offset    - offset in cache.
 * @param   id        - NV item ID to search for.
 *
 * @return  offset of the item, 0 when not found
 */
static uint16 findItemInCache(uint16 offset, osalSnvId_t id)
{
  offset -= FLASH_WORD_SIZE;

  while (offset >= NV_PAGE_HDR_SIZE  && offset < FLASH_PAGE_SIZE)
  {
    NvItemHdr_t hdr;
		
    // Read header
    memcpy( &hdr, RAM_BUFFER_ADDRESS + offset, FLASH_WORD_SIZE);
    
    //If you find id return true else update offset and search again
    if ( hdr.id == id )
    {
      return 1;
    }
    else
    {
      offset -= hdr.len + FLASH_WORD_SIZE;  
    }
  }

  return 0;
}

/*********************************************************************
 * @fn      writeItem
 *
 * @brief   Write a data item to NV. Function can write an entire item to NV
 *
 * @param   offset     - offset within the NV page where to write the new item
 * @param   id         - NV item ID
 * @param   alignedLen - Length of data to write, alinged in flash word
 *                       boundary
 * @param   pBuf - Data to write.
 *
 * @return  none
 */
static void writeItem(uint16 offset, osalSnvId_t id, uint16 alignedLen, uint8 *pBuf )
{
  NvItemHdr_t hdr;

  hdr.id = 0xFFFF;
  hdr.len = alignedLen | NV_INVALID_LEN_MARK;

  // Write the len portion of the header first
  writeWord(offset + alignedLen, (uint8 *) &hdr, 1);

  // remove invalid len mark
  hdr.len &= ~NV_INVALID_LEN_MARK;
  writeWord(offset + alignedLen, (uint8 *) &hdr, 1);

  // Copy over the data
  writeWord(offset, pBuf, alignedLen / FLASH_WORD_SIZE);

  // value is valid. Write header except for the most significant bit.
  hdr.id = id | NV_INVALID_ID_MARK;
  writeWord(offset + alignedLen, (uint8 *) &hdr, 1);

  // write the most significant bit
  hdr.id &= ~NV_INVALID_ID_MARK;
  writeWord(offset + alignedLen, (uint8 *) &hdr, 1);
}

/*********************************************************************
 * @fn      compactPage
 *
 * @brief   Compacts the page specified.
 *
 * @param   none
 *
 * @return  remaining unused bytes on the flash page.
 */
static uint16 compactPage()
{
  uint8 state;
  uint16 srcOff, dstOff;
  osalSnvId_t lastId = (osalSnvId_t) 0xFFFF;

  // Start writing to base of RAM buffer.
  dstOff = 0; 

  // Disable cache to use as a temporary storage buffer during compaction.
  state = disableCache();
	
  // Read from the latest value
  srcOff = pgOff - sizeof(NvItemHdr_t);

  while (srcOff >= NV_PAGE_HDR_SIZE)
  {
    NvItemHdr_t hdr;

    HalFlashRead(NV_PAGE, srcOff, (uint8 *) &hdr, FLASH_WORD_SIZE);

    if (hdr.id == 0xFFFF || hdr.id == NV_ITEM_NULL)
    {
      // Invalid entry. Skip this one.
      if (hdr.len & NV_INVALID_LEN_MARK)
      {
        srcOff -= FLASH_WORD_SIZE;
      }
      else
      {
        if (hdr.len + FLASH_WORD_SIZE <= srcOff)
        {
          srcOff -= hdr.len + FLASH_WORD_SIZE;
        }
        else
        {
          // invalid length. Source page must be a corrupt page.
          // This is possible only if the NV initialization failed upon erasing
          // what is selected as active page.
          // This is supposed to be a very rare case, as power should be
          // shutdown exactly during erase and then the page header is
          // still retained as either the Xfer or the Active state.

          // For production code, it might be useful to attempt to erase the page
          // so that at next power cycle at least the device is runnable
          // (with all entries removed).
          // However, it might be still better not to attempt erasing the page
          // just to see if this very rare case actually happened.
          //erasePage(srcPg);

          HAL_ASSERT_FORCED();
          
          // Enable cache use
          enableCache(state);
          
          return 0;
        }
      }

      continue;
    }

    // Consider only valid item
    if (!(hdr.id & NV_INVALID_ID_MARK) && hdr.id != lastId)
    {
      // lastId is used to speed up compacting in case the same item ID
      // items were neighboring each other contiguously.
      lastId = (osalSnvId_t) hdr.id;
      
      // Check if the latest value of the item was already written
      if (findItemInCache(dstOff, lastId) == 0)
      {
        // This item was not copied over yet, This must be the latest value.
        // Transfer item from flash active page to cache memory after while loop completes a 
        // compacted version of the flash memory will exist in cache.  
        HalFlashRead(NV_PAGE, srcOff - hdr.len, RAM_BUFFER_ADDRESS + dstOff, 
                     hdr.len + FLASH_WORD_SIZE);

        dstOff += hdr.len + FLASH_WORD_SIZE;
      }
    }
    srcOff -= hdr.len + FLASH_WORD_SIZE;
  }
  
  // Erase the currently active page
  erasePage();
  
  // Write items back from cache to srcPg
  if (dstOff > 0)
  {
    if (NV_CHECK_VOLTAGE())
    {
      HalFlashWrite((uint32)NV_FLASH + NV_PAGE_HDR_SIZE, RAM_BUFFER_ADDRESS, 
                    dstOff);
      
      // Set srcPg as the active page
      setActivePage();
    }
    else
    {
      failW = NVINTF_FAILURE;
    }
  }

  // Enable cache use
  enableCache(state);
	
  if (!failW)
  {
    pgOff = dstOff + NV_PAGE_HDR_SIZE; // update active page offset
    
    return ( FLASH_PAGE_SIZE - dstOff );
  }
  else
  {
    pgOff = 0;
    
    return 0;
  }
}

/*********************************************************************
 * @fn      verifyWordM
 *
 * @brief   verify the written word.
 *
 * @param   offset - A valid offset into the page.
 * @param   pBuf   - Pointer to source buffer.
 * @param   cnt    - Number of 4-byte blocks to verify.
 *
 * @return  none
 */
static void verifyWordM(uint16 offset, uint8 *pBuf, osalSnvLen_t cnt )
{
  uint8 tmp[FLASH_WORD_SIZE];

  while (cnt--)
  {
    // Reading byte per byte will reduce code size but will slow down
    // and not sure it will meet the timing requirements.
    HalFlashRead(NV_PAGE, offset, tmp, FLASH_WORD_SIZE);
    if (FALSE == osal_memcmp(tmp, pBuf, FLASH_WORD_SIZE))
    {
      failF = failW = NVINTF_FAILURE;
      return;
    }
    offset += FLASH_WORD_SIZE;
    pBuf += FLASH_WORD_SIZE;
  }
}

/*********************************************************************
 * @fn      writeWord
 *
 * @brief   Writes a Flash-WORD to NV.
 *
 * @param   offset - A valid offset into the page.
 * @param   pBuf   - Pointer to source buffer.
 * @param   cnt    - Number of words to write.
 *
 * @return  none
 */
static void writeWord( uint16 offset, uint8 *pBuf, osalSnvLen_t cnt )
{
  uint32 addr = (uint32) HalFlashGetAddress(NV_PAGE, offset);
  
  if (NV_CHECK_VOLTAGE())
  {
    // Enter Critical Section
    halIntState_t cs;
    HAL_ENTER_CRITICAL_SECTION(cs);
    
    // Disable Cache
    uint8 state;
    state = disableCache();

    // Write data.
    HalFlashWrite(addr, pBuf, cnt * (FLASH_WORD_SIZE) );

    // Enable cache.
    enableCache(state);

    verifyWordM(offset, pBuf, cnt);

    HAL_EXIT_CRITICAL_SECTION(cs);
  }
  else
  {
    failW = NVINTF_FAILURE;
  }
}

/*********************************************************************
 * @fn      enableCache
 *
 * @brief   enable cache.
 *
 * @param   state - the VIMS state returned from disableCache.
 *
 * @return  none.
 */
static void enableCache ( uint8 state )
{
  if ( state != VIMS_MODE_DISABLED )
  {
    // Enable the Cache.
    VIMSModeSet( VIMS_BASE, VIMS_MODE_ENABLED );
  }
}

/*********************************************************************
 * @fn      disableCache
 *
 * @brief   invalidate and disable cache.
 *
 * @param   none
 *
 * @return  VIMS state
 */
static uint8 disableCache ( void )
{
  uint8 state = VIMSModeGet( VIMS_BASE );
  
  // Check VIMS state
  if ( state != VIMS_MODE_DISABLED )
  {
    // Invalidate cache
    VIMSModeSet( VIMS_BASE, VIMS_MODE_DISABLED );
  
    // Wait for disabling to be complete
    while ( VIMSModeGet( VIMS_BASE ) != VIMS_MODE_DISABLED ); 
    
  }
  
  return state;
}

/*********************************************************************
*********************************************************************/

 
