/*******************************************************************************
  Filename:       osal_snv_wrapper.c
  Revised:        $Date: 2015-02-10 14:18:04 -0800 (Tue, 10 Feb 2015) $
  Revision:       $Revision: 42483 $

  Description:    This module defines the OSAL simple non-volatile memory 
                  functions as a wrapper to On Chip One Page SNV implementation.


  Copyright 2009-2014 Texas Instruments Incorporated. All rights reserved.

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
#if !defined( OSAL_SNV )
// Use 2 page SNV by default
#define OSAL_SNV 2
#endif

// Map 2 and 0 to 2 page SNV.  0 was arbitrarily chosen to go here.
#if OSAL_SNV == 2 || OSAL_SNV == 0
#if OSAL_SNV == 0 && !defined(NO_OSAL_SNV)
#define NO_OSAL_SNV
#endif //OSAL_SNV == 0 && !defined(NO_OSAL_SNV)
#include "osal_snv.c"
#elif OSAL_SNV == 1 // This is the 1 page SNV
#include "osal_snv.h"
#include "./../../../services/nv/cc26xx/nvocop.c"

#ifndef SYSTEM_ID
#define SYSTEM_ID NVINTF_SYSID_NVDRVR
#endif

// Convert a threshold percentage to bytes.
#define THRESHOLD2BYTES(x) ((FLASH_PAGE_SIZE) - (((FLASH_PAGE_SIZE) * (x)) / 100))

/*********************************************************************
 * @fn      osal_snv_init
 *
 * @brief   Initialize NV service.
 *
 * @param   none
 *
 * @return  SUCCESS if initialization succeeds. FAILURE, otherwise.
 */
uint8 osal_snv_init( void )
{  
  return NVOCOP_initNV(NULL);
}

/*********************************************************************
 * @fn      osal_snv_read
 *
 * @brief   Read data from NV.
 *
 * @param   id   - Valid NV item Id.
 * @param   len  - Length of data to read.
 * @param   *pBuf - Data is read into this buffer.
 *
 * @return  SUCCESS if successful.
 *          Otherwise, NV_OPER_FAILED for failure.
 */
uint8 osal_snv_read( osalSnvId_t id, osalSnvLen_t len, void *pBuf)
{
  NVINTF_itemID_t nv_id;
  
  nv_id.itemID = id;
  nv_id.subID = 0;
  nv_id.systemID = SYSTEM_ID;
  
  return NVOCOP_readItem(nv_id, 0, len, pBuf);
}

/*********************************************************************
 * @fn      osal_snv_write
 *
 * @brief   Write a data item to NV.
 *
 * @param   id   - Valid NV item Id.
 * @param   len  - Length of data to write.
 * @param   *pBuf - Data to write.
 *
 * @return  SUCCESS if successful, NV_OPER_FAILED if failed.
 */
uint8 osal_snv_write( osalSnvId_t id, osalSnvLen_t len, void *pBuf)
{
  NVINTF_itemID_t nv_id;
  
  nv_id.itemID = id;
  nv_id.subID = 0;
  nv_id.systemID = SYSTEM_ID;


  return NVOCOP_writeItem(nv_id, len, pBuf);
}

/*********************************************************************
 * @fn      osal_snv_compact
 *
 * @brief   Compacts NV if its usage has reached a specific threshold.
 *
 * @param   threshold - compaction threshold.
 *
 * @return  NV_INTF_SUCCESS if successful,
 *          NV_INTF_FAILURE if failed, or
 *          NV_INTF_BADPARAM if threshold invalid.
 */
uint8 osal_snv_compact( uint8 threshold )
{
  // convert percentage to approximate byte threshold.
  if (threshold <= 100)
  {
    return NVOCOP_compactNV(THRESHOLD2BYTES(threshold));
  }
  
  return NVINTF_BADPARAM;
}

#else // bad OSAL_SNV value
#error "Valid OSAL_SNV values are 0, 1, or 2!"
#endif //OSAL_SNV
