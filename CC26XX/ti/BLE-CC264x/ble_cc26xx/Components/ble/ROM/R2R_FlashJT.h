/*******************************************************************************
  Filename:       R2R_FlashJT.h
  Revised:        $Date: 2015-07-20 15:51:01 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44375 $

  Description:    This file contains the defines for every LL and HCI function
                  which can be mapped to either itself (for Flash-Only build),
                  or to jump table offset in flash (ROM build). The latter can
                  be used to relocate any function to flash in the event that
                  software needs to be replaced.

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

#ifndef R2R_FLASH_JT_H
#define R2R_FLASH_JT_H

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

/*******************************************************************************
 * CONSTANTS
 */

// ROM's RAM table offset to R2R flash jump table pointer.
#define ROM_RAM_TABLE_R2R                          4

// Defines used for the flash jump table routines that are not part of build.
// Note: Any change to this table must accompany a change to R2R_Flash_JT[]!
#define R2R_JT_LOCATION                            (&RAM_BASE_ADDR[ROM_RAM_TABLE_R2R])

#define R2R_JT_BASE                                (*((uint32 **)R2R_JT_LOCATION))
#define R2R_JT_OFFSET(index)                       (*(R2R_JT_BASE+(index)))

// HCI ROM-to-ROM Functions
#define MAP_HCI_CommandCompleteEvent               ((void            (*) (uint16, uint8, uint8 *))                                                                       R2R_JT_OFFSET(0))
#define MAP_HCI_CommandStatusEvent                 ((void            (*) (uint8, uint16))                                                                                R2R_JT_OFFSET(1))
#define MAP_HCI_DataBufferOverflowEvent            ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(2))
#define MAP_HCI_DisconnectCmd                      ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(3))
#define MAP_HCI_EXT_AdvEventNoticeCmd              ((uint8           (*) (uint8, uint6))                                                                                 R2R_JT_OFFSET(4))
#define MAP_HCI_EXT_BuildRevisionCmd               ((uint8           (*) (uint8, uint16))                                                                                R2R_JT_OFFSET(5))
#define MAP_HCI_EXT_ClkDivOnHaltCmd                ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(6))
#define MAP_HCI_EXT_ConnEventNoticeCmd             ((uint8           (*) (uint16, uint8, uint16))                                                                        R2R_JT_OFFSET(7))
#define MAP_HCI_EXT_DeclareNvUsageCmd              ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(8))
#define MAP_HCI_EXT_DecryptCmd                     ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(9))
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
#define MAP_HCI_EXT_DelaySleepCmd                  ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(10))
#define MAP_HCI_EXT_DisconnectImmedCmd             ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(11))
#define MAP_HCI_EXT_EnablePTMCmd                   ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(12))
#define MAP_HCI_EXT_EndModemTestCmd                ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(13))
#define MAP_HCI_EXT_ExtendRfRangeCmd               ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(14))
#define MAP_HCI_EXT_HaltDuringRfCmd                ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(15))
#define MAP_HCI_EXT_MapPmIoPortCmd                 ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(16))
#define MAP_HCI_EXT_ModemHopTestTxCmd              ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(17))
#define MAP_HCI_EXT_ModemTestRxCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(18))
#define MAP_HCI_EXT_ModemTestTxCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(19))
#define MAP_HCI_EXT_NumComplPktsLimitCmd           ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(20))
#define MAP_HCI_EXT_OnePktPerEvtCmd                ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(21))
#define MAP_HCI_EXT_OverlappedProcessingCmd        ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(22))
#define MAP_HCI_EXT_PERbyChanCmd                   ((uint8           (*) (uint16, perByChan_t *))                                                                        R2R_JT_OFFSET(23))
#define MAP_HCI_EXT_PacketErrorRateCmd             ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(24))
#define MAP_HCI_EXT_ResetSystemCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(25))
#define MAP_HCI_EXT_SaveFreqTuneCmd                ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(26))
#define MAP_HCI_EXT_SetBDADDRCmd                   ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(27))
#define MAP_HCI_EXT_SetFastTxResponseTimeCmd       ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(28))
#define MAP_HCI_EXT_SetFreqTuneCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(29))
#define MAP_HCI_EXT_SetLocalSupportedFeaturesCmd   ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(30))
#define MAP_HCI_EXT_SetMaxDtmTxPowerCmd            ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(31))
#define MAP_HCI_EXT_SetRxGainCmd                   ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(32))
#define MAP_HCI_EXT_SetSCACmd                      ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(33))
#define MAP_HCI_EXT_SetSlaveLatencyOverrideCmd     ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(34))
#define MAP_HCI_EXT_SetTxPowerCmd                  ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(35))
#define MAP_HCI_HardwareErrorEvent                 ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(36))
#define MAP_HCI_HostBufferSizeCmd                  ((uint8           (*) (uint16, uint8, uint16, uint16))                                                                R2R_JT_OFFSET(37))
#define MAP_HCI_HostNumCompletedPktCmd             ((uint8           (*) (uint8, uint16 *, uint16 *))                                                                    R2R_JT_OFFSET(38))
#define MAP_HCI_LE_AddWhiteListCmd                 ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(39))
#define MAP_HCI_LE_ClearWhiteListCmd               ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(40))
#define MAP_HCI_LE_ConnUpdateCmd                   ((uint8           (*) (uint16, uint16, uint16, uint16, uint16, uint16, uint16))                                       R2R_JT_OFFSET(41))
#define MAP_HCI_LE_CreateConnCancelCmd             ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(42))
#define MAP_HCI_LE_CreateConnCmd                   ((uint8           (*) (uint16, uint16, uint8, uint8, uint8 *, uint8, uint16, uint16, uint16, uint16, uint16, uint16)) R2R_JT_OFFSET(43))
#define MAP_HCI_LE_EncryptCmd                      ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(44))
#define MAP_HCI_LE_LtkReqNegReplyCmd               ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(45))
#define MAP_HCI_LE_LtkReqReplyCmd                  ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(46))
#define MAP_HCI_LE_RandCmd                         ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(47))
#define MAP_HCI_LE_ReadAdvChanTxPowerCmd           ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(48))
#define MAP_HCI_LE_ReadBufSizeCmd                  ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(49))
#define MAP_HCI_LE_ReadChannelMapCmd               ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(50))
#define MAP_HCI_LE_ReadLocalSupportedFeaturesCmd   ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(51))
#define MAP_HCI_LE_ReadRemoteUsedFeaturesCmd       ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(52))
#define MAP_HCI_LE_ReadSupportedStatesCmd          ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(53))
#define MAP_HCI_LE_ReadWhiteListSizeCmd            ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(54))
#define MAP_HCI_LE_ReceiverTestCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(55))
#define MAP_HCI_LE_RemoveWhiteListCmd              ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(56))
#define MAP_HCI_LE_SetAdvDataCmd                   ((uint8           (*) (uint8, uint8 *)                                                                                R2R_JT_OFFSET(57))
#define MAP_HCI_LE_SetAdvEnableCmd                 ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(58))
#define MAP_HCI_LE_SetAdvParamCmd                  ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(59))
#define MAP_HCI_LE_SetEventMaskCmd                 ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(60))
#define MAP_HCI_LE_SetHostChanClassificationCmd    ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(61))
#define MAP_HCI_LE_SetRandomAddressCmd             ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(62))
#define MAP_HCI_LE_SetScanEnableCmd                ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(63))
#define MAP_HCI_LE_SetScanParamCmd                 ((uint8           (*) (uint8, uint16, uint16, uint8, uint8))                                                          R2R_JT_OFFSET(64))
#define MAP_HCI_LE_SetScanRspDataCmd               ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(65))
#define MAP_HCI_LE_StartEncyptCmd                  ((uint8           (*) (uint16, uint8 *, uint8 *, uint8 *))                                                            R2R_JT_OFFSET(66))
#define MAP_HCI_LE_TestEndCmd                      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(67))
#define MAP_HCI_LE_TransmitterTestCmd              ((uint8           (*) (uint8, uint8, uint8))                                                                          R2R_JT_OFFSET(68))
#define MAP_HCI_NumOfCompletedPacketsEvent         ((uint8           (*) (uint8, uint16 *, uint16 *))                                                                    R2R_JT_OFFSET(69))
#define MAP_HCI_ReadBDADDRCmd                      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(70))
#define MAP_HCI_ReadLocalSupportedCommandsCmd      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(71))
// ROM WORKAROUND - Remove. This is not a BLE command.
#define MAP_HCI_ReadLocalSupportedFeaturesCmd      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(72))
#define MAP_HCI_ReadLocalVersionInfoCmd            ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(73))
#define MAP_HCI_ReadRemoteVersionInfoCmd           ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(74))
#define MAP_HCI_ReadRssiCmd                        ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(75))
#define MAP_HCI_ReadTransmitPowerLevelCmd          ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(76))
#define MAP_HCI_ResetCmd                           ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(77))
#define MAP_HCI_ReverseBytes                       ((void            (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(78))
#define MAP_HCI_SendCommandCompleteEvent           ((void            (*) (uint8, uint16, uint8, uint8 *))                                                                R2R_JT_OFFSET(79))
#define MAP_HCI_SendCommandStatusEvent             ((void            (*) (uint8, uint16, uint16))                                                                        R2R_JT_OFFSET(80))
#define MAP_HCI_SendControllerToHostEvent          ((void            (*) (uint8, uint8, uint8 *))                                                                        R2R_JT_OFFSET(81))
#define MAP_HCI_SendDataPkt                        ((uint8           (*) (uint16, uint8, uint16, uint8 *))                                                               R2R_JT_OFFSET(82))
#define MAP_HCI_SetControllerToHostFlowCtrlCmd     ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(83))
#define MAP_HCI_SetEventMaskCmd                    ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(84))
#define MAP_HCI_ValidConnTimeParams                ((uint8           (*) (uint16, uint16, uint16, uint16))                                                               R2R_JT_OFFSET(85))
#define MAP_HCI_VendorSpecifcCommandCompleteEvent  ((void            (*) (uint16, uint8, uint8 *))                                                                       R2R_JT_OFFSET(86))
#define MAP_HCI_bm_alloc                           ((void *          (*) (uint16))                                                                                       R2R_JT_OFFSET(87))
// LL ROM-to-ROM Functions
#define MAP_LL_AddWhiteListDevice                  ((uint8           (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(88))
#define MAP_LL_AdvReportCback                      ((void            (*) (uint8, uint8, uint8 *, uint8, uint8 *, int8))                                                  R2R_JT_OFFSET(89))
#define MAP_LL_ChanMapUpdate                       ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(90))
#define MAP_LL_ClearWhiteList                      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(91))
#define MAP_LL_ConnActive                          ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(92))
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & CONN_PARAM_REQ_CFG)
#define MAP_LL_ConnParamUpdateCback                ((void            (*) (uint8, uint16, uint16, uint16, uint16))                                                        R2R_JT_OFFSET(93))
#else // !CONN_PARAM_REQ_CFG
#define MAP_LL_ConnParamUpdateCback                ((void            (*) (uint16, uint16, uint16, uint16))                                                               R2R_JT_OFFSET(93))
#endif // CONN_PARAM_REQ_CFG
#define MAP_LL_ConnUpdate                          ((uint8           (*) (uint16, uint16, uint16, uint16, uint16, uint16, uint16))                                       R2R_JT_OFFSET(94))
#define MAP_LL_ConnectionCompleteCback             ((void            (*) (uint8, uint16, uint8, uint8, uint8 *, uint16, uint16, uint16, uint8))                          R2R_JT_OFFSET(95))
#define MAP_LL_CreateConn                          ((uint8           (*) (uint16, uint16, uint8, uint8, uint8 *, uint8, uint16, uint16, uint16, uint16, uint16, uint16)) R2R_JT_OFFSET(96))
#define MAP_LL_CreateConnCancel                    ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(97))
#define MAP_LL_CtrlToHostFlowControl               ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(98))
#define MAP_LL_DirectTestEnd                       ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(99))
#define MAP_LL_DirectTestEndDoneCback              ((void            (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(100))
#define MAP_LL_DirectTestRxTest                    ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(101))
#define MAP_LL_DirectTestTxTest                    ((uint8           (*) (uint8, uint8, uint8, uint8))                                                                   R2R_JT_OFFSET(102))
#define MAP_LL_Disconnect                          ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(103))
#define MAP_LL_DisconnectCback                     ((void            (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(104))
#define MAP_LL_ENC_AES128_Decrypt                  ((void            (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(105))
#define MAP_LL_ENC_AES128_Encrypt                  ((void            (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(106))
#define MAP_LL_ENC_Decrypt                         ((uint8           (*) (llConnState_t *, uint8, uint8, uint8 *))                                                       R2R_JT_OFFSET(107))
#define MAP_LL_ENC_DecryptMsg                      ((void            (*) (uint8 *, uint8, uint8, uint8 *, uint8 *))                                                      R2R_JT_OFFSET(108))
#define MAP_LL_ENC_Encrypt                         ((void            (*) (llConnState_t *, uint8, uint8, uint8 *))                                                       R2R_JT_OFFSET(109))
#define MAP_LL_ENC_EncryptMsg                      ((void            (*) (uint8 *, uint8, uint8, uint8 *))                                                               R2R_JT_OFFSET(110))
#define MAP_LL_ENC_GenDeviceIV                     ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(111))
#define MAP_LL_ENC_GenDeviceSKD                    ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(112))
#define MAP_LL_ENC_GenerateNonce                   ((void            (*) (uint32, uint8, uint8 *))                                                                       R2R_JT_OFFSET(113))
#define MAP_LL_ENC_GeneratePseudoRandNum           ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(114))
#define MAP_LL_ENC_GenerateTrueRandNum             ((uint8           (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(115))
#define MAP_LL_ENC_Init                            ((void            (*) (void))                                                                                         R2R_JT_OFFSET(116))
#define MAP_LL_ENC_LoadKey                         ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(117))
#define MAP_LL_ENC_ReverseBytes                    ((void            (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(118))
#define MAP_LL_EXT_AdvEventNotice                  ((uint8           (*) (uint8, uint16))                                                                                R2R_JT_OFFSET(119))
#define MAP_LL_EXT_BuildRevision                   ((uint8           (*) (uint8, uint16, uint8 *))                                                                       R2R_JT_OFFSET(120))
#define MAP_LL_EXT_ClkDivOnHalt                    ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(121))
#define MAP_LL_EXT_ConnEventNotice                 ((uint8           (*) (uint16, uint8, uint16))                                                                        R2R_JT_OFFSET(122))
#define MAP_LL_EXT_DeclareNvUsage                  ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(123))
#define MAP_LL_EXT_Decrypt                         ((uint8           (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(124))
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
#define MAP_LL_EXT_DelaySleep                      ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(125))
#define MAP_LL_EXT_DisconnectImmed                 ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(126))
#define MAP_LL_EXT_EndModemTest                    ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(127))
#define MAP_LL_EXT_ExtendRfRange                   ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(128))
#define MAP_LL_EXT_ExtendRfRangeCback              ((void            (*) (void))                                                                                         R2R_JT_OFFSET(129))
#define MAP_LL_EXT_HaltDuringRf                    ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(130))
#define MAP_LL_EXT_MapPmIoPort                     ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(131))
#define MAP_LL_EXT_ModemHopTestTx                  ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(132))
#define MAP_LL_EXT_ModemTestRx                     ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(133))
#define MAP_LL_EXT_ModemTestTx                     ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(134))
#define MAP_LL_EXT_NumComplPktsLimit               ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(135))
#define MAP_LL_EXT_OnePacketPerEvent               ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(136))
#define MAP_LL_EXT_OverlappedProcessing            ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(137))
#define MAP_LL_EXT_PERbyChan                       ((uint8           (*) (uint16, perByChan_t *))                                                                        R2R_JT_OFFSET(138))
#define MAP_LL_EXT_PacketErrorRate                 ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(139))
#define MAP_LL_EXT_PacketErrorRateCback            ((void            (*) (uint16, uint16, uint16, uint16))                                                               R2R_JT_OFFSET(140))
#define MAP_LL_EXT_ResetSystem                     ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(141))
#define MAP_LL_EXT_SaveFreqTune                    ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(142))
#define MAP_LL_EXT_SetBDADDR                       ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(143))
#define MAP_LL_EXT_SetFastTxResponseTime           ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(144))
#define MAP_LL_EXT_SetFreqTune                     ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(145))
#define MAP_LL_EXT_SetLocalSupportedFeatures       ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(146))
#define MAP_LL_EXT_SetMaxDtmTxPower                ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(147))
#define MAP_LL_EXT_SetRxGain                       ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(148))
#define MAP_LL_EXT_SetRxGainCback                  ((void            (*) (void))                                                                                         R2R_JT_OFFSET(149))
#define MAP_LL_EXT_SetSCA                          ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(150))
#define MAP_LL_EXT_SetSlaveLatencyOverride         ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(151))
#define MAP_LL_EXT_SetTxPower                      ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(152))
#define MAP_LL_EXT_SetTxPowerCback                 ((void            (*) (void))                                                                                         R2R_JT_OFFSET(153))
#define MAP_LL_EncChangeCback                      ((void            (*) (uint16, uint8, uint8))                                                                         R2R_JT_OFFSET(154))
#define MAP_LL_EncKeyRefreshCback                  ((void            (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(155))
#define MAP_LL_EncLtkNegReply                      ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(156))
#define MAP_LL_EncLtkReply                         ((uint8           (*) (uint16, uint8 *))                                                                              R2R_JT_OFFSET(157))
#define MAP_LL_EncLtkReqCback                      ((void            (*) (uint16, uint8 *, uint8 *))                                                                     R2R_JT_OFFSET(158))
#define MAP_LL_Encrypt                             ((uint8           (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(159))
#define MAP_LL_Init                                ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(160))
#define MAP_LL_NumEmptyWlEntries                   ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(161))
#define MAP_LL_ProcessEvent                        ((uint16          (*) (uint8, uint16))                                                                                R2R_JT_OFFSET(162))
#define MAP_LL_PseudoRand                          ((uint8           (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(163))
#define MAP_LL_RX_bm_alloc                         ((void *          (*) (uint16))                                                                                       R2R_JT_OFFSET(164))
#define MAP_LL_Rand                                ((uint8           (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(165))
#define MAP_LL_RandCback                           ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(166))
#define MAP_LL_ReadAdvChanTxPower                  ((uint8           (*) (int8 *))                                                                                       R2R_JT_OFFSET(167))
#define MAP_LL_ReadBDADDR                          ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(168))
#define MAP_LL_ReadChanMap                         ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(169))
#define MAP_LL_ReadLocalSupportedFeatures          ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(170))
#define MAP_LL_ReadLocalVersionInfo                ((uint8           (*) (uint8 *, uint16 *, uint16 *))                                                                  R2R_JT_OFFSET(171))
#define MAP_LL_ReadRemoteUsedFeatures              ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(172))
#define MAP_LL_ReadRemoteUsedFeaturesCompleteCback ((uint8           (*) (uint8, uint16, uint8 *))                                                                       R2R_JT_OFFSET(173))
#define MAP_LL_ReadRemoteVersionInfo               ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(174))
#define MAP_LL_ReadRemoteVersionInfoCback          ((void            (*) (uint8, uint16, uint8, uint16, uint16))                                                         R2R_JT_OFFSET(175))
#define MAP_LL_ReadRssi                            ((uint8           (*) (uint16, int8 *))                                                                               R2R_JT_OFFSET(176))
#define MAP_LL_ReadSupportedStates                 ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(177))
#define MAP_LL_ReadTxPowerLevel                    ((uint8           (*) (uint8, uint8, int8 *))                                                                         R2R_JT_OFFSET(178))
#define MAP_LL_ReadWlSize                          ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(179))
#define MAP_LL_RemoveWhiteListDevice               ((uint8           (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(180))
#define MAP_LL_Reset                               ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(181))
#define MAP_LL_RxDataCompleteCback                 ((void            (*) (uint16, uint8 *, uint8, uint8, int8))                                                          R2R_JT_OFFSET(182))
#define MAP_LL_SetAdvControl                       ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(183))
#define MAP_LL_SetAdvData                          ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(184))
#define MAP_LL_SetAdvParam                         ((uint8           (*) (uint16, uint16, uint8, uint8, uint8, uint8 *, uint8, uint8))                                   R2R_JT_OFFSET(185))
#define MAP_LL_SetRandomAddress                    ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(186))
#define MAP_LL_SetScanControl                      ((uint8           (*) (uint8, uint8))                                                                                 R2R_JT_OFFSET(187))
#define MAP_LL_SetScanParam                        ((uint8           (*) (uint8, uint16, uint16, uint8, uint8))                                                          R2R_JT_OFFSET(188))
#define MAP_LL_SetScanRspData                      ((uint8           (*) (uint8, uint8 *))                                                                               R2R_JT_OFFSET(189))
#define MAP_LL_StartEncrypt                        ((uint8           (*) (uint16, uint8 *, uint8 *, uint8 *))                                                            R2R_JT_OFFSET(190))
#define MAP_LL_TX_bm_alloc                         ((void *          (*) (uint16))                                                                                       R2R_JT_OFFSET(191))
#define MAP_LL_TxData                              ((uint8           (*) (uint16, uint8 *, uint8, uint8))                                                                R2R_JT_OFFSET(192))
#define MAP_WL_AddEntry                            ((uint8           (*) (wlTable_t *, uint8 *, uint8, uint8))                                                           R2R_JT_OFFSET(193))
#define MAP_WL_Clear                               ((void            (*) (wlTable_t *))                                                                                  R2R_JT_OFFSET(194))
#define MAP_WL_ClearEntry                          ((void            (*) (wlEntry_t *))                                                                                  R2R_JT_OFFSET(195))
#define MAP_WL_ClearIgnoreList                     ((uint8           (*) (wlTable_t *))                                                                                  R2R_JT_OFFSET(196))
#define MAP_WL_FindEntry                           ((uint8           (*) (wlTable_t *, uint8 *, uint8))                                                                  R2R_JT_OFFSET(197))
#define MAP_WL_GetNumFreeEntries                   ((uint8           (*) (wlTable_t *))                                                                                  R2R_JT_OFFSET(198))
#define MAP_WL_GetSize                             ((uint8           (*) (wlTable_t *))                                                                                  R2R_JT_OFFSET(199))
#define MAP_WL_Init                                ((void            (*) (wlTable_t *))                                                                                  R2R_JT_OFFSET(200))
#define MAP_WL_RemoveEntry                         ((uint8           (*) (wlTable_t *, uint8 *, uint8))                                                                  R2R_JT_OFFSET(201))
#define MAP_WL_SetWlIgnore                         ((uint8           (*) (wlTable_t *, uint8 *, uint8))                                                                  R2R_JT_OFFSET(202))
#define MAP_llActiveTask                           ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(203))
#define MAP_llAdv_TaskAbort                        ((void            (*) (void))                                                                                         R2R_JT_OFFSET(204))
#define MAP_llAdv_TaskConnect                      ((void            (*) (void))                                                                                         R2R_JT_OFFSET(205))
#define MAP_llAdv_TaskEnd                          ((void            (*) (void))                                                                                         R2R_JT_OFFSET(206))
#define MAP_llAllocConnId                          ((llConnState_t * (*) (void))                                                                                         R2R_JT_OFFSET(207))
#define MAP_llAllocTask                            ((taskInfo_t *    (*) (uint8))                                                                                        R2R_JT_OFFSET(208))
#define MAP_llAtLeastTwoChans                      ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(209))
#define MAP_llCalcScaFactor                        ((uint16          (*) (uint8))                                                                                        R2R_JT_OFFSET(210))
#define MAP_llCheckForLstoDuringSL                 ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(211))
#define MAP_llCheckWhiteListUsage                  ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(212))
#define MAP_llClearRfInts                          ((void            (*) (void))                                                                                         R2R_JT_OFFSET(213))
#define MAP_llConnCleanup                          ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(214))
#define MAP_llConnTerminate                        ((void            (*) (llConnState_t *, uint8))                                                                       R2R_JT_OFFSET(215))
#define MAP_llConvertCtrlProcTimeoutToEvent        ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(216))
#define MAP_llConvertLstoToEvent                   ((void            (*) (llConnState_t *, connParam_t *))                                                               R2R_JT_OFFSET(217))
#define MAP_llDataQEmpty                           ((uint8           (*) (llDataQ_t *))                                                                                  R2R_JT_OFFSET(218))
#define MAP_llDataQFull                            ((uint8           (*) (llDataQ_t *))                                                                                  R2R_JT_OFFSET(219))
#define MAP_llDequeueCtrlPkt                       ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(220))
#define MAP_llDequeueDataQ                         ((uint8           (*) (llDataQ_t *, txData_t **))                                                                     R2R_JT_OFFSET(221))
#define MAP_llDirAdv_TaskEnd                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(222))
#define MAP_llDisableRfInts                        ((void            (*) (void))                                                                                         R2R_JT_OFFSET(223))
#define MAP_llEnableRfInts                         ((void            (*) (void))                                                                                         R2R_JT_OFFSET(224))
#define MAP_llEnqueueCtrlPkt                       ((void            (*) (llConnState_t *, uint8))                                                                       R2R_JT_OFFSET(225))
#define MAP_llEnqueueDataQ                         ((uint8           (*) (llDataQ_t *, txData_t *))                                                                      R2R_JT_OFFSET(226))
#define MAP_llEnqueueHeadDataQ                     ((uint8           (*) (llDataQ_t *, txData_t *))                                                                      R2R_JT_OFFSET(227))
#define MAP_llEqAlreadyValidAddr                   ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(228))
#define MAP_llEqSynchWord                          ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(229))
#define MAP_llEqualBytes                           ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(230))
#define MAP_llEventDelta                           ((uint16          (*) (uint16, uint16))                                                                               R2R_JT_OFFSET(231))
#define MAP_llEventInRange                         ((uint8           (*) (uint16, uint16, uint16))                                                                       R2R_JT_OFFSET(232))
#define MAP_llFindNextSecTask                      ((taskInfo_t *    (*) (uint8))                                                                                        R2R_JT_OFFSET(233))
#define MAP_llFindStartType                        ((uint8           (*) (taskInfo_t *, taskInfo_t *))                                                                   R2R_JT_OFFSET(234))
#define MAP_llFreeTask                             ((void            (*) (taskInfo_t **))                                                                                R2R_JT_OFFSET(235))
#define MAP_llGenerateCRC                          ((uint32          (*) (void))                                                                                         R2R_JT_OFFSET(236))
#define MAP_llGenerateValidAccessAddr              ((uint32          (*) (void))                                                                                         R2R_JT_OFFSET(237))
#define MAP_llGetActiveTasks                       ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(238))
#define MAP_llGetAdvChanPDU                        ((void            (*) (uint8 *, uint8 *, uint8 *, uint8 *, uint8 *, int8 *))                                          R2R_JT_OFFSET(239))
#define MAP_llGetCurrentTask                       ((taskInfo_t *    (*) (void))                                                                                         R2R_JT_OFFSET(240))
#define MAP_llGetCurrentTime                       ((uint32          (*) (void))                                                                                         R2R_JT_OFFSET(241))
#define MAP_llGetMinCI                             ((uint16          (*) (uint16))                                                                                       R2R_JT_OFFSET(242))
#define MAP_llGetNextConn                          ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(243))
#define MAP_llGetNextDataChan                      ((uint8           (*) (llConnState_t *, uint16))                                                                      R2R_JT_OFFSET(244))
#define MAP_llGetNumTasks                          ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(245))
#define MAP_llGetNumTxDataEntries                  ((uint8           (*) (dataEntryQ_t *))                                                                               R2R_JT_OFFSET(246))
#define MAP_llGetTask                              ((taskInfo_t *    (*) (uint8))                                                                                        R2R_JT_OFFSET(247))
#define MAP_llGetTaskState                         ((uint8           (*) (uint8))                                                                                        R2R_JT_OFFSET(248))
#define MAP_llGetTxPower                           ((int8            (*) (void))                                                                                         R2R_JT_OFFSET(249))
#define MAP_llGtSixConsecZerosOrOnes               ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(250))
#define MAP_llGtTwentyFourTransitions              ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(251))
#define MAP_llHaltRadio                            ((void            (*) (uint16))                                                                                       R2R_JT_OFFSET(252))
#define MAP_llInitFeatureSet                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(253))
#define MAP_llInitRAT                              ((void            (*) (void))                                                                                         R2R_JT_OFFSET(254))
#define MAP_llInit_TaskConnect                     ((void            (*) (void))                                                                                         R2R_JT_OFFSET(255))
#define MAP_llInit_TaskEnd                         ((void            (*) (void))                                                                                         R2R_JT_OFFSET(256))
#define MAP_llLtTwoChangesInLastSixBits            ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(257))
#define MAP_llMaster_TaskEnd                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(258))
#define MAP_llMemCopyDst                           ((uint8 *         (*) (uint8 *, uint8 *, uint8))                                                                      R2R_JT_OFFSET(259))
#define MAP_llMemCopySrc                           ((uint8 *         (*) (uint8 *, uint8 *, uint8))                                                                      R2R_JT_OFFSET(260))
#define MAP_llMoveTempTxDataEntries                ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(261))
#define MAP_llOneBitSynchWordDiffer                ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(262))
#define MAP_llPatchCM0                             ((void            (*) (void))                                                                                         R2R_JT_OFFSET(263))
#define MAP_llPendingUpdateParam                   ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(264))
#define MAP_llProcessChanMap                       ((void            (*) (llConnState_t *, uint8 *))                                                                     R2R_JT_OFFSET(265))
#define MAP_llProcessMasterControlPacket           ((void            (*) (llConnState_t *, uint8 *))                                                                     R2R_JT_OFFSET(266))
#define MAP_llProcessMasterControlProcedures       ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(267))
#define MAP_llProcessTxData                        ((void            (*) (llConnState_t *, uint8))                                                                       R2R_JT_OFFSET(268))
#define MAP_llProcessPostRfOps                     ((void            (*) (void))                                                                                         R2R_JT_OFFSET(269))
#define MAP_llProcessScanRxFIFO                    ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(270))
#define MAP_llProcessSlaveControlPacket            ((void            (*) (llConnState_t *, uint8 *))                                                                     R2R_JT_OFFSET(271))
#define MAP_llProcessSlaveControlProcedures        ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(272))
#define MAP_llRatChanCBack_A                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(273))
#define MAP_llRatChanCBack_B                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(274))
#define MAP_llRatChanCBack_C                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(275))
#define MAP_llRatChanCBack_D                       ((void            (*) (void))                                                                                         R2R_JT_OFFSET(276))
#define MAP_llReleaseAllConnId                     ((void            (*) (void))                                                                                         R2R_JT_OFFSET(277))
#define MAP_llReleaseConnId                        ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(278))
#define MAP_llReplaceCtrlPkt                       ((void            (*) (llConnState_t *, uint8))                                                                       R2R_JT_OFFSET(279))
#define MAP_llResetRadio                           ((void            (*) (void))                                                                                         R2R_JT_OFFSET(280))
#define MAP_llRfInit                               ((void            (*) (void))                                                                                         R2R_JT_OFFSET(281))
#define MAP_llRfStartFS                            ((void            (*) (uint8, uint16))                                                                                R2R_JT_OFFSET(282))
#define MAP_llScan_TaskEnd                         ((void            (*) (void))                                                                                         R2R_JT_OFFSET(283))
#define MAP_llScheduleTask                         ((void            (*) (taskInfo_t *))                                                                                 R2R_JT_OFFSET(284))
#define MAP_llScheduler                            ((void            (*) (void))                                                                                         R2R_JT_OFFSET(285))
#define MAP_llSchedulerInit                        ((void            (*) (void))                                                                                         R2R_JT_OFFSET(286))
#define MAP_llSetFreqTune                          ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(287))
#define MAP_llSetNextDataChan                      ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(288))
#define MAP_llSetTxPower                           ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(289))
#define MAP_llSetupAdv                             ((void            (*) (void))                                                                                         R2R_JT_OFFSET(290))
#define MAP_llSetupAdvDataEntryQueue               ((dataEntryQ_t *  (*) (void))                                                                                         R2R_JT_OFFSET(291))
#define MAP_llSetupConn                            ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(292))
#define MAP_llSetupConnRxDataEntryQueue            ((dataEntryQ_t *  (*) (uint8))                                                                                        R2R_JT_OFFSET(293))
#define MAP_llSetupEncReq                          ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(294))
#define MAP_llSetupEncRsp                          ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(295))
#define MAP_llSetupFeatureSetReq                   ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(296))
#define MAP_llSetupFeatureSetRsp                   ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(297))
#define MAP_llSetupInit                            ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(298))
#define MAP_llSetupInitDataEntryQueue              ((dataEntryQ_t *  (*) (void))                                                                                         R2R_JT_OFFSET(299))
#define MAP_llSetupMailbox                         ((void            (*) (void))                                                                                         R2R_JT_OFFSET(300))
#define MAP_llSetupNextMasterEvent                 ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(301))
#define MAP_llSetupNextSlaveEvent                  ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(302))
#define MAP_llSetupPauseEncReq                     ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(303))
#define MAP_llSetupPauseEncRsp                     ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(304))
#define MAP_llSetupRATChanCompare                  ((void            (*) (uint8, uint32))                                                                                R2R_JT_OFFSET(305))
#define MAP_llSetupRejectInd                       ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(306))
#define MAP_llSetupRfHal                           ((void            (*) (void))                                                                                         R2R_JT_OFFSET(307))
#define MAP_llSetupScan                            ((void            (*) (void))                                                                                         R2R_JT_OFFSET(308))
#define MAP_llSetupScanDataEntryQueue              ((dataEntryQ_t *  (*) (void))                                                                                         R2R_JT_OFFSET(309))
#define MAP_llSetupStartEncReq                     ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(310))
#define MAP_llSetupStartEncRsp                     ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(311))
#define MAP_llSetupTermInd                         ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(312))
#define MAP_llSetupUnknownRsp                      ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(313))
#define MAP_llSetupUpdateChanReq                   ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(314))
#define MAP_llSetupUpdateParamReq                  ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(315))
#define MAP_llSetupVersionIndReq                   ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(316))
#define MAP_llSlave_TaskEnd                        ((void            (*) (void))                                                                                         R2R_JT_OFFSET(317))
#define MAP_llTimeCompare                          ((uint8           (*) (uint32, uint32))                                                                               R2R_JT_OFFSET(318))
#define MAP_llTimeDelta                            ((uint32          (*) (uint32, uint32))                                                                               R2R_JT_OFFSET(319))
#define MAP_llValidAccessAddr                      ((uint8           (*) (uint32))                                                                                       R2R_JT_OFFSET(320))
#define MAP_llWriteTxData                          ((uint8           (*) (llConnState_t *, uint8 *, uint8, uint8))                                                       R2R_JT_OFFSET(321))
#define MAP_llTaskError                            ((void            (*) (void))                                                                                         R2R_JT_OFFSET(322))
#define MAP_llHardwareError                        ((void            (*) (uint8))                                                                                        R2R_JT_OFFSET(323))
#define MAP_cpe0IntCback                           ((void            (*) (void))                                                                                         R2R_JT_OFFSET(324))
#define MAP_cpe1IntCback                           ((void            (*) (void))                                                                                         R2R_JT_OFFSET(325))
#define MAP_hwIntCback                             ((void            (*) (void))                                                                                         R2R_JT_OFFSET(326))
// PATCH
#define MAP_HCI_EXT_GetConnInfoCmd                 ((uint8           (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(327))
#define MAP_HCI_ReadAuthPayloadTimeoutCmd          ((uint8           (*) (uint16, uint16 *))                                                                             R2R_JT_OFFSET(328))
#define MAP_HCI_WriteAuthPayloadTimeoutCmd         ((uint8           (*) (uint16, uint16))                                                                               R2R_JT_OFFSET(329))
#define MAP_HCI_LE_RemoteConnParamReqReplyCmd      ((uint8           (*) (uint16, uint16, uint16, uint16, uint16, uint16, uint16))                                       R2R_JT_OFFSET(330))
#define MAP_HCI_LE_RemoteConnParamReqNegReplyCmd   ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(331))
#define MAP_LL_EXT_GetConnInfo                     ((uint8           (*) (uint8 *, uint8 *, uint8 *))                                                                    R2R_JT_OFFSET(332))
#define MAP_LL_ReadAuthPayloadTimeout              ((uint8           (*) (uint16, uint16 *))                                                                             R2R_JT_OFFSET(333))
#define MAP_LL_WriteAuthPayloadTimeout             ((uint8           (*) (uint16, uint16))                                                                               R2R_JT_OFFSET(334))
#define MAP_LL_AuthPayloadTimeoutExpiredCback      ((void            (*) (uint16))                                                                                       R2R_JT_OFFSET(335))
#define MAP_LL_RemoteConnParamReqReply             ((uint8           (*) (uint16, uint16, uint16, uint16, uint16, uint16, uint16))                                       R2R_JT_OFFSET(336))
#define MAP_LL_RemoteConnParamReqNegReply          ((uint8           (*) (uint16, uint8))                                                                                R2R_JT_OFFSET(337))
#define MAP_LL_RemoteConnParamReqCback             ((void            (*) (uint16, uint16, uint16, uint16, uint16))                                                       R2R_JT_OFFSET(338))
#define MAP_llSetupPingReq                         ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(339))
#define MAP_llSetupPingRsp                         ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(340))
#define MAP_llSetupConnParamReq                    ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(341))
#define MAP_llCBTimer_AptoExpiredCback             ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(342))
#define MAP_llFragmentPDU                          ((uint8           (*) (llConnState_t *, uint8 *, uint16))                                                             R2R_JT_OFFSET(343))
#define MAP_llCombinePDU                           ((void            (*) (uint16, uint8 *, uint16, uint8))                                                               R2R_JT_OFFSET(344))
#define MAP_llSetupRejectIndExt                    ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(345))
#define MAP_llVerifyConnParamReqParams             ((void            (*) (uint16, uint16, uint16, uint8, uint16, uint16 *))                                              R2R_JT_OFFSET(346))
#define MAP_llSetupConnParamRsp                    ((uint8           (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(347))
#define MAP_llAlignToNextEvent                     ((void            (*) (llConnState_t *))                                                                              R2R_JT_OFFSET(348))
#define MAP_llGetTotalNumTxDataEntries             ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(349))
#define MAP_hciInitEventMasks                      ((uint8           (*) (void))                                                                                         R2R_JT_OFFSET(350))
#define MAP_HCI_SetEventMaskPage2Cmd               ((uint8           (*) (uint8 *))                                                                                      R2R_JT_OFFSET(351))
#define MAP_llRealignConn                          ((void            (*) (llConnState_t *, uint32))                                                                      R2R_JT_OFFSET(352))
#define MAP_llSortActiveConns                      ((void            (*) (uint8 *, uint8))                                                                               R2R_JT_OFFSET(353))
#define MAP_LL_GetNumActiveConns                   ((void            (*) (uint8 *))                                                                                      R2R_JT_OFFSET(354))
#define MAP_llConnExists                           ((void            (*) (uint8, uint8 *, uint8))                                                                        R2R_JT_OFFSET(355))
// V5.0
#define MAP_HCI_LE_ReadPhyCmd                      ((uint8           (*) (uint16))                                                                                       R2R_JT_OFFSET(356))
#define MAP_HCI_LE_SetDefaultPhyCmd                ((uint8           (*) (uint8, uint8, uint8))                                                                          R2R_JT_OFFSET(357))
#define MAP_HCI_LE_SetPhyCmd                       ((uint8           (*) (uint16, uint8, uint8, uint8))                                                                  R2R_JT_OFFSET(358))
#define MAP_HCI_LE_EnhancedRxTestCmd               ((uint8           (*) (uint8, uint8, uint8))                                                                          R2R_JT_OFFSET(359))
#define MAP_HCI_LE_EnhancedTxTestCmd               ((uint8           (*) (uint8, uint8, uint8, uint8))                                                                   R2R_JT_OFFSET(360))
//
#define MAP_LL_ReadPhy                             ((uint8           (*) (uint16, uint8 *, uint8 *))                                                                     R2R_JT_OFFSET(361))
#define MAP_LL_SetDefaultPhy                       ((uint8           (*) (uint8, uint8, uint8))                                                                          R2R_JT_OFFSET(362))
#define MAP_LL_SetPhy                              ((uint8           (*) (uint16, uint8, uint8, uint8))                                                                  R2R_JT_OFFSET(363))
#define MAP_LL_PhyUpdateCompleteEventCback         ((void            (*) (uint8, uint16, uint8, uint8))                                                                  R2R_JT_OFFSET(364))
#define MAP_LL_EnhancedRxTest                      ((uint8           (*) (uint8, uint8, uint8))                                                                          R2R_JT_OFFSET(365))
#define MAP_LL_EnhancedTxTest                      ((uint8           (*) (uint8, uint8, uint8, uint8))                                                                   R2R_JT_OFFSET(366))
//
#define MAP_llSetupPhyCtrlPkt                      ((uint8           (*) (llConnState_t *, uint8))                                                                       R2R_JT_OFFSET(367))
#define MAP_llGetRfOverrideRegs                    ((regOverride_t * (*) (void))                                                                                         R2R_JT_OFFSET(368))
#define MAP_llSendReject                           ((void            (*) (llConnState_t *, uint8, uint8))                                                                R2R_JT_OFFSET(369))

