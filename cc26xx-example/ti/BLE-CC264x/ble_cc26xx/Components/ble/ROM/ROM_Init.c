/*******************************************************************************
  Filename:       ROM_Init.c
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    This file contains the entry point for the BLE ROM.

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

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include <inc/hw_types.h>
#include "ROM_Init.h"
//
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Memory.h"
#include "OSAL_PwrMgr.h"
#include "osal_bufmgr.h"
#include <driverlib/vims.h>
#include <driverlib/interrupt.h>
#include "hal_assert.h"
#include "hci_tl.h"
#include "hal_trng_wrapper.h"
#include "rfHal.h"
#include "ll_config.h"
#include <inc/hw_sysctl.h>
#include <inc/hw_ioc.h>
#include "mb.h"
#include "ll_sleep.h"
#include "ICall.h"

/*******************************************************************************
 * EXTERNS
 */

// RAM address containing a pointer to the R2F flash jump table base address.
// Note: This linker imported symbol is treated as a variable by the compiler.

// ROM base address where the ROM's C runtime routine is expected.
extern uint32 ROM_BASE_ADDR;

// RAM base address of a table a pointers that are used by ROM and which are
// initialized by ROM_Init.
extern uint32 RAM_BASE_ADDR;

// Checksum start/end address and value for ROM and Common ROM.
extern const uint32 __checksum_begin;
extern const uint32 __checksum_end;

// Function pointers used by the Flash software (provided by RTOS).
extern ICall_Dispatcher ICall_dispatcher;
extern ICall_EnterCS    ICall_enterCriticalSection;
extern ICall_LeaveCS    ICall_leaveCriticalSection;

// calculate checksum routine
extern uint16 slow_crc16( uint16 crc, uint8 *pAddr, uint32 len );
extern uint8  validChecksum( const uint32 *beginAddr, const uint32 *endAddr );

/*******************************************************************************
 * PROTOTYPES
 */

void ROM_Spinlock( void );

/*******************************************************************************
 * MACROS
 */

// ICall function pointers and R2F/R2R flash JT pointers for ROM
#define icallRomDispatchPtr    (uint32 *)(&RAM_BASE_ADDR+0)
#define icallRomEnterCSPtr     (uint32 *)(&RAM_BASE_ADDR+1)
#define icallRomLeaveCSPtr     (uint32 *)(&RAM_BASE_ADDR+2)
#define r2fRomPtr              (uint32 *)(&RAM_BASE_ADDR+3)
#define r2rRomPtr              (uint32 *)(&RAM_BASE_ADDR+4)

// Runtime Init code for Common ROM
#define RT_Init_ROM ((RT_Init_fp)&ROM_BASE_ADDR)

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef void (*RT_Init_fp)(void);

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// ROM-to-Flash Flash Jump Table
// Note: Any change here must accompany a change to R2F_FlashJT.h defines!

// Patched Externs
//extern

#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(R2F_Flash_JT, 4)
#else
#pragma data_alignment=4
#endif
const uint32 R2F_Flash_JT[] =
{
  // ROM-to-Flash Functions
  (uint32)osal_mem_alloc,                                // R2F_JT_OFFSET[0]
  (uint32)osal_mem_free,                                 // R2F_JT_OFFSET[1]
  (uint32)osal_pwrmgr_task_state,                        // R2F_JT_OFFSET[2]
  (uint32)osal_msg_allocate,                             // R2F_JT_OFFSET[3]
  (uint32)osal_msg_send,                                 // R2F_JT_OFFSET[4]
  (uint32)osal_set_event,                                // R2F_JT_OFFSET[5]
  (uint32)osal_memcpy,                                   // R2F_JT_OFFSET[6]
  (uint32)osal_memset,                                   // R2F_JT_OFFSET[7]
  (uint32)osal_bm_alloc,                                 // R2F_JT_OFFSET[8]
  (uint32)osal_bm_free,                                  // R2F_JT_OFFSET[9]
  (uint32)osal_bm_adjust_header,                         // R2F_JT_OFFSET[10]
  (uint32)osal_start_timerEx,                            // R2F_JT_OFFSET[11]
  (uint32)osal_stop_timerEx,                             // R2F_JT_OFFSET[12]
  (uint32)osal_clear_event,                              // R2F_JT_OFFSET[13]
  (uint32)Onboard_soft_reset,                            // R2F_JT_OFFSET[14]
  (uint32)IntMasterEnable,                               // R2F_JT_OFFSET[15]
  (uint32)IntMasterDisable,                              // R2F_JT_OFFSET[16]
  (uint32)NOROM_IntEnable,                               // R2F_JT_OFFSET[17]
  (uint32)NOROM_IntDisable,                              // R2F_JT_OFFSET[18]
  (uint32)halAssertHandler,                              // R2F_JT_OFFSET[19]
  (uint32)HalTRNG_InitTRNG,                              // R2F_JT_OFFSET[20]
  (uint32)HalTRNG_GetTRNG,                               // R2F_JT_OFFSET[21]
  (uint32)LL_PM_Init,                                    // R2F_JT_OFFSET[22]
  (uint32)LL_PM_GetRfCoreState,                          // R2F_JT_OFFSET[23]
  (uint32)LL_PM_StartRfTask,                             // R2F_JT_OFFSET[24]
  (uint32)LL_PM_PowerOnReq,                              // R2F_JT_OFFSET[25]
  (uint32)LL_PM_PowerCycleRadio,                         // R2F_JT_OFFSET[26]
  (uint32)LL_PM_ForceSysBusThroughRF,                    // R2F_JT_OFFSET[27]
  (uint32)LL_PM_Enter_AES,                               // R2F_JT_OFFSET[28]
  (uint32)LL_PM_Exit_AES,                                // R2F_JT_OFFSET[29]
  (uint32)LL_PM_PowerOnRfCore,                           // R2F_JT_OFFSET[30]
  (uint32)LL_PM_PowerOffRfCore,                          // R2F_JT_OFFSET[31]
  (uint32)LL_PM_StopCurTaskTimer,                        // R2F_JT_OFFSET[32]
  // ROM-to-RAM Data
  (uint32)&hciTaskID,                                    // R2F_JT_OFFSET[33]
  (uint32)&hciL2capTaskID,                               // R2F_JT_OFFSET[34]
  (uint32)&hciGapTaskID,                                 // R2F_JT_OFFSET[35]
  (uint32)&hciSmpTaskID,                                 // R2F_JT_OFFSET[36]
  (uint32)&hciTestTaskID,                                // R2F_JT_OFFSET[37]
  (uint32)&llConfigTable,                                // R2F_JT_OFFSET[38]
};

#include "ll.h"
#include "ll_common.h"
#include "ll_enc.h"
#include "ll_wl.h"
#include "ll_timerDrift.h"
#include "ll_rat.h"

