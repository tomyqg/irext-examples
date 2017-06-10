/*******************************************************************************
  Filename:       R2F_FlashJT.h
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    This file contains the defines for every flash function call
                  using the ROM-to-Flash Flash Jump Table.

  Copyright 2014-2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef R2F_FLASH_JT_H
#define R2F_FLASH_JT_H

#if defined( ROM_BUILD )

/*******************************************************************************
 * EXTERNS
 */

// ROM's RAM table for pointers to ICall functions and flash jump tables.
// Note: This linker imported symbol is treated as a variable by the compiler.
// 0: iCall Dispatch Function Pointer
// 1: iCall Enter Critical Section Function Pointer
// 2: iCall Leave Critical Section Function Pointer
// 3: R2F Flash Jump Table Pointer
// 4: R2R Flash Jump Table Pointer
extern uint32 RAM_BASE_ADDR[];

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "ll_config.h"

/*******************************************************************************
 * CONSTANTS
 */

// ROM's RAM table offset to R2F flash jump table pointer.
#define ROM_RAM_TABLE_R2F          3

// Defines used for the flash jump table routines that are not part of build.
// Note: Any change to this table must accompany a change to R2F_Flash_JT[]!
#define R2F_JT_LOCATION            (&RAM_BASE_ADDR[ROM_RAM_TABLE_R2F])

#define R2F_JT_BASE                (*((uint32 **)R2F_JT_LOCATION))
#define R2F_JT_OFFSET(index)       (*(R2F_JT_BASE+(index)))

// ROM-to-Flash Functions
#define osal_mem_alloc             ((void   *(*) (uint16))                       R2F_JT_OFFSET(0))
#define osal_mem_free              ((void    (*) (void *))                       R2F_JT_OFFSET(1))
#define osal_pwrmgr_task_state     ((uint8   (*) (uint8, uint8))                 R2F_JT_OFFSET(2))
#define osal_msg_allocate          ((uint8  *(*) (uint16))                       R2F_JT_OFFSET(3))
#define osal_msg_send              ((uint8   (*) (uint8, uint8 *))               R2F_JT_OFFSET(4))
#define osal_set_event             ((uint8   (*) (uint8, uint16))                R2F_JT_OFFSET(5))
#define osal_memcpy                ((void   *(*) (void *, const void *, uint32)) R2F_JT_OFFSET(6))
#define osal_memset                ((void   *(*) (void *, uint8, int))           R2F_JT_OFFSET(7))
#define osal_bm_alloc              ((void   *(*) (uint16))                       R2F_JT_OFFSET(8))
#define osal_bm_free               ((void    (*) (void *))                       R2F_JT_OFFSET(9))
#define osal_bm_adjust_header      ((void   *(*) (void *, int16))                R2F_JT_OFFSET(10))
#define osal_start_timerEx         ((uint8   (*) (uint8, uint16, uint32))        R2F_JT_OFFSET(11))
#define osal_stop_timerEx          ((uint8   (*) (uint8, uint16))                R2F_JT_OFFSET(12))
#define osal_clear_event           ((uint8   (*) (uint8, uint16))                R2F_JT_OFFSET(13))
#define Onboard_soft_reset         ((void    (*) (void))                         R2F_JT_OFFSET(14))
#define IntMasterEnable            ((uint8   (*) (void))                         R2F_JT_OFFSET(15))
#define IntMasterDisable           ((uint8   (*) (void))                         R2F_JT_OFFSET(16))
#define IntEnable                  ((void    (*) (uint32))                       R2F_JT_OFFSET(17))
#define IntDisable                 ((void    (*) (uint32))                       R2F_JT_OFFSET(18))
#define halAssertHandler           ((void    (*) (void))                         R2F_JT_OFFSET(19))
#define HalTRNG_InitTRNG           ((void    (*) (void))                         R2F_JT_OFFSET(20))
#define HalTRNG_GetTRNG            ((uint32  (*) (void))                         R2F_JT_OFFSET(21))
#define LL_PM_Init                 ((void    (*) (void))                         R2F_JT_OFFSET(22))
#define LL_PM_GetRfCoreState       ((uint8   (*) (void))                         R2F_JT_OFFSET(23))
#define LL_PM_StartRfTask          ((void    (*) (taskInfo_t *))                 R2F_JT_OFFSET(24))
#define LL_PM_PowerOnReq           ((void    (*) (void))                         R2F_JT_OFFSET(25))
#define LL_PM_PowerCycleRadio      ((void    (*) (void))                         R2F_JT_OFFSET(26))
#define LL_PM_ForceSysBusThroughRF ((void    (*) (void))                         R2F_JT_OFFSET(27))
#define LL_PM_Enter_AES            ((void    (*) (void))                         R2F_JT_OFFSET(28))
#define LL_PM_Exit_AES             ((void    (*) (void))                         R2F_JT_OFFSET(29))
#define LL_PM_PowerOnRfCore        ((void    (*) (void))                         R2F_JT_OFFSET(30))
#define LL_PM_PowerOffRfCore       ((void    (*) (void))                         R2F_JT_OFFSET(31))
#define LL_PM_StopCurTaskTimer     ((void    (*) (taskInfo_t *))                 R2F_JT_OFFSET(32))
// ROM-to-RAM Data
#define hciTaskID                  (*(uint8 *)                                   R2F_JT_OFFSET(33))
#define hciL2capTaskID             (*(uint8 *)                                   R2F_JT_OFFSET(34))
#define hciGapTaskID               (*(uint8 *)                                   R2F_JT_OFFSET(35))
#define hciSmpTaskID               (*(uint8 *)                                   R2F_JT_OFFSET(36))
#define hciTestTaskID              (*(uint8 *)                                   R2F_JT_OFFSET(37))
#define llConfigTable              (*(llCfgTable_t *)                            R2F_JT_OFFSET(38))

#endif // ROM_BUILD

#endif /* R2F_FLASH_JT_H */