#else // No R2R Flash JT

// HCI ROM-to-ROM Functions
#define MAP_HCI_CommandCompleteEvent               HCI_CommandCompleteEvent
#define MAP_HCI_CommandStatusEvent                 HCI_CommandStatusEvent
#define MAP_HCI_DataBufferOverflowEvent            HCI_DataBufferOverflowEvent
#define MAP_HCI_DisconnectCmd                      HCI_DisconnectCmd
#define MAP_HCI_EXT_AdvEventNoticeCmd              HCI_EXT_AdvEventNoticeCmd
#define MAP_HCI_EXT_BuildRevisionCmd               HCI_EXT_BuildRevisionCmd
#define MAP_HCI_EXT_ClkDivOnHaltCmd                HCI_EXT_ClkDivOnHaltCmd
#define MAP_HCI_EXT_ConnEventNoticeCmd             HCI_EXT_ConnEventNoticeCmd
#define MAP_HCI_EXT_DeclareNvUsageCmd              HCI_EXT_DeclareNvUsageCmd
#define MAP_HCI_EXT_DecryptCmd                     HCI_EXT_DecryptCmd
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
#define MAP_HCI_EXT_DelaySleepCmd                  HCI_EXT_DelaySleepCmd
#define MAP_HCI_EXT_DisconnectImmedCmd             HCI_EXT_DisconnectImmedCmd
#define MAP_HCI_EXT_EnablePTMCmd                   HCI_EXT_EnablePTMCmd
#define MAP_HCI_EXT_EndModemTestCmd                HCI_EXT_EndModemTestCmd
#define MAP_HCI_EXT_ExtendRfRangeCmd               HCI_EXT_ExtendRfRangeCmd
#define MAP_HCI_EXT_HaltDuringRfCmd                HCI_EXT_HaltDuringRfCmd
#define MAP_HCI_EXT_MapPmIoPortCmd                 HCI_EXT_MapPmIoPortCmd
#define MAP_HCI_EXT_ModemHopTestTxCmd              HCI_EXT_ModemHopTestTxCmd
#define MAP_HCI_EXT_ModemTestRxCmd                 HCI_EXT_ModemTestRxCmd
#define MAP_HCI_EXT_ModemTestTxCmd                 HCI_EXT_ModemTestTxCmd
#define MAP_HCI_EXT_NumComplPktsLimitCmd           HCI_EXT_NumComplPktsLimitCmd
#define MAP_HCI_EXT_OnePktPerEvtCmd                HCI_EXT_OnePktPerEvtCmd
#define MAP_HCI_EXT_OverlappedProcessingCmd        HCI_EXT_OverlappedProcessingCmd
#define MAP_HCI_EXT_PERbyChanCmd                   HCI_EXT_PERbyChanCmd
#define MAP_HCI_EXT_PacketErrorRateCmd             HCI_EXT_PacketErrorRateCmd
#define MAP_HCI_EXT_ResetSystemCmd                 HCI_EXT_ResetSystemCmd
#define MAP_HCI_EXT_SaveFreqTuneCmd                HCI_EXT_SaveFreqTuneCmd
#define MAP_HCI_EXT_SetBDADDRCmd                   HCI_EXT_SetBDADDRCmd
#define MAP_HCI_EXT_SetFastTxResponseTimeCmd       HCI_EXT_SetFastTxResponseTimeCmd
#define MAP_HCI_EXT_SetFreqTuneCmd                 HCI_EXT_SetFreqTuneCmd
#define MAP_HCI_EXT_SetLocalSupportedFeaturesCmd   HCI_EXT_SetLocalSupportedFeaturesCmd
#define MAP_HCI_EXT_SetMaxDtmTxPowerCmd            HCI_EXT_SetMaxDtmTxPowerCmd
#define MAP_HCI_EXT_SetRxGainCmd                   HCI_EXT_SetRxGainCmd
#define MAP_HCI_EXT_SetSCACmd                      HCI_EXT_SetSCACmd
#define MAP_HCI_EXT_SetSlaveLatencyOverrideCmd     HCI_EXT_SetSlaveLatencyOverrideCmd
#define MAP_HCI_EXT_SetTxPowerCmd                  HCI_EXT_SetTxPowerCmd
#define MAP_HCI_HardwareErrorEvent                 HCI_HardwareErrorEvent
#define MAP_HCI_HostBufferSizeCmd                  HCI_HostBufferSizeCmd
#define MAP_HCI_HostNumCompletedPktCmd             HCI_HostNumCompletedPktCmd
#define MAP_HCI_LE_AddWhiteListCmd                 HCI_LE_AddWhiteListCmd
#define MAP_HCI_LE_ClearWhiteListCmd               HCI_LE_ClearWhiteListCmd
#define MAP_HCI_LE_ConnUpdateCmd                   HCI_LE_ConnUpdateCmd
#define MAP_HCI_LE_CreateConnCancelCmd             HCI_LE_CreateConnCancelCmd
#define MAP_HCI_LE_CreateConnCmd                   HCI_LE_CreateConnCmd
#define MAP_HCI_LE_EncryptCmd                      HCI_LE_EncryptCmd
#define MAP_HCI_LE_LtkReqNegReplyCmd               HCI_LE_LtkReqNegReplyCmd
#define MAP_HCI_LE_LtkReqReplyCmd                  HCI_LE_LtkReqReplyCmd
#define MAP_HCI_LE_RandCmd                         HCI_LE_RandCmd
#define MAP_HCI_LE_ReadAdvChanTxPowerCmd           HCI_LE_ReadAdvChanTxPowerCmd
#define MAP_HCI_LE_ReadBufSizeCmd                  HCI_LE_ReadBufSizeCmd
#define MAP_HCI_LE_ReadChannelMapCmd               HCI_LE_ReadChannelMapCmd
#define MAP_HCI_LE_ReadLocalSupportedFeaturesCmd   HCI_LE_ReadLocalSupportedFeaturesCmd
#define MAP_HCI_LE_ReadRemoteUsedFeaturesCmd       HCI_LE_ReadRemoteUsedFeaturesCmd
#define MAP_HCI_LE_ReadSupportedStatesCmd          HCI_LE_ReadSupportedStatesCmd
#define MAP_HCI_LE_ReadWhiteListSizeCmd            HCI_LE_ReadWhiteListSizeCmd
#define MAP_HCI_LE_ReceiverTestCmd                 HCI_LE_ReceiverTestCmd
#define MAP_HCI_LE_RemoveWhiteListCmd              HCI_LE_RemoveWhiteListCmd
#define MAP_HCI_LE_SetAdvDataCmd                   HCI_LE_SetAdvDataCmd
#define MAP_HCI_LE_SetAdvEnableCmd                 HCI_LE_SetAdvEnableCmd
#define MAP_HCI_LE_SetAdvParamCmd                  HCI_LE_SetAdvParamCmd
#define MAP_HCI_LE_SetEventMaskCmd                 HCI_LE_SetEventMaskCmd
#define MAP_HCI_LE_SetHostChanClassificationCmd    HCI_LE_SetHostChanClassificationCmd
#define MAP_HCI_LE_SetRandomAddressCmd             HCI_LE_SetRandomAddressCmd
#define MAP_HCI_LE_SetScanEnableCmd                HCI_LE_SetScanEnableCmd
#define MAP_HCI_LE_SetScanParamCmd                 HCI_LE_SetScanParamCmd
#define MAP_HCI_LE_SetScanRspDataCmd               HCI_LE_SetScanRspDataCmd
#define MAP_HCI_LE_StartEncyptCmd                  HCI_LE_StartEncyptCmd
#define MAP_HCI_LE_TestEndCmd                      HCI_LE_TestEndCmd
#define MAP_HCI_LE_TransmitterTestCmd              HCI_LE_TransmitterTestCmd
#define MAP_HCI_NumOfCompletedPacketsEvent         HCI_NumOfCompletedPacketsEvent
#define MAP_HCI_ReadBDADDRCmd                      HCI_ReadBDADDRCmd
#define MAP_HCI_ReadLocalSupportedCommandsCmd      HCI_ReadLocalSupportedCommandsCmd
// ROM WORKAROUND - Remove. This is not a BLE command.
#define MAP_HCI_ReadLocalSupportedFeaturesCmd      HCI_ReadLocalSupportedFeaturesCmd
#define MAP_HCI_ReadLocalVersionInfoCmd            HCI_ReadLocalVersionInfoCmd
#define MAP_HCI_ReadRemoteVersionInfoCmd           HCI_ReadRemoteVersionInfoCmd
#define MAP_HCI_ReadRssiCmd                        HCI_ReadRssiCmd
#define MAP_HCI_ReadTransmitPowerLevelCmd          HCI_ReadTransmitPowerLevelCmd
#define MAP_HCI_ResetCmd                           HCI_ResetCmd
#define MAP_HCI_ReverseBytes                       HCI_ReverseBytes
#define MAP_HCI_SendCommandCompleteEvent           HCI_SendCommandCompleteEvent
#define MAP_HCI_SendCommandStatusEvent             HCI_SendCommandStatusEvent
#define MAP_HCI_SendControllerToHostEvent          HCI_SendControllerToHostEvent
#define MAP_HCI_SendDataPkt                        HCI_SendDataPkt
#define MAP_HCI_SetControllerToHostFlowCtrlCmd     HCI_SetControllerToHostFlowCtrlCmd
#define MAP_HCI_SetEventMaskCmd                    HCI_SetEventMaskCmd
#define MAP_HCI_ValidConnTimeParams                HCI_ValidConnTimeParams
#define MAP_HCI_VendorSpecifcCommandCompleteEvent  HCI_VendorSpecifcCommandCompleteEvent
#define MAP_HCI_bm_alloc                           HCI_bm_alloc