// ROM-to-ROM Flash Jump Table - Controller ROM
// Note: Any change here must accompany a change to R2R_FlashJT.h defines!

#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(R2R_Flash_JT, 4)
#else
#pragma data_alignment=4
#endif
const uint32 R2R_Flash_JT[] =
{
  // HCI ROM-to-ROM Functions
  (uint32)HCI_CommandCompleteEvent,                      // R2R_JT_OFFSET(0)
  (uint32)HCI_CommandStatusEvent,                        // R2R_JT_OFFSET(1)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_DataBufferOverflowEvent,                   // R2R_JT_OFFSET(2)
  (uint32)HCI_DisconnectCmd,                             // R2R_JT_OFFSET(3)
#else // not part of build configuration
  (uint32)(uint32)ROM_Spinlock,                          // PLACE HOLDER
  (uint32)(uint32)ROM_Spinlock,                          // PLACE HOLDER
  #define HCI_DataBufferOverflowEvent
  #define HCI_DisconnectCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)HCI_EXT_AdvEventNoticeCmd,                     // R2R_JT_OFFSET(4)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_AdvEventNoticeCmd
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

  (uint32)HCI_EXT_BuildRevisionCmd,                      // R2R_JT_OFFSET(5)
  (uint32)HCI_EXT_ClkDivOnHaltCmd,                       // R2R_JT_OFFSET(6)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_EXT_ConnEventNoticeCmd,                    // R2R_JT_OFFSET(7)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_ConnEventNoticeCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_EXT_DeclareNvUsageCmd,                     // R2R_JT_OFFSET(8)
  (uint32)HCI_EXT_DecryptCmd,                            // R2R_JT_OFFSET(9)
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
  (uint32)HCI_EXT_DelaySleepCmd,                         // R2R_JT_OFFSET(10)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_EXT_DisconnectImmedCmd,                    // R2R_JT_OFFSET(11)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_DisconnectImmedCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_EXT_EnablePTMCmd,                          // R2R_JT_OFFSET(12)
  (uint32)HCI_EXT_EndModemTestCmd,                       // R2R_JT_OFFSET(13)
  (uint32)HCI_EXT_ExtendRfRangeCmd,                      // R2R_JT_OFFSET(14)
  (uint32)HCI_EXT_HaltDuringRfCmd,                       // R2R_JT_OFFSET(15)
  (uint32)HCI_EXT_MapPmIoPortCmd,                        // R2R_JT_OFFSET(16)
  (uint32)HCI_EXT_ModemHopTestTxCmd,                     // R2R_JT_OFFSET(17)
  (uint32)HCI_EXT_ModemTestRxCmd,                        // R2R_JT_OFFSET(18)
  (uint32)HCI_EXT_ModemTestTxCmd,                        // R2R_JT_OFFSET(19)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_EXT_NumComplPktsLimitCmd,                  // R2R_JT_OFFSET(20)
  (uint32)HCI_EXT_OnePktPerEvtCmd,                       // R2R_JT_OFFSET(21)
  (uint32)HCI_EXT_OverlappedProcessingCmd,               // R2R_JT_OFFSET(22)
  (uint32)HCI_EXT_PERbyChanCmd,                          // R2R_JT_OFFSET(23)
  (uint32)HCI_EXT_PacketErrorRateCmd,                    // R2R_JT_OFFSET(24)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_NumComplPktsLimitCmd
  #define HCI_EXT_OnePktPerEvtCmd
  #define HCI_EXT_OverlappedProcessingCmd
  #define HCI_EXT_PERbyChanCmd
  #define HCI_EXT_PacketErrorRateCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_EXT_ResetSystemCmd,                        // R2R_JT_OFFSET(25)
  (uint32)HCI_EXT_SaveFreqTuneCmd,                       // R2R_JT_OFFSET(26)
  (uint32)HCI_EXT_SetBDADDRCmd,                          // R2R_JT_OFFSET(27)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)HCI_EXT_SetFastTxResponseTimeCmd,              // R2R_JT_OFFSET(28)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_SetFastTxResponseTimeCmd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)HCI_EXT_SetFreqTuneCmd,                        // R2R_JT_OFFSET(29)
  (uint32)HCI_EXT_SetLocalSupportedFeaturesCmd,          // R2R_JT_OFFSET(30)
  (uint32)HCI_EXT_SetMaxDtmTxPowerCmd,                   // R2R_JT_OFFSET(31)
  (uint32)HCI_EXT_SetRxGainCmd,                          // R2R_JT_OFFSET(32)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_EXT_SetSCACmd,                             // R2R_JT_OFFSET(33)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_SetSCACmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)HCI_EXT_SetSlaveLatencyOverrideCmd,            // R2R_JT_OFFSET(34)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_SetSlaveLatencyOverrideCmd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)HCI_EXT_SetTxPowerCmd,                         // R2R_JT_OFFSET(35)
  (uint32)HCI_HardwareErrorEvent,                        // R2R_JT_OFFSET(36)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_HostBufferSizeCmd,                         // R2R_JT_OFFSET(37)
  (uint32)HCI_HostNumCompletedPktCmd,                    // R2R_JT_OFFSET(38)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_HostBufferSizeCmd
  #define HCI_HostNumCompletedPktCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_LE_AddWhiteListCmd,                        // R2R_JT_OFFSET(39)
  (uint32)HCI_LE_ClearWhiteListCmd,                      // R2R_JT_OFFSET(40)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_LE_ConnUpdateCmd,                          // R2R_JT_OFFSET(41)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_ConnUpdateCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)HCI_LE_CreateConnCancelCmd,                    // R2R_JT_OFFSET(42)
  (uint32)HCI_LE_CreateConnCmd,                          // R2R_JT_OFFSET(43)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_CreateConnCancelCmd
  #define HCI_LE_CreateConnCmd
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)HCI_LE_EncryptCmd,                             // R2R_JT_OFFSET(44)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)HCI_LE_LtkReqNegReplyCmd,                      // R2R_JT_OFFSET(45)
  (uint32)HCI_LE_LtkReqReplyCmd,                         // R2R_JT_OFFSET(46)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_LtkReqNegReplyCmd
  #define HCI_LE_LtkReqReplyCmd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)HCI_LE_RandCmd,                                // R2R_JT_OFFSET(47)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)HCI_LE_ReadAdvChanTxPowerCmd,                  // R2R_JT_OFFSET(48)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_ReadAdvChanTxPowerCmd
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

  (uint32)HCI_LE_ReadBufSizeCmd,                         // R2R_JT_OFFSET(49)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_LE_ReadChannelMapCmd,                      // R2R_JT_OFFSET(50)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_ReadChannelMapCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_LE_ReadLocalSupportedFeaturesCmd,          // R2R_JT_OFFSET(51)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_LE_ReadRemoteUsedFeaturesCmd,              // R2R_JT_OFFSET(52)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_ReadRemoteUsedFeaturesCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_LE_ReadSupportedStatesCmd,                 // R2R_JT_OFFSET(53)
  (uint32)HCI_LE_ReadWhiteListSizeCmd,                   // R2R_JT_OFFSET(54)
  (uint32)HCI_LE_ReceiverTestCmd,                        // R2R_JT_OFFSET(55)
  (uint32)HCI_LE_RemoveWhiteListCmd,                     // R2R_JT_OFFSET(56)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)HCI_LE_SetAdvDataCmd,                          // R2R_JT_OFFSET(57)
  (uint32)HCI_LE_SetAdvEnableCmd,                        // R2R_JT_OFFSET(58)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_SetAdvDataCmd
  #define HCI_LE_SetAdvEnableCmd
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_LE_SetAdvParamCmd,                         // R2R_JT_OFFSET(59)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_SetAdvParamCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_LE_SetEventMaskCmd,                        // R2R_JT_OFFSET(60)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)HCI_LE_SetHostChanClassificationCmd,           // R2R_JT_OFFSET(61)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_SetHostChanClassificationCmd
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)HCI_LE_SetRandomAddressCmd,                    // R2R_JT_OFFSET(62)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)HCI_LE_SetScanEnableCmd,                       // R2R_JT_OFFSET(63)
  (uint32)HCI_LE_SetScanParamCmd,                        // R2R_JT_OFFSET(64)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_SetScanEnableCmd
  #define HCI_LE_SetScanParamCmd
#endif

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)HCI_LE_SetScanRspDataCmd,                      // R2R_JT_OFFSET(65)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_SetScanRspDataCmd
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)HCI_LE_StartEncyptCmd,                         // R2R_JT_OFFSET(66)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_StartEncyptCmd
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)HCI_LE_TestEndCmd,                             // R2R_JT_OFFSET(67)
  (uint32)HCI_LE_TransmitterTestCmd,                     // R2R_JT_OFFSET(68)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_NumOfCompletedPacketsEvent,                // R2R_JT_OFFSET(69)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_NumOfCompletedPacketsEvent
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_ReadBDADDRCmd,                             // R2R_JT_OFFSET(70)
  (uint32)HCI_ReadLocalSupportedCommandsCmd,             // R2R_JT_OFFSET(71)
// ROM WORKAROUND - Remove. This is not a BLE command.
  (uint32)HCI_ReadLocalSupportedFeaturesCmd,             // R2R_JT_OFFSET(72)
  (uint32)HCI_ReadLocalVersionInfoCmd,                   // R2R_JT_OFFSET(73)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_ReadRemoteVersionInfoCmd,                  // R2R_JT_OFFSET(74)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_ReadRemoteVersionInfoCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_ReadRssiCmd,                               // R2R_JT_OFFSET(75)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_ReadTransmitPowerLevelCmd,                 // R2R_JT_OFFSET(76)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_ReadTransmitPowerLevelCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_ResetCmd,                                  // R2R_JT_OFFSET(77)
  (uint32)HCI_ReverseBytes,                              // R2R_JT_OFFSET(78)
  (uint32)HCI_SendCommandCompleteEvent,                  // R2R_JT_OFFSET(79)
  (uint32)HCI_SendCommandStatusEvent,                    // R2R_JT_OFFSET(80)
  (uint32)HCI_SendControllerToHostEvent,                 // R2R_JT_OFFSET(81)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_SendDataPkt,                               // R2R_JT_OFFSET(82)
  (uint32)HCI_SetControllerToHostFlowCtrlCmd,            // R2R_JT_OFFSET(83)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_SendDataPkt
  #define HCI_SetControllerToHostFlowCtrlCmd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)HCI_SetEventMaskCmd,                           // R2R_JT_OFFSET(84)
  (uint32)HCI_ValidConnTimeParams,                       // R2R_JT_OFFSET(85)
  (uint32)HCI_VendorSpecifcCommandCompleteEvent,         // R2R_JT_OFFSET(86)
  (uint32)HCI_bm_alloc,                                  // R2R_JT_OFFSET(87)

  // LL ROM-to-ROM Functions
  (uint32)LL_AddWhiteListDevice,                         // R2R_JT_OFFSET(88)
  (uint32)LL_AdvReportCback,                             // R2R_JT_OFFSET(89)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)LL_ChanMapUpdate,                              // R2R_JT_OFFSET(90)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ChanMapUpdate
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)LL_ClearWhiteList,                             // R2R_JT_OFFSET(91)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ConnActive,                                 // R2R_JT_OFFSET(92)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ConnActive
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)LL_ConnParamUpdateCback,                       // R2R_JT_OFFSET(93)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ConnParamUpdateCback
#endif // CTRL_V41_CONFIG=CONN_PARAM_REQ_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ConnUpdate,                                 // R2R_JT_OFFSET(94)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ConnUpdate
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)
#else // feature not included
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)LL_ConnUpdate,                                 // R2R_JT_OFFSET(94)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ConnUpdate
#endif // CTRL_CONFIG=INIT_CFG
#endif // CTRL_V41_CONFIG=CONN_PARAM_REQ_CFG

  (uint32)LL_ConnectionCompleteCback,                    // R2R_JT_OFFSET(95)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)LL_CreateConn,                                 // R2R_JT_OFFSET(96)
  (uint32)LL_CreateConnCancel,                           // R2R_JT_OFFSET(97)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_CreateConn
  #define LL_CreateConnCancel
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_CtrlToHostFlowControl,                      // R2R_JT_OFFSET(98)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_CtrlToHostFlowControl
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)LL_DirectTestEnd,                              // R2R_JT_OFFSET(99)
  (uint32)LL_DirectTestEndDoneCback,                     // R2R_JT_OFFSET(100)
  (uint32)LL_DirectTestRxTest,                           // R2R_JT_OFFSET(101)
  (uint32)LL_DirectTestTxTest,                           // R2R_JT_OFFSET(102)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_Disconnect,                                 // R2R_JT_OFFSET(103)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_Disconnect
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)LL_DisconnectCback,                            // R2R_JT_OFFSET(104)
  (uint32)LL_ENC_AES128_Decrypt,                         // R2R_JT_OFFSET(105)
  (uint32)LL_ENC_AES128_Encrypt,                         // R2R_JT_OFFSET(106)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ENC_Decrypt,                                // R2R_JT_OFFSET(107)
  (uint32)LL_ENC_DecryptMsg,                             // R2R_JT_OFFSET(108)
  (uint32)LL_ENC_Encrypt,                                // R2R_JT_OFFSET(109)
  (uint32)LL_ENC_EncryptMsg,                             // R2R_JT_OFFSET(110)
  (uint32)LL_ENC_GenDeviceIV,                            // R2R_JT_OFFSET(111)
  (uint32)LL_ENC_GenDeviceSKD,                           // R2R_JT_OFFSET(112)
  (uint32)LL_ENC_GenerateNonce,                          // R2R_JT_OFFSET(113)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ENC_Decrypt
  #define LL_ENC_DecryptMsg
  #define LL_ENC_Encrypt
  #define LL_ENC_EncryptMsg
  #define LL_ENC_GenDeviceIV
  #define LL_ENC_GenDeviceSKD
  #define LL_ENC_GenerateNonce
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_ENC_GeneratePseudoRandNum,                  // R2R_JT_OFFSET(114)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG | SCAN_CFG))
  (uint32)LL_ENC_GenerateTrueRandNum,                    // R2R_JT_OFFSET(115)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ENC_GenerateTrueRandNum
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG | SCAN_CFG

  (uint32)LL_ENC_Init,                                   // R2R_JT_OFFSET(116)
  (uint32)LL_ENC_LoadKey,                                // R2R_JT_OFFSET(117)
  (uint32)LL_ENC_ReverseBytes,                           // R2R_JT_OFFSET(118)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)LL_EXT_AdvEventNotice,                         // R2R_JT_OFFSET(119)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_AdvEventNotice
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

  (uint32)LL_EXT_BuildRevision,                          // R2R_JT_OFFSET(120)
  (uint32)LL_EXT_ClkDivOnHalt,                           // R2R_JT_OFFSET(121)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_EXT_ConnEventNotice,                        // R2R_JT_OFFSET(122)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_ConnEventNotice
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)LL_EXT_DeclareNvUsage,                         // R2R_JT_OFFSET(123)
  (uint32)LL_EXT_Decrypt,                                // R2R_JT_OFFSET(124)
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
(uint32)LL_EXT_DelaySleep,                             // R2R_JT_OFFSET(125)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_EXT_DisconnectImmed,                        // R2R_JT_OFFSET(126)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_DisconnectImmed
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_EXT_EndModemTest,                           // R2R_JT_OFFSET(127)
  (uint32)LL_EXT_ExtendRfRange,                          // R2R_JT_OFFSET(128)
  (uint32)LL_EXT_ExtendRfRangeCback,                     // R2R_JT_OFFSET(129)
  (uint32)LL_EXT_HaltDuringRf,                           // R2R_JT_OFFSET(130)
  (uint32)LL_EXT_MapPmIoPort,                            // R2R_JT_OFFSET(131)
  (uint32)LL_EXT_ModemHopTestTx,                         // R2R_JT_OFFSET(132)
  (uint32)LL_EXT_ModemTestRx,                            // R2R_JT_OFFSET(133)
  (uint32)LL_EXT_ModemTestTx,                            // R2R_JT_OFFSET(134)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_EXT_NumComplPktsLimit,                      // R2R_JT_OFFSET(135)
  (uint32)LL_EXT_OnePacketPerEvent,                      // R2R_JT_OFFSET(136)
  (uint32)LL_EXT_OverlappedProcessing,                   // R2R_JT_OFFSET(137)
  (uint32)LL_EXT_PERbyChan,                              // R2R_JT_OFFSET(138)
  (uint32)LL_EXT_PacketErrorRate,                        // R2R_JT_OFFSET(139)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_NumComplPktsLimit
  #define LL_EXT_OnePacketPerEvent
  #define LL_EXT_OverlappedProcessing
  #define LL_EXT_PERbyChan
  #define LL_EXT_PacketErrorRate
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_EXT_PacketErrorRateCback,                   // R2R_JT_OFFSET(140)
  (uint32)LL_EXT_ResetSystem,                            // R2R_JT_OFFSET(141)
  (uint32)LL_EXT_SaveFreqTune,                           // R2R_JT_OFFSET(142)
  (uint32)LL_EXT_SetBDADDR,                              // R2R_JT_OFFSET(143)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)LL_EXT_SetFastTxResponseTime,                  // R2R_JT_OFFSET(144)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                          // PLACE HOLDER
  #define LL_EXT_SetFastTxResponseTime
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)LL_EXT_SetFreqTune,                            // R2R_JT_OFFSET(145)
  (uint32)LL_EXT_SetLocalSupportedFeatures,              // R2R_JT_OFFSET(146)
  (uint32)LL_EXT_SetMaxDtmTxPower,                       // R2R_JT_OFFSET(147)
  (uint32)LL_EXT_SetRxGain,                              // R2R_JT_OFFSET(148)
  (uint32)LL_EXT_SetRxGainCback,                         // R2R_JT_OFFSET(149)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_EXT_SetSCA,                                 // R2R_JT_OFFSET(150)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_SetSCA
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)LL_EXT_SetSlaveLatencyOverride,                // R2R_JT_OFFSET(151)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EXT_SetSlaveLatencyOverride
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)LL_EXT_SetTxPower,                             // R2R_JT_OFFSET(152)
  (uint32)LL_EXT_SetTxPowerCback,                        // R2R_JT_OFFSET(153)

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_CONN_CFG) || (CTRL_CONFIG & INIT_CFG))
  (uint32)LL_EncChangeCback,                             // R2R_JT_OFFSET(154)
  (uint32)LL_EncKeyRefreshCback,                         // R2R_JT_OFFSET(155)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EncChangeCback
  #define LL_EncKeyRefreshCback
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)LL_EncLtkNegReply,                             // R2R_JT_OFFSET(156)
  (uint32)LL_EncLtkReply,                                // R2R_JT_OFFSET(157)
  (uint32)LL_EncLtkReqCback,                             // R2R_JT_OFFSET(158)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_EncLtkNegReply
  #define LL_EncLtkReply
  #define LL_EncLtkReqCback
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)LL_Encrypt,                                    // R2R_JT_OFFSET(159)
  (uint32)LL_Init,                                       // R2R_JT_OFFSET(160)
  (uint32)LL_NumEmptyWlEntries,                          // R2R_JT_OFFSET(161)
  (uint32)LL_ProcessEvent,                               // R2R_JT_OFFSET(162)
  (uint32)LL_PseudoRand,                                 // R2R_JT_OFFSET(163)

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_CONN_CFG) || (CTRL_CONFIG & INIT_CFG))
  (uint32)LL_RX_bm_alloc,                                // R2R_JT_OFFSET(164)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_RX_bm_alloc
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_Rand,                                       // R2R_JT_OFFSET(165)
  (uint32)LL_RandCback,                                  // R2R_JT_OFFSET(166)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)LL_ReadAdvChanTxPower,                         // R2R_JT_OFFSET(167)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ReadAdvChanTxPower
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

  (uint32)LL_ReadBDADDR,                                 // R2R_JT_OFFSET(168)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ReadChanMap,                                // R2R_JT_OFFSET(169)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ReadChanMap
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_ReadLocalSupportedFeatures,                 // R2R_JT_OFFSET(170)
  (uint32)LL_ReadLocalVersionInfo,                       // R2R_JT_OFFSET(171)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ReadRemoteUsedFeatures,                     // R2R_JT_OFFSET(172)
  (uint32)LL_ReadRemoteUsedFeaturesCompleteCback,        // R2R_JT_OFFSET(173)
  (uint32)LL_ReadRemoteVersionInfo,                      // R2R_JT_OFFSET(174)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ReadRemoteUsedFeatures
  #define LL_ReadRemoteUsedFeaturesCompleteCback
  #define LL_ReadRemoteVersionInfo
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_ReadRemoteVersionInfoCback,                 // R2R_JT_OFFSET(175)
  (uint32)LL_ReadRssi,                                   // R2R_JT_OFFSET(176)
  (uint32)LL_ReadSupportedStates,                        // R2R_JT_OFFSET(177)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_ReadTxPowerLevel,                           // R2R_JT_OFFSET(178)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ReadTxPowerLevel