// LL ROM-to-ROM Functions
#define MAP_LL_AddWhiteListDevice                  LL_AddWhiteListDevice
#define MAP_LL_AdvReportCback                      LL_AdvReportCback
#define MAP_LL_ChanMapUpdate                       LL_ChanMapUpdate
#define MAP_LL_ClearWhiteList                      LL_ClearWhiteList
#define MAP_LL_ConnActive                          LL_ConnActive
#define MAP_LL_ConnParamUpdateCback                LL_ConnParamUpdateCback
#define MAP_LL_ConnUpdate                          LL_ConnUpdate
#define MAP_LL_ConnectionCompleteCback             LL_ConnectionCompleteCback
#define MAP_LL_CreateConn                          LL_CreateConn
#define MAP_LL_CreateConnCancel                    LL_CreateConnCancel
#define MAP_LL_CtrlToHostFlowControl               LL_CtrlToHostFlowControl
#define MAP_LL_DirectTestEnd                       LL_DirectTestEnd
#define MAP_LL_DirectTestEndDoneCback              LL_DirectTestEndDoneCback
#define MAP_LL_DirectTestRxTest                    LL_DirectTestRxTest
#define MAP_LL_DirectTestTxTest                    LL_DirectTestTxTest
#define MAP_LL_Disconnect                          LL_Disconnect
#define MAP_LL_DisconnectCback                     LL_DisconnectCback
#define MAP_LL_ENC_AES128_Decrypt                  LL_ENC_AES128_Decrypt
#define MAP_LL_ENC_AES128_Encrypt                  LL_ENC_AES128_Encrypt
#define MAP_LL_ENC_Decrypt                         LL_ENC_Decrypt
#define MAP_LL_ENC_DecryptMsg                      LL_ENC_DecryptMsg
#define MAP_LL_ENC_Encrypt                         LL_ENC_Encrypt
#define MAP_LL_ENC_EncryptMsg                      LL_ENC_EncryptMsg
#define MAP_LL_ENC_GenDeviceIV                     LL_ENC_GenDeviceIV
#define MAP_LL_ENC_GenDeviceSKD                    LL_ENC_GenDeviceSKD
#define MAP_LL_ENC_GenerateNonce                   LL_ENC_GenerateNonce
#define MAP_LL_ENC_GeneratePseudoRandNum           LL_ENC_GeneratePseudoRandNum
#define MAP_LL_ENC_GenerateTrueRandNum             LL_ENC_GenerateTrueRandNum
#define MAP_LL_ENC_Init                            LL_ENC_Init
#define MAP_LL_ENC_LoadKey                         LL_ENC_LoadKey
#define MAP_LL_ENC_ReverseBytes                    LL_ENC_ReverseBytes
#define MAP_LL_EXT_AdvEventNotice                  LL_EXT_AdvEventNotice
#define MAP_LL_EXT_BuildRevision                   LL_EXT_BuildRevision
#define MAP_LL_EXT_ClkDivOnHalt                    LL_EXT_ClkDivOnHalt
#define MAP_LL_EXT_ConnEventNotice                 LL_EXT_ConnEventNotice
#define MAP_LL_EXT_DeclareNvUsage                  LL_EXT_DeclareNvUsage
#define MAP_LL_EXT_Decrypt                         LL_EXT_Decrypt
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
#define MAP_LL_EXT_DelaySleep                      LL_EXT_DelaySleep
#define MAP_LL_EXT_DisconnectImmed                 LL_EXT_DisconnectImmed
#define MAP_LL_EXT_EndModemTest                    LL_EXT_EndModemTest
#define MAP_LL_EXT_ExtendRfRange                   LL_EXT_ExtendRfRange
#define MAP_LL_EXT_ExtendRfRangeCback              LL_EXT_ExtendRfRangeCback
#define MAP_LL_EXT_HaltDuringRf                    LL_EXT_HaltDuringRf
#define MAP_LL_EXT_MapPmIoPort                     LL_EXT_MapPmIoPort
#define MAP_LL_EXT_ModemHopTestTx                  LL_EXT_ModemHopTestTx
#define MAP_LL_EXT_ModemTestRx                     LL_EXT_ModemTestRx
#define MAP_LL_EXT_ModemTestTx                     LL_EXT_ModemTestTx
#define MAP_LL_EXT_NumComplPktsLimit               LL_EXT_NumComplPktsLimit
#define MAP_LL_EXT_OnePacketPerEvent               LL_EXT_OnePacketPerEvent
#define MAP_LL_EXT_OverlappedProcessing            LL_EXT_OverlappedProcessing
#define MAP_LL_EXT_PERbyChan                       LL_EXT_PERbyChan
#define MAP_LL_EXT_PacketErrorRate                 LL_EXT_PacketErrorRate
#define MAP_LL_EXT_PacketErrorRateCback            LL_EXT_PacketErrorRateCback
#define MAP_LL_EXT_ResetSystem                     LL_EXT_ResetSystem
#define MAP_LL_EXT_SaveFreqTune                    LL_EXT_SaveFreqTune
#define MAP_LL_EXT_SetBDADDR                       LL_EXT_SetBDADDR
#define MAP_LL_EXT_SetFastTxResponseTime           LL_EXT_SetFastTxResponseTime
#define MAP_LL_EXT_SetFreqTune                     LL_EXT_SetFreqTune
#define MAP_LL_EXT_SetLocalSupportedFeatures       LL_EXT_SetLocalSupportedFeatures
#define MAP_LL_EXT_SetMaxDtmTxPower                LL_EXT_SetMaxDtmTxPower
#define MAP_LL_EXT_SetRxGain                       LL_EXT_SetRxGain
#define MAP_LL_EXT_SetRxGainCback                  LL_EXT_SetRxGainCback
#define MAP_LL_EXT_SetSCA                          LL_EXT_SetSCA
#define MAP_LL_EXT_SetSlaveLatencyOverride         LL_EXT_SetSlaveLatencyOverride
#define MAP_LL_EXT_SetTxPower                      LL_EXT_SetTxPower
#define MAP_LL_EXT_SetTxPowerCback                 LL_EXT_SetTxPowerCback
#define MAP_LL_EncChangeCback                      LL_EncChangeCback
#define MAP_LL_EncKeyRefreshCback                  LL_EncKeyRefreshCback
#define MAP_LL_EncLtkNegReply                      LL_EncLtkNegReply
#define MAP_LL_EncLtkReply                         LL_EncLtkReply
#define MAP_LL_EncLtkReqCback                      LL_EncLtkReqCback
#define MAP_LL_Encrypt                             LL_Encrypt
#define MAP_LL_Init                                LL_Init
#define MAP_LL_NumEmptyWlEntries                   LL_NumEmptyWlEntries
#define MAP_LL_ProcessEvent                        LL_ProcessEvent
#define MAP_LL_PseudoRand                          LL_PseudoRand
#define MAP_LL_RX_bm_alloc                         LL_RX_bm_alloc
#define MAP_LL_Rand                                LL_Rand
#define MAP_LL_RandCback                           LL_RandCback
#define MAP_LL_ReadAdvChanTxPower                  LL_ReadAdvChanTxPower
#define MAP_LL_ReadBDADDR                          LL_ReadBDADDR
#define MAP_LL_ReadChanMap                         LL_ReadChanMap
#define MAP_LL_ReadLocalSupportedFeatures          LL_ReadLocalSupportedFeatures
#define MAP_LL_ReadLocalVersionInfo                LL_ReadLocalVersionInfo
#define MAP_LL_ReadRemoteUsedFeatures              LL_ReadRemoteUsedFeatures
#define MAP_LL_ReadRemoteUsedFeaturesCompleteCback LL_ReadRemoteUsedFeaturesCompleteCback
#define MAP_LL_ReadRemoteVersionInfo               LL_ReadRemoteVersionInfo
#define MAP_LL_ReadRemoteVersionInfoCback          LL_ReadRemoteVersionInfoCback
#define MAP_LL_ReadRssi                            LL_ReadRssi
#define MAP_LL_ReadSupportedStates                 LL_ReadSupportedStates
#define MAP_LL_ReadTxPowerLevel                    LL_ReadTxPowerLevel
#define MAP_LL_ReadWlSize                          LL_ReadWlSize
#define MAP_LL_RemoveWhiteListDevice               LL_RemoveWhiteListDevice
#define MAP_LL_Reset                               LL_Reset
#define MAP_LL_RxDataCompleteCback                 LL_RxDataCompleteCback
#define MAP_LL_SetAdvControl                       LL_SetAdvControl
#define MAP_LL_SetAdvData                          LL_SetAdvData
#define MAP_LL_SetAdvParam                         LL_SetAdvParam
#define MAP_LL_SetRandomAddress                    LL_SetRandomAddress
#define MAP_LL_SetScanControl                      LL_SetScanControl
#define MAP_LL_SetScanParam                        LL_SetScanParam
#define MAP_LL_SetScanRspData                      LL_SetScanRspData
#define MAP_LL_StartEncrypt                        LL_StartEncrypt
#define MAP_LL_TX_bm_alloc                         LL_TX_bm_alloc
#define MAP_LL_TxData                              LL_TxData
#define MAP_WL_AddEntry                            WL_AddEntry
#define MAP_WL_Clear                               WL_Clear
#define MAP_WL_ClearEntry                          WL_ClearEntry
#define MAP_WL_ClearIgnoreList                     WL_ClearIgnoreList
#define MAP_WL_FindEntry                           WL_FindEntry
#define MAP_WL_GetNumFreeEntries                   WL_GetNumFreeEntries
#define MAP_WL_GetSize                             WL_GetSize
#define MAP_WL_Init                                WL_Init
#define MAP_WL_RemoveEntry                         WL_RemoveEntry
#define MAP_WL_SetWlIgnore                         WL_SetWlIgnore
#define MAP_llActiveTask                           llActiveTask
#define MAP_llAdv_TaskAbort                        llAdv_TaskAbort
#define MAP_llAdv_TaskConnect                      llAdv_TaskConnect
#define MAP_llAdv_TaskEnd                          llAdv_TaskEnd
#define MAP_llAllocConnId                          llAllocConnId
#define MAP_llAllocTask                            llAllocTask
#define MAP_llAtLeastTwoChans                      llAtLeastTwoChans
#define MAP_llCalcScaFactor                        llCalcScaFactor
#define MAP_llCheckForLstoDuringSL                 llCheckForLstoDuringSL
#define MAP_llCheckWhiteListUsage                  llCheckWhiteListUsage
#define MAP_llClearRfInts                          llClearRfInts
#define MAP_llConnCleanup                          llConnCleanup
#define MAP_llConnTerminate                        llConnTerminate
#define MAP_llConvertCtrlProcTimeoutToEvent        llConvertCtrlProcTimeoutToEvent
#define MAP_llConvertLstoToEvent                   llConvertLstoToEvent
#define MAP_llDataQEmpty                           llDataQEmpty
#define MAP_llDataQFull                            llDataQFull
#define MAP_llDequeueCtrlPkt                       llDequeueCtrlPkt
#define MAP_llDequeueDataQ                         llDequeueDataQ
#define MAP_llDirAdv_TaskEnd                       llDirAdv_TaskEnd
#define MAP_llDisableRfInts                        llDisableRfInts
#define MAP_llEnableRfInts                         llEnableRfInts
#define MAP_llEnqueueCtrlPkt                       llEnqueueCtrlPkt
#define MAP_llEnqueueDataQ                         llEnqueueDataQ
#define MAP_llEnqueueHeadDataQ                     llEnqueueHeadDataQ
#define MAP_llEqAlreadyValidAddr                   llEqAlreadyValidAddr
#define MAP_llEqSynchWord                          llEqSynchWord
#define MAP_llEqualBytes                           llEqualBytes
#define MAP_llEventDelta                           llEventDelta
#define MAP_llEventInRange                         llEventInRange
#define MAP_llFindNextSecTask                      llFindNextSecTask
#define MAP_llFindStartType                        llFindStartType
#define MAP_llFreeTask                             llFreeTask
#define MAP_llGenerateCRC                          llGenerateCRC
#define MAP_llGenerateValidAccessAddr              llGenerateValidAccessAddr
#define MAP_llGetActiveTasks                       llGetActiveTasks
#define MAP_llGetAdvChanPDU                        llGetAdvChanPDU
#define MAP_llGetCurrentTask                       llGetCurrentTask
#define MAP_llGetCurrentTime                       llGetCurrentTime
#define MAP_llGetMinCI                             llGetMinCI
#define MAP_llGetNextConn                          llGetNextConn
#define MAP_llGetNextDataChan                      llGetNextDataChan
#define MAP_llGetNumTasks                          llGetNumTasks
#define MAP_llGetNumTxDataEntries                  llGetNumTxDataEntries
#define MAP_llGetTask                              llGetTask
#define MAP_llGetTaskState                         llGetTaskState
#define MAP_llGetTxPower                           llGetTxPower
#define MAP_llGtSixConsecZerosOrOnes               llGtSixConsecZerosOrOnes
#define MAP_llGtTwentyFourTransitions              llGtTwentyFourTransitions
#define MAP_llHaltRadio                            llHaltRadio
#define MAP_llInitFeatureSet                       llInitFeatureSet
#define MAP_llInitRAT                              llInitRAT
#define MAP_llInit_TaskConnect                     llInit_TaskConnect
#define MAP_llInit_TaskEnd                         llInit_TaskEnd
#define MAP_llLtTwoChangesInLastSixBits            llLtTwoChangesInLastSixBits
#define MAP_llMaster_TaskEnd                       llMaster_TaskEnd
#define MAP_llMemCopyDst                           llMemCopyDst
#define MAP_llMemCopySrc                           llMemCopySrc
#define MAP_llMoveTempTxDataEntries                llMoveTempTxDataEntries
#define MAP_llOneBitSynchWordDiffer                llOneBitSynchWordDiffer
#define MAP_llPatchCM0                             llPatchCM0
#define MAP_llPendingUpdateParam                   llPendingUpdateParam
#define MAP_llProcessChanMap                       llProcessChanMap
#define MAP_llProcessMasterControlPacket           llProcessMasterControlPacket
#define MAP_llProcessMasterControlProcedures       llProcessMasterControlProcedures
#define MAP_llProcessTxData                        llProcessTxData
#define MAP_llProcessPostRfOps                     llProcessPostRfOps
#define MAP_llProcessScanRxFIFO                    llProcessScanRxFIFO
#define MAP_llProcessSlaveControlPacket            llProcessSlaveControlPacket
#define MAP_llProcessSlaveControlProcedures        llProcessSlaveControlProcedures
#define MAP_llRatChanCBack_A                       llRatChanCBack_A
#define MAP_llRatChanCBack_B                       llRatChanCBack_B
#define MAP_llRatChanCBack_C                       llRatChanCBack_C
#define MAP_llRatChanCBack_D                       llRatChanCBack_D
#define MAP_llReleaseAllConnId                     llReleaseAllConnId
#define MAP_llReleaseConnId                        llReleaseConnId
#define MAP_llReplaceCtrlPkt                       llReplaceCtrlPkt
#define MAP_llResetRadio                           llResetRadio
#define MAP_llRfInit                               llRfInit
#define MAP_llRfStartFS                            llRfStartFS
#define MAP_llScan_TaskEnd                         llScan_TaskEnd
#define MAP_llScheduleTask                         llScheduleTask
#define MAP_llScheduler                            llScheduler
#define MAP_llSchedulerInit                        llSchedulerInit
#define MAP_llSetFreqTune                          llSetFreqTune
#define MAP_llSetNextDataChan                      llSetNextDataChan
#define MAP_llSetTxPower                           llSetTxPower
#define MAP_llSetupAdv                             llSetupAdv
#define MAP_llSetupAdvDataEntryQueue               llSetupAdvDataEntryQueue
#define MAP_llSetupConn                            llSetupConn
#define MAP_llSetupConnRxDataEntryQueue            llSetupConnRxDataEntryQueue
#define MAP_llSetupEncReq                          llSetupEncReq
#define MAP_llSetupEncRsp                          llSetupEncRsp
#define MAP_llSetupFeatureSetReq                   llSetupFeatureSetReq
#define MAP_llSetupFeatureSetRsp                   llSetupFeatureSetRsp
#define MAP_llSetupInit                            llSetupInit
#define MAP_llSetupInitDataEntryQueue              llSetupInitDataEntryQueue
#define MAP_llSetupMailbox                         llSetupMailbox
#define MAP_llSetupNextMasterEvent                 llSetupNextMasterEvent
#define MAP_llSetupNextSlaveEvent                  llSetupNextSlaveEvent
#define MAP_llSetupPauseEncReq                     llSetupPauseEncReq
#define MAP_llSetupPauseEncRsp                     llSetupPauseEncRsp
#define MAP_llSetupRATChanCompare                  llSetupRATChanCompare
#define MAP_llSetupRejectInd                       llSetupRejectInd
#define MAP_llSetupRfHal                           llSetupRfHal
#define MAP_llSetupScan                            llSetupScan
#define MAP_llSetupScanDataEntryQueue              llSetupScanDataEntryQueue
#define MAP_llSetupStartEncReq                     llSetupStartEncReq
#define MAP_llSetupStartEncRsp                     llSetupStartEncRsp
#define MAP_llSetupTermInd                         llSetupTermInd
#define MAP_llSetupUnknownRsp                      llSetupUnknownRsp
#define MAP_llSetupUpdateChanReq                   llSetupUpdateChanReq
#define MAP_llSetupUpdateParamReq                  llSetupUpdateParamReq
#define MAP_llSetupVersionIndReq                   llSetupVersionIndReq
#define MAP_llSlave_TaskEnd                        llSlave_TaskEnd
#define MAP_llTimeCompare                          llTimeCompare
#define MAP_llTimeDelta                            llTimeDelta
#define MAP_llValidAccessAddr                      llValidAccessAddr
#define MAP_llWriteTxData                          llWriteTxData
#define MAP_llTaskError                            llTaskError
#define MAP_llHardwareError                        llHardwareError
#define MAP_cpe0IntCback                           cpe0IntCback
#define MAP_cpe1IntCback                           cpe1IntCback
#define MAP_hwIntCback                             hwIntCback
// PATCH
#define MAP_HCI_EXT_GetConnInfoCmd                 HCI_EXT_GetConnInfoCmd
#define MAP_HCI_ReadAuthPayloadTimeoutCmd          HCI_ReadAuthPayloadTimeoutCmd
#define MAP_HCI_WriteAuthPayloadTimeoutCmd         HCI_WriteAuthPayloadTimeoutCmd
#define MAP_HCI_LE_RemoteConnParamReqReplyCmd      HCI_LE_RemoteConnParamReqReplyCmd
#define MAP_HCI_LE_RemoteConnParamReqNegReplyCmd   HCI_LE_RemoteConnParamReqNegReplyCmd
#define MAP_LL_EXT_GetConnInfo                     LL_EXT_GetConnInfo
#define MAP_LL_ReadAuthPayloadTimeout              LL_ReadAuthPayloadTimeout
#define MAP_LL_WriteAuthPayloadTimeout             LL_WriteAuthPayloadTimeout
#define MAP_LL_AuthPayloadTimeoutExpiredCback      LL_AuthPayloadTimeoutExpiredCback
#define MAP_LL_RemoteConnParamReqReply             LL_RemoteConnParamReqReply
#define MAP_LL_RemoteConnParamReqNegReply          LL_RemoteConnParamReqNegReply
#define MAP_LL_RemoteConnParamReqCback             LL_RemoteConnParamReqCback
#define MAP_llSetupPingReq                         llSetupPingReq
#define MAP_llSetupPingRsp                         llSetupPingRsp
#define MAP_llSetupConnParamReq                    llSetupConnParamReq
#define MAP_llCBTimer_AptoExpiredCback             llCBTimer_AptoExpiredCback
#define MAP_llFragmentPDU                          llFragmentPDU
#define MAP_llCombinePDU                           llCombinePDU
#define MAP_llSetupRejectIndExt                    llSetupRejectIndExt
#define MAP_llVerifyConnParamReqParams             llVerifyConnParamReqParams
#define MAP_llSetupConnParamRsp                    llSetupConnParamRsp
#define MAP_llAlignToNextEvent                     llAlignToNextEvent
#define MAP_llGetTotalNumTxDataEntries             llGetTotalNumTxDataEntries
#define MAP_hciInitEventMasks                      hciInitEventMasks
#define MAP_HCI_SetEventMaskPage2Cmd               HCI_SetEventMaskPage2Cmd
#define MAP_llRealignConn                          llRealignConn
#define MAP_llSortActiveConns                      llSortActiveConns
#define MAP_LL_GetNumActiveConns                   LL_GetNumActiveConns
#define MAP_llConnExists                           llConnExists
// V5.0
#define MAP_HCI_LE_ReadPhyCmd                      HCI_LE_ReadPhyCmd
#define MAP_HCI_LE_SetDefaultPhyCmd                HCI_LE_SetDefaultPhyCmd
#define MAP_HCI_LE_SetPhyCmd                       HCI_LE_SetPhyCmd
#define MAP_HCI_LE_EnhancedRxTestCmd               HCI_LE_EnhancedRxTestCmd
#define MAP_HCI_LE_EnhancedTxTestCmd               HCI_LE_EnhancedTxTestCmd
//
#define MAP_LL_ReadPhy                             LL_ReadPhy
#define MAP_LL_SetDefaultPhy                       LL_SetDefaultPhy
#define MAP_LL_SetPhy                              LL_SetPhy
#define MAP_LL_PhyUpdateCompleteEventCback         LL_PhyUpdateCompleteEventCback
#define MAP_LL_EnhancedRxTest                      LL_EnhancedRxTest
#define MAP_LL_EnhancedTxTest                      LL_EnhancedTxTest
//
#define MAP_llSetupPhyCtrlPkt                      llSetupPhyCtrlPkt
#define MAP_llGetRfOverrideRegs                    llGetRfOverrideRegs
#define MAP_llSendReject                           llSendReject

#endif // ROM_BUILD

#endif /* R2R_FLASH_JT_H */