#endif // CTRL_CONFIG=ADV_CONN_CFG | INIT_CFG

  (uint32)LL_ReadWlSize,                                 // R2R_JT_OFFSET(179)
  (uint32)LL_RemoveWhiteListDevice,                      // R2R_JT_OFFSET(180)
  (uint32)LL_Reset,                                      // R2R_JT_OFFSET(181)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_RxDataCompleteCback,                        // R2R_JT_OFFSET(182)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_RxDataCompleteCback
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)LL_SetAdvControl,                              // R2R_JT_OFFSET(183)
  (uint32)LL_SetAdvData,                                 // R2R_JT_OFFSET(184)
  (uint32)LL_SetAdvParam,                                // R2R_JT_OFFSET(185)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_SetAdvControl
  #define LL_SetAdvData
  #define LL_SetAdvParam
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

  (uint32)LL_SetRandomAddress,                           // R2R_JT_OFFSET(186)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)LL_SetScanControl,                             // R2R_JT_OFFSET(187)
  (uint32)LL_SetScanParam,                               // R2R_JT_OFFSET(188)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_SetScanControl
  #define LL_SetScanParam
#endif // CTRL_CONFIG=SCAN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)LL_SetScanRspData,                             // R2R_JT_OFFSET(189)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_SetScanRspData
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)LL_StartEncrypt,                               // R2R_JT_OFFSET(190)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_StartEncrypt
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_TX_bm_alloc,                                // R2R_JT_OFFSET(191)
  (uint32)LL_TxData,                                     // R2R_JT_OFFSET(192)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_TX_bm_alloc
  #define LL_TxData
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)WL_AddEntry,                                   // R2R_JT_OFFSET(193)
  (uint32)WL_Clear,                                      // R2R_JT_OFFSET(194)
  (uint32)WL_ClearEntry,                                 // R2R_JT_OFFSET(195)
  (uint32)WL_ClearIgnoreList,                            // R2R_JT_OFFSET(196)
  (uint32)WL_FindEntry,                                  // R2R_JT_OFFSET(197)
  (uint32)WL_GetNumFreeEntries,                          // R2R_JT_OFFSET(198)
  (uint32)WL_GetSize,                                    // R2R_JT_OFFSET(199)
  (uint32)WL_Init,                                       // R2R_JT_OFFSET(200)
  (uint32)WL_RemoveEntry,                                // R2R_JT_OFFSET(201)
  (uint32)WL_SetWlIgnore,                                // R2R_JT_OFFSET(202)
  (uint32)llActiveTask,                                  // R2R_JT_OFFSET(203)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)llAdv_TaskAbort,                               // R2R_JT_OFFSET(204)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAdv_TaskAbort
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llAdv_TaskConnect,                             // R2R_JT_OFFSET(205)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAdv_TaskConnect
#endif // CTRL_CONFIG=ADV_CONN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)llAdv_TaskEnd,                                 // R2R_JT_OFFSET(206)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAdv_TaskEnd
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llAllocConnId,                                 // R2R_JT_OFFSET(207)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAllocConnId
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llAllocTask,                                   // R2R_JT_OFFSET(208)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llAtLeastTwoChans,                             // R2R_JT_OFFSET(209)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAtLeastTwoChans
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llCalcScaFactor,                               // R2R_JT_OFFSET(210)
  (uint32)llCheckForLstoDuringSL,                        // R2R_JT_OFFSET(211)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llCalcScaFactor
  #define llCheckForLstoDuringSL
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llCheckWhiteListUsage,                         // R2R_JT_OFFSET(212)
  (uint32)llClearRfInts,                                 // R2R_JT_OFFSET(213)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llConnCleanup,                                 // R2R_JT_OFFSET(214)
  (uint32)llConnTerminate,                               // R2R_JT_OFFSET(215)
  (uint32)llConvertCtrlProcTimeoutToEvent,               // R2R_JT_OFFSET(216)
  (uint32)llConvertLstoToEvent,                          // R2R_JT_OFFSET(217)
  (uint32)llDataQEmpty,                                  // R2R_JT_OFFSET(218)
  (uint32)llDataQFull,                                   // R2R_JT_OFFSET(219)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llConnCleanup
  #define llConnTerminate
  #define llConvertCtrlProcTimeoutToEvent
  #define llConvertLstoToEvent
  #define llDataQEmpty
  #define llDataQFull
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llDequeueCtrlPkt,                              // R2R_JT_OFFSET(220)
  (uint32)llDequeueDataQ,                                // R2R_JT_OFFSET(221)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llDequeueCtrlPkt
  #define llDequeueDataQ
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llDirAdv_TaskEnd,                              // R2R_JT_OFFSET(222)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llDirAdv_TaskEnd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llDisableRfInts,                               // R2R_JT_OFFSET(223)
  (uint32)llEnableRfInts,                                // R2R_JT_OFFSET(224)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llEnqueueCtrlPkt,                              // R2R_JT_OFFSET(225)
  (uint32)llEnqueueDataQ,                                // R2R_JT_OFFSET(226)
  (uint32)llEnqueueHeadDataQ,                            // R2R_JT_OFFSET(227)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llEnqueueCtrlPkt
  #define llEnqueueDataQ
  #define llEnqueueHeadDataQ
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llEqAlreadyValidAddr,                          // R2R_JT_OFFSET(228)
  (uint32)llEqSynchWord,                                 // R2R_JT_OFFSET(229)
  (uint32)llEqualBytes,                                  // R2R_JT_OFFSET(230)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llEqAlreadyValidAddr
  #define llEqSynchWord
  #define llEqualBytes
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)llEventDelta,                                  // R2R_JT_OFFSET(231)
  (uint32)llEventInRange,                                // R2R_JT_OFFSET(232)
  (uint32)llFindNextSecTask,                             // R2R_JT_OFFSET(233)
  (uint32)llFindStartType,                               // R2R_JT_OFFSET(234)
  (uint32)llFreeTask,                                    // R2R_JT_OFFSET(235)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llGenerateCRC,                                 // R2R_JT_OFFSET(236)
  (uint32)llGenerateValidAccessAddr,                     // R2R_JT_OFFSET(237)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGenerateCRC
  #define llGenerateValidAccessAddr
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)llGetActiveTasks,                              // R2R_JT_OFFSET(238)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)llGetAdvChanPDU,                               // R2R_JT_OFFSET(239)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGetAdvChanPDU
#endif // CTRL_CONFIG=SCAN_CFG

  (uint32)llGetCurrentTask,                              // R2R_JT_OFFSET(240)
  (uint32)llGetCurrentTime,                              // R2R_JT_OFFSET(241)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llGetMinCI,                                    // R2R_JT_OFFSET(242)
  (uint32)llGetNextConn,                                 // R2R_JT_OFFSET(243)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGetMinCI
  #define llGetNextConn
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llGetNextDataChan,                             // R2R_JT_OFFSET(244)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGetNextDataChan
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llGetNumTasks,                                 // R2R_JT_OFFSET(245)

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_CONN_CFG) || (CTRL_CONFIG & INIT_CFG))
  (uint32)llGetNumTxDataEntries,                         // R2R_JT_OFFSET(246)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGetNumTxDataEntries
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llGetTask,                                     // R2R_JT_OFFSET(247)
  (uint32)llGetTaskState,                                // R2R_JT_OFFSET(248)
  (uint32)llGetTxPower,                                  // R2R_JT_OFFSET(249)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llGtSixConsecZerosOrOnes,                      // R2R_JT_OFFSET(250)
  (uint32)llGtTwentyFourTransitions,                     // R2R_JT_OFFSET(251)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGtSixConsecZerosOrOnes
  #define llGtTwentyFourTransitions
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llHaltRadio,                                   // R2R_JT_OFFSET(252)
  (uint32)llInitFeatureSet,                              // R2R_JT_OFFSET(253)
  (uint32)llInitRAT,                                     // R2R_JT_OFFSET(254)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llInit_TaskConnect,                            // R2R_JT_OFFSET(255)
  (uint32)llInit_TaskEnd,                                // R2R_JT_OFFSET(256)
  (uint32)llLtTwoChangesInLastSixBits,                   // R2R_JT_OFFSET(257)
  (uint32)llMaster_TaskEnd,                              // R2R_JT_OFFSET(258)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llInit_TaskConnect
  #define llInit_TaskEnd
  #define llLtTwoChangesInLastSixBits
  #define llMaster_TaskEnd
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llMemCopyDst,                                  // R2R_JT_OFFSET(259)
  (uint32)llMemCopySrc,                                  // R2R_JT_OFFSET(260)

#if defined(CTRL_CONFIG) && ((CTRL_CONFIG & ADV_CONN_CFG) || (CTRL_CONFIG & INIT_CFG))
  (uint32)llMoveTempTxDataEntries,                       // R2R_JT_OFFSET(261)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llMoveTempTxDataEntries
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llOneBitSynchWordDiffer,                       // R2R_JT_OFFSET(262)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llOneBitSynchWordDiffer
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llPatchCM0,                                    // R2R_JT_OFFSET(263)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llPendingUpdateParam,                          // R2R_JT_OFFSET(264)
  (uint32)llProcessChanMap,                              // R2R_JT_OFFSET(265)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llPendingUpdateParam
  #define llProcessChanMap
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llProcessMasterControlPacket,                  // R2R_JT_OFFSET(266)
  (uint32)llProcessMasterControlProcedures,              // R2R_JT_OFFSET(267)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llProcessMasterControlPacket
  #define llProcessMasterControlProcedures
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llProcessTxData,                               // R2R_JT_OFFSET(268)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llProcessTxData
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llProcessPostRfOps,                            // R2R_JT_OFFSET(269)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)llProcessScanRxFIFO,                           // R2R_JT_OFFSET(270)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llProcessScanRxFIFO
#endif // CTRL_CONFIG=SCAN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llProcessSlaveControlPacket,                   // R2R_JT_OFFSET(271)
  (uint32)llProcessSlaveControlProcedures,               // R2R_JT_OFFSET(272)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llProcessSlaveControlPacket
  #define llProcessSlaveControlProcedures
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llRatChanCBack_A,                              // R2R_JT_OFFSET(273)
  (uint32)llRatChanCBack_B,                              // R2R_JT_OFFSET(274)
  (uint32)llRatChanCBack_C,                              // R2R_JT_OFFSET(275)
  (uint32)llRatChanCBack_D,                              // R2R_JT_OFFSET(276)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llReleaseAllConnId,                            // R2R_JT_OFFSET(277)
  (uint32)llReleaseConnId,                               // R2R_JT_OFFSET(278)
  (uint32)llReplaceCtrlPkt,                              // R2R_JT_OFFSET(279)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llReleaseAllConnId
  #define llReleaseConnId
  #define llReplaceCtrlPkt
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llResetRadio,                                  // R2R_JT_OFFSET(280)
  (uint32)llRfInit,                                      // R2R_JT_OFFSET(281)
  (uint32)llRfStartFS,                                   // R2R_JT_OFFSET(282)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)llScan_TaskEnd,                                // R2R_JT_OFFSET(283)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llScan_TaskEnd
#endif // CTRL_CONFIG=SCAN_CFG

  (uint32)llScheduleTask,                                // R2R_JT_OFFSET(284)
  (uint32)llScheduler,                                   // R2R_JT_OFFSET(285)
  (uint32)llSchedulerInit,                               // R2R_JT_OFFSET(286)
  (uint32)llSetFreqTune,                                 // R2R_JT_OFFSET(287)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetNextDataChan,                             // R2R_JT_OFFSET(288)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetNextDataChan
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llSetTxPower,                                  // R2R_JT_OFFSET(289)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  (uint32)llSetupAdv,                                    // R2R_JT_OFFSET(290)
  (uint32)llSetupAdvDataEntryQueue,                      // R2R_JT_OFFSET(291)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupAdv
  #define llSetupAdvDataEntryQueue
#endif // CTRL_CONFIG=(ADV_NCONN_CFG | ADV_CONN_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupConn,                                   // R2R_JT_OFFSET(292)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupConn
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupConnRxDataEntryQueue,                   // R2R_JT_OFFSET(293)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupConnRxDataEntryQueue
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupEncReq,                                 // R2R_JT_OFFSET(294)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupEncReq
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llSetupEncRsp,                                 // R2R_JT_OFFSET(295)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupEncRsp
#endif // CTRL_CONFIG=ADV_CONN_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & SLV_FEAT_EXCHG_CFG)
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupFeatureSetReq,                          // R2R_JT_OFFSET(296)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupFeatureSetReq
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)
#else // feature not included
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupFeatureSetReq,                          // R2R_JT_OFFSET(296)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupFeatureSetReq
#endif // CTRL_CONFIG=INIT_CFG
#endif // CTRL_V41_CONFIG=SLV_FEAT_EXCHG_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupFeatureSetRsp,                          // R2R_JT_OFFSET(297)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupFeatureSetRsp
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupInit,                                   // R2R_JT_OFFSET(298)
  (uint32)llSetupInitDataEntryQueue,                     // R2R_JT_OFFSET(299)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupInit
  #define llSetupInitDataEntryQueue
#endif // CTRL_CONFIG=INIT_CFG

  (uint32)llSetupMailbox,                                // R2R_JT_OFFSET(300)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupNextMasterEvent,                        // R2R_JT_OFFSET(301)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupNextMasterEvent
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llSetupNextSlaveEvent,                         // R2R_JT_OFFSET(302)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupNextSlaveEvent
#endif // CTRL_CONFIG=ADV_CONN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupPauseEncReq,                            // R2R_JT_OFFSET(303)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupPauseEncReq
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupPauseEncRsp,                            // R2R_JT_OFFSET(304)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupPauseEncRsp
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)llSetupRATChanCompare,                         // R2R_JT_OFFSET(305)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llSetupRejectInd,                              // R2R_JT_OFFSET(306)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupRejectInd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llSetupRfHal,                                  // R2R_JT_OFFSET(307)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  (uint32)llSetupScan,                                   // R2R_JT_OFFSET(308)
  (uint32)llSetupScanDataEntryQueue,                     // R2R_JT_OFFSET(309)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupScan
  #define llSetupScanDataEntryQueue
#endif // CTRL_CONFIG=SCAN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llSetupStartEncReq,                            // R2R_JT_OFFSET(310)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupStartEncReq
#endif // CTRL_CONFIG=ADV_CONN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupStartEncRsp,                            // R2R_JT_OFFSET(311)
  (uint32)llSetupTermInd,                                // R2R_JT_OFFSET(312)
  (uint32)llSetupUnknownRsp,                             // R2R_JT_OFFSET(313)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupStartEncRsp
  #define llSetupTermInd
  #define llSetupUnknownRsp
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llSetupUpdateChanReq,                          // R2R_JT_OFFSET(314)
  (uint32)llSetupUpdateParamReq,                         // R2R_JT_OFFSET(315)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupUpdateChanReq
  #define llSetupUpdateParamReq
#endif // CTRL_CONFIG=INIT_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSetupVersionIndReq,                          // R2R_JT_OFFSET(316)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupVersionIndReq
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llSlave_TaskEnd,                               // R2R_JT_OFFSET(317)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSlave_TaskEnd
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llTimeCompare,                                 // R2R_JT_OFFSET(318)
  (uint32)llTimeDelta,                                   // R2R_JT_OFFSET(319)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  (uint32)llValidAccessAddr,                             // R2R_JT_OFFSET(320)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llValidAccessAddr
#endif // CTRL_CONFIG=ADV_CONN_CFG

  (uint32)llWriteTxData,                                 // R2R_JT_OFFSET(321)
  (uint32)llTaskError,                                   // R2R_JT_OFFSET(322)
  (uint32)llHardwareError,                               // R2R_JT_OFFSET(323)
  (uint32)cpe0IntCback,                                  // R2R_JT_OFFSET(324)
  (uint32)cpe1IntCback,                                  // R2R_JT_OFFSET(325)
  (uint32)hwIntCback,                                    // R2R_JT_OFFSET(326)

  //////////////////////////////////////////////////////////////////////////////
  // PATCH - New Software
  //////////////////////////////////////////////////////////////////////////////

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_EXT_GetConnInfoCmd,                        // R2R_JT_OFFSET(327)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
  (uint32)HCI_ReadAuthPayloadTimeoutCmd,                 // R2R_JT_OFFSET(328)
  (uint32)HCI_WriteAuthPayloadTimeoutCmd,                // R2R_JT_OFFSET(329)
#else // !PING_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_ReadAuthPayloadTimeoutCmd
  #define HCI_WriteAuthPayloadTimeoutCmd
#endif // PING_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)HCI_LE_RemoteConnParamReqReplyCmd,             // R2R_JT_OFFSET(330)
  (uint32)HCI_LE_RemoteConnParamReqNegReplyCmd,          // R2R_JT_OFFSET(331)
#else // !CONN_PARAM_REQ_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_RemoteConnParamReqReplyCmd
  #define HCI_LE_RemoteConnParamReqNegReplyCmd
#endif // CONN_PARAM_REQ_CFG

  (uint32)LL_EXT_GetConnInfo,                            // R2R_JT_OFFSET(332)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
  (uint32)LL_ReadAuthPayloadTimeout,                     // R2R_JT_OFFSET(333)
  (uint32)LL_WriteAuthPayloadTimeout,                    // R2R_JT_OFFSET(334)
  (uint32)LL_AuthPayloadTimeoutExpiredCback,             // R2R_JT_OFFSET(335)
#else // !PING_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_ReadAuthPayloadTimeout
  #define LL_WriteAuthPayloadTimeout
  #define LL_AuthPayloadTimeoutExpiredCback
#endif // PING_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)LL_RemoteConnParamReqReply,                    // R2R_JT_OFFSET(336)
  (uint32)LL_RemoteConnParamReqNegReply,                 // R2R_JT_OFFSET(337)
  (uint32)LL_RemoteConnParamReqCback,                    // R2R_JT_OFFSET(338)
#else // !CONN_PARAM_REQ_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_RemoteConnParamReqReply
  #define LL_RemoteConnParamReqNegReply
  #define LL_RemoteConnParamReqCback
#endif // CONN_PARAM_REQ_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
  (uint32)llSetupPingReq,                                // R2R_JT_OFFSET(339)
  (uint32)llSetupPingRsp,                                // R2R_JT_OFFSET(340)
#else // !PING_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupPingReq
  #define llSetupPingRsp
#endif // PING_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)llSetupConnParamReq,                           // R2R_JT_OFFSET(341)
#else // !CONN_PARAM_REQ_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupConnParamReq
#endif // CONN_PARAM_REQ_CFG

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
  (uint32)llCBTimer_AptoExpiredCback,                    // R2R_JT_OFFSET(342)
#else // !PING_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llCBTimer_AptoExpiredCback
#endif // PING_CFG

  (uint32)llFragmentPDU,                                 // R2R_JT_OFFSET(343)
  (uint32)llCombinePDU,                                  // R2R_JT_OFFSET(344)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)llSetupRejectIndExt,                           // R2R_JT_OFFSET(345)
#else // !CONN_PARAM_REQ_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupRejectIndExt
#endif // CTRL_V41_CONFIG=CONN_PARAM_REQ_CFG

  (uint32)llVerifyConnParamReqParams,                    // R2R_JT_OFFSET(346)

#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_EXT_GetConnInfoCmd
  #define HCI_ReadAuthPayloadTimeoutCmd
  #define HCI_WriteAuthPayloadTimeoutCmd
  #define HCI_LE_RemoteConnParamReqReplyCmd
  #define HCI_LE_RemoteConnParamReqNegReplyCmd
  #define LL_EXT_GetConnInfo
  #define LL_ReadAuthPayloadTimeout
  #define LL_WriteAuthPayloadTimeout
  #define LL_AuthPayloadTimeoutExpiredCback
  #define LL_RemoteConnParamReqReply
  #define LL_RemoteConnParamReqNegReply
  #define LL_RemoteConnParamReqCback
  #define llSetupPingReq
  #define llSetupPingRsp
  #define llSetupConnParamReq
  #define llCBTimer_AptoExpiredCback
  #define llFragmentPDU
  #define llCombinePDU
  #define llSetupRejectIndExt
  #define llVerifyConnParamReqParams
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG) &&                    \
    defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
  (uint32)llSetupConnParamRsp,                           // R2R_JT_OFFSET(347)
#else // !(ADV_CONN_CFG & CONN_PARAM_REQ_CFG)
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSetupConnParamRsp
#endif // ADV_CONN_CFG & CONN_PARAM_REQ_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  (uint32)llAlignToNextEvent,                            // R2R_JT_OFFSET(348)
#else // !ADV_CONN_CFG
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llAlignToNextEvent
#endif // CTRL_V41_CONFIG=CONN_PARAM_REQ_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llGetTotalNumTxDataEntries,                    // R2R_JT_OFFSET(349)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llGetTotalNumTxDataEntries
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

  (uint32)hciInitEventMasks,                             // R2R_JT_OFFSET(350)
  (uint32)HCI_SetEventMaskPage2Cmd,                      // R2R_JT_OFFSET(351)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG) &&             \
    defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llRealignConn,                                 // R2R_JT_OFFSET(352)
  (uint32)llSortActiveConns,                             // R2R_JT_OFFSET(353)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llRealignConn
  #define llSortActiveConns
#endif // MST_SLV_CFG && (ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)LL_GetNumActiveConns,                          // R2R_JT_OFFSET(354)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define LL_GetNumActiveConns
#endif // CTRL_CONFIG=(ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG) &&             \
    defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llConnExists,                                  // R2R_JT_OFFSET(355)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llConnExists
#endif // MST_SLV_CFG && (ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & PHY_2MBPS_CFG) &&           \
    defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)HCI_LE_ReadPhyCmd,                             // R2R_JT_OFFSET(356)
  (uint32)HCI_LE_SetDefaultPhyCmd,                       // R2R_JT_OFFSET(357)
  (uint32)HCI_LE_SetPhyCmd,                              // R2R_JT_OFFSET(358)
  (uint32)HCI_LE_EnhancedRxTestCmd,                      // R2R_JT_OFFSET(359)
  (uint32)HCI_LE_EnhancedTxTestCmd,                      // R2R_JT_OFFSET(360)
  (uint32)LL_ReadPhy,                                    // R2R_JT_OFFSET(361)
  (uint32)LL_SetDefaultPhy,                              // R2R_JT_OFFSET(362)
  (uint32)LL_SetPhy,                                     // R2R_JT_OFFSET(363)
  (uint32)LL_PhyUpdateCompleteEventCback,                // R2R_JT_OFFSET(364)
  (uint32)LL_EnhancedTxTest,                             // R2R_JT_OFFSET(365)
  (uint32)LL_EnhancedRxTest,                             // R2R_JT_OFFSET(366)
  (uint32)llSetupPhyCtrlPkt,                             // R2R_JT_OFFSET(367)
  (uint32)llGetRfOverrideRegs,                           // R2R_JT_OFFSET(368)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define HCI_LE_ReadPhyCmd
  #define HCI_LE_SetDefaultPhyCmd
  #define HCI_LE_SetPhyCmd
  #define HCI_LE_EnhancedRxTestCmd
  #define HCI_LE_EnhancedTxTestCmd
  #define LL_ReadPhy
  #define LL_SetDefaultPhy
  #define LL_SetPhy
  #define LL_PhyUpdateCompleteEventCback
  #define LL_EnhancedTxTest
  #define LL_EnhancedRxTest
  #define llSetupPhyCtrlPkt
  #define llGetRfOverrideRegs
#endif // PHY_2MBPS_CFG & (ADV_CONN_CFG | INIT_CFG)

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  (uint32)llSendReject,                                  // R2R_JT_OFFSET(369)
#else // not part of build configuration
  (uint32)ROM_Spinlock,                                  // PLACE HOLDER
  #define llSendReject
#endif // ADV_CONN_CFG | INIT_CFG
};


/*******************************************************************************
 * @fn          BLE ROM Spinlock
 *
 * @brief       This routine is used to trap indexing errors in R2R JT.
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
void ROM_Spinlock( void )
{
  volatile uint8 i = 1;

  while(i);
}


/*******************************************************************************
 * @fn          BLE ROM Initialization
 *
 * @brief       This routine initializes the BLE Controller ROM software. First,
 *              the image's CRC is verified. Next, its C runtime is initialized.
 *              Then the ICall function pointers for dispatch, and enter/leave
 *              critical section are initialized. Finally, the location of the
 *              R2F and R2R flash jump tables are initialized.
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
void ROM_Init( void )
{
#if defined( ENABLE_ROM_CHECKSUM_CHECK )
  volatile uint8 i;

  // verify the Controller ROM image
  i = validChecksum(&__checksum_begin, &__checksum_end);

  // trap a checksum failure - what now?
  while( !i );
#endif // ENABLE_ROM_CHECKSUM_CHECK

  /*
  ** Controller ROM
  */

  // execute the ROM C runtime initialization
  // Note: This is the ROM's C Runtime initialization, not the flash's, which
  //       has already taken place.
  RT_Init_ROM();

  // initialize ICall function pointers for ROM
  // Note: The address of these functions is determined by the Application, and
  //       is passed to the Stack image via startup_entry.
  *icallRomDispatchPtr = (uint32)ICall_dispatcher;
  *icallRomEnterCSPtr  = (uint32)ICall_enterCriticalSection;
  *icallRomLeaveCSPtr  = (uint32)ICall_leaveCriticalSection;

  // initialize RAM pointer to R2F Flash JT for ROM code
  *r2fRomPtr = (uint32)R2F_Flash_JT;

  // initialize RAM pointer to R2R Flash JT for ROM code
  *r2rRomPtr = (uint32)R2R_Flash_JT;

  return;
}

/*******************************************************************************
 */

