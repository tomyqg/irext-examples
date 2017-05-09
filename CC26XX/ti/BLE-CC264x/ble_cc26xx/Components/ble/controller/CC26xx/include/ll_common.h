/*******************************************************************************
  Filename:       ll_common.h
  Revised:        $Date: 2012-02-15 14:12:20 -0800 (Wed, 15 Feb 2012) $
  Revision:       $Revision: 29309 $

  Description:    This file contains the Link Layer (LL) types, constants,
                  API's etc. for the Bluetooth Low Energy (ULE) Controller that
                  are internally common to LL routines.

  Copyright 2009-2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef LL_COMMON_H
#define LL_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

// TEMP
#define NEAR_FUNC


/*******************************************************************************
 * INCLUDES
 */

#include "OSAL.h"
#include "ll.h"
#include "ll_scheduler.h"
#include "hal_assert.h"
#include "rfHal.h"

#ifdef LL_COLLECT_METRICS
#include "ll_metric.h"
#endif // LL_COLLECT_METRICS

// four ports allowed: 1, 2, 3, 4
#ifdef DEBUG_IJET_SWO
#define ITM_Port32(n) (*((volatile unsigned int *)(0xE0000000+4*n)))
#endif // DEBUG_IJET_SWO

/*******************************************************************************
 * MACROS
 */

// Note: These macros assume the packet header has already been masked with
//       LL_DATA_PDU_HDR_LLID_MASK.
#define LL_DATA_PDU( pktHdr )     ((pktHdr) != LL_DATA_PDU_HDR_LLID_CONTROL_PKT)
#define LL_CTRL_PDU( pktHdr )     ((pktHdr) == LL_DATA_PDU_HDR_LLID_CONTROL_PKT)
#define LL_INVALID_LLID( pktHdr ) ((pktHdr) == LL_DATA_PDU_HDR_LLID_RESERVED)

// CONNECT_REQ Header LSO
#define LL_CONNECT_REQ_HDR_GET_TYPE( val ) ((val) & 0x07)

// local ASSERT handler
#if defined( DEBUG )
#define LL_ASSERT(cond) {volatile uint8 i = (cond); while(!i);}
#else // !DEBUG
// Note: Use HALNODEBUG to eliminate HAL assert handling (i.e. no assert).
// Note: If HALNODEBUG is not used, use ASSERT_RESET to reset system on assert.
//       Otherwise, evaluation board hazard lights are used.
// Note: Unused input parameter possible when HALNODEBUG; PC-Lint error 715.
#define LL_ASSERT(cond) HAL_ASSERT(cond)
#endif // DEBUG

// RSSI Correction

#define ADI0_TRIM_MASK                           BV(5)

#define GET_RSSI_OFFSET()                                                      \
  ((*((uint32 *)(FCFG1_BASE + LL_RSSI_OFFSET))>>9) & 0xFF)

#define GET_RSSI_CORRECTION( rfConfig )                                        \
  ((rfConfig) & ADI0_TRIM_MASK) ? (int8)GET_RSSI_OFFSET() : (int8)0

// checks if RSSI is valid - returns boolean
#define LL_CHECK_RSSI_VALID( rssi )                                            \
          ((rssi) == LL_RF_RSSI_UNDEFINED || (rssi) == LL_RF_RSSI_INVALID)) ?  \
          TRUE                                                              :  \
          FALSE

// corrects RSSI if valid, otherwise returns not available
// Note: Input is uint8, output int8.
#define LL_CHECK_LAST_RSSI( rssi )                                             \
          ((rssi) == LL_RF_RSSI_UNDEFINED || (rssi) == LL_RF_RSSI_INVALID)  ?  \
          (int8)LL_RSSI_NOT_AVAILABLE                                       :  \
          ((int8)(rssi) - rssiCorrection)

#define CHECK_CRITICAL_SECTION() (__get_BASEPRI() & 0x20 )

#define LL_CMP_BDADDR( dstPtr, srcPtr )                                        \
  ( ((dstPtr)[0] == (srcPtr)[0]) &&                                            \
    ((dstPtr)[1] == (srcPtr)[1]) &&                                            \
    ((dstPtr)[2] == (srcPtr)[2]) &&                                            \
    ((dstPtr)[3] == (srcPtr)[3]) &&                                            \
    ((dstPtr)[4] == (srcPtr)[4]) &&                                            \
    ((dstPtr)[5] == (srcPtr)[5]) )


/*
** Control Procedure Macros
*/
#define SET_FEATURE_FLAG( flags, flag ) ((flags) |= (flag))
#define TST_FEATURE_FLAG( flags, flag ) ((flags) & (flag))
#define CLR_FEATURE_FLAG( flags, flag ) ((flags) &= ~(flag))

#define ONLY_ONE_BIT_SET( x ) (((x) != 0) && !((x) & ((x)-1)))

#if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & PHY_2MBPS_CFG)
#define LL_GET_RF_OVERRIDE_REGS_PTR      MAP_llGetRfOverrideRegs()
#else // !PHY_2MBPS_CFG
#define LL_GET_RF_OVERRIDE_REGS_PTR      llConfigTable.userCfgPtr->rfRegPtr
#endif // PHY_2MBPS_CFG

/*******************************************************************************
 * CONSTANTS
 */

// Link Layer State
#define LL_STATE_IDLE                                  0x00
#define LL_STATE_ADV_UNDIRECTED                        0x01
#define LL_STATE_ADV_DIRECTED                          0x02
#define LL_STATE_ADV_DISCOVER                          0x03
#define LL_STATE_ADV_NONCONN                           0x04
#define LL_STATE_SCAN                                  0x05
#define LL_STATE_INIT                                  0x06
#define LL_STATE_CONN_SLAVE                            0x07
#define LL_STATE_CONN_MASTER                           0x08
#define LL_STATE_DIRECT_TEST_MODE_TX                   0x09
#define LL_STATE_DIRECT_TEST_MODE_RX                   0x0A
#define LL_STATE_MODEM_TEST_TX                         0x0B
#define LL_STATE_MODEM_TEST_RX                         0x0C
#define LL_STATE_MODEM_TEST_TX_FREQ_HOPPING            0x0D

// LL Events
#define LL_EVT_POST_PROCESS_RF                         0x0001
#define LL_EVT_DIRECTED_ADV_FAILED                     0x0002
#define LL_EVT_SLAVE_CONN_CREATED                      0x0004
#define LL_EVT_MASTER_CONN_CREATED                     0x0008
#define LL_EVT_MASTER_CONN_CANCELLED                   0x0010
#define LL_EVT_START_32KHZ_XOSC_DELAY                  0x0020
#define LL_EVT_32KHZ_XOSC_DELAY                        0x0040
#define LL_EVT_SLAVE_CONN_CREATED_BAD_PARAM            0x0080
#define LL_EVT_RX_BUFFER_CHECK                         0x0100
#define LL_EVT_RESET_SYSTEM_HARD                       0x0200
#define LL_EVT_RESET_SYSTEM_SOFT                       0x0400
#define LL_RESERVED1                                   0x0800
#define LL_RESERVED2                                   0x1000
#define LL_EVT_INIT_DONE                               0x2000
#define LL_EVT_OUT_OF_MEMORY                           0x4000
#define SYS_RESERVED                                   SYS_EVENT_MSG

// Hardware Failure Status
#define HW_FAIL_PAST_START_TRIG                        0x80
#define HW_FAIL_OUT_OF_MEMORY                          0x81
#define HW_FAIL_FW_INTERNAL_ERROR                      0x82
#define HW_FAIL_INVAILD_RF_COMMAND                     0x83
#define HW_FAIL_UNKNOWN_RF_STATUS                      0x84
#define HW_FAIL_UNEXPECTED_RF_STATUS                   0x85
#define HW_FAIL_UNKNOWN_LL_STATE                       0x86
#define HW_FAIL_FS_PROG_ERROR                          0x87
#define HW_FAIL_FS_FAIL_TO_START                       0x88
#define HW_FAIL_RF_INIT_ERROR                          0x89
#define HW_FAIL_PDU_SIZE_EXCEEDS_MTU                   0x8A
#define HW_FAIL_PKT_LEN_EXCEEDS_PDU_SIZE               0x8B
#define HW_FAIL_INADEQUATE_PKT_LEN                     0x8C
#define HW_FAIL_DISALLOWED_PHY_CHANGE                  0x8D
#define HW_FAIL_UNKNOWN_ERROR                          0xFF

/*
** Air Interface Packets
*/

// Packet Type
#define LL_PKT_EVT_TYPE_MASK                           0x0F
//
#define LL_PKT_TYPE_ADV_IND                            0
#define LL_PKT_TYPE_ADV_DIRECT_IND                     1
#define LL_PKT_TYPE_ADV_NONCONN_IND                    2
#define LL_PKT_TYPE_SCAN_REQ                           3
#define LL_PKT_TYPE_SCAN_RSP                           4
#define LL_PKT_TYPE_CONNECT_REQ                        5
#define LL_PKT_TYPE_ADV_SCAN_IND                       6
#define LL_PKT_TYPE_RESERVED                           7

// Packet Related Information
#define LL_PKT_PREAMBLE_LEN                            1
#define LL_PKT_SYNCH_LEN                               4
#define LL_PKT_LLID_LEN                                1
#define LL_PKT_HDR_LEN                                 2
#define LL_PKT_MIC_LEN                                 4
#define LL_PKT_CRC_LEN                                 3

// Payload sizes
// Note: For control packets, this is the Opcode + CtrData.
#define LL_CONNECT_REQ_PAYLOAD_LEN                     18
#define LL_CONN_UPDATE_REQ_PAYLOAD_LEN                 12
#define LL_CHAN_MAP_REQ_PAYLOAD_LEN                    8
#define LL_TERM_IND_PAYLOAD_LEN                        2
#define LL_ENC_REQ_PAYLOAD_LEN                         23
#define LL_ENC_RSP_PAYLOAD_LEN                         13
#define LL_START_ENC_REQ_PAYLOAD_LEN                   1
#define LL_START_ENC_RSP_PAYLOAD_LEN                   1
#define LL_PAUSE_ENC_REQ_PAYLOAD_LEN                   1
#define LL_PAUSE_ENC_RSP_PAYLOAD_LEN                   1
#define LL_REJECT_IND_PAYLOAD_LEN                      2
#define LL_FEATURE_REQ_PAYLOAD_LEN                     9
#define LL_FEATURE_RSP_PAYLOAD_LEN                     9
#define LL_VERSION_IND_PAYLOAD_LEN                     6
#define LL_UNKNOWN_RSP_PAYLOAD_LEN                     2
#define LL_REJECT_IND_EXT_PAYLOAD_LEN                  3
#define LL_CONN_PARAM_REQ_PAYLOAD_LEN                  24
#define LL_CONN_PARAM_RSP_PAYLOAD_LEN                  24
#define LL_PING_REQ_PAYLOAD_LEN                        1
#define LL_PING_RSP_PAYLOAD_LEN                        1
#define LL_LENGTH_REQ_PAYLOAD_LEN                      8
#define LL_LENGTH_RSP_PAYLOAD_LEN                      8
#define LL_PHY_REQ_PAYLOAD_LEN                         3
#define LL_PHY_RSP_PAYLOAD_LEN                         3
#define LL_PHY_UPDATE_REQ_PAYLOAD_LEN                  5
// miscellaneous fields, in bytes
#define LL_CONNECT_REQ_LL_DATA_LEN                     22
#define LL_CONNECT_REQ_PKT_LEN                         34
#define LL_NUM_BYTES_FOR_CHAN_MAP                      5   //(LL_MAX_NUM_ADV_CHAN+LL_MAX_NUM_DATA_CHAN)/sizeof(uint8)

// Terminate Indication Control Packet
// Note: MD bit is set to 1 to force sending this packet with MD=1 even though
//       the rest of the TX FIFO is empty and the MD configuration is set to
//       use automatic MD insertion based on FIFO contents.
// Note: The bytes are listed in little endian and read right-to-left:
//       0x13: MD=1, LLID=3 (LL control packet)
//       0x02: Length=1 plus one since NR will decrement the length
//       0x02: CtrlType=2 (TERMINATE_IND)
#define LL_TERM_IND_PKT_HDR                            ((1 << LL_DATA_PDU_HDR_MD_BIT) | LL_DATA_PDU_HDR_LLID_CONTROL_PKT)

// max number of sequential NACKS before closing a connection event
#define LL_MAX_NUM_RX_NACKS_ALLOWED                    4

// control procedure timeout in coarse timer ticks
#define LL_MAX_CTRL_PROC_TIMEOUT                       64000 // 40s

// authenticated payload timeout
#define LL_APTO_DEFAULT_VALUE                          30000 // 30s in ms

// max future number of events for an update to parameters or data channel
#define LL_MAX_UPDATE_COUNT_RANGE                      32767

// Connection Setup
#define LL_LINK_SETUP_TIMEOUT                          5  // 6 connection intervals (i.e. 0..5)
#define LL_LINK_MIN_WIN_OFFSET                         2  // in 625us units
#define LL_LINK_WIN_OFFSET_ADJ                         2  // in 625us units

// Adv PDU Header Fields
#define LL_ADV_PDU_HDR_TXADDR                          6
#define LL_ADV_PDU_HDR_RXADDR                          7

// Data PDU Header Fields
#define LL_DATA_PDU_HDR_LLID_MASK                      0x03
//
#define LL_DATA_PDU_HDR_LLID_RESERVED                  0
#define LL_DATA_PDU_HDR_LLID_DATA_PKT_NEXT             1
#define LL_DATA_PDU_HDR_LLID_DATA_PKT_FIRST            2
#define LL_DATA_PDU_HDR_LLID_CONTROL_PKT               3
//
#define LL_DATA_PDU_HDR_NESN_BIT                       2
#define LL_DATA_PDU_HDR_SN_BIT                         3
#define LL_DATA_PDU_HDR_MD_BIT                         4
#define LL_DATA_PDU_HDR_NESN_MASK                      0x04
#define LL_DATA_PDU_HDR_SN_MASK                        0x08
#define LL_DATA_PDU_HDR_MD_MASK                        0x10

// Data PDU overhead due to HCI packet type(1), connId(2), and length(2)
// Note: This is temporary until replace by BM alloc/free.
#define LL_DATA_HCI_OVERHEAD_LENGTH                    5

// Data PDU Control Packet Types
#define LL_CTRL_CONNECTION_UPDATE_REQ                  0  // M
#define LL_CTRL_CHANNEL_MAP_REQ                        1  // M
#define LL_CTRL_TERMINATE_IND                          2  // M, S
#define LL_CTRL_ENC_REQ                                3  // M
#define LL_CTRL_ENC_RSP                                4  //  , S
#define LL_CTRL_START_ENC_REQ                          5  //  , S
#define LL_CTRL_START_ENC_RSP                          6  // M, S
#define LL_CTRL_UNKNOWN_RSP                            7  // M, S
#define LL_CTRL_FEATURE_REQ                            8  // M
#define LL_CTRL_FEATURE_RSP                            9  // M, S
#define LL_CTRL_PAUSE_ENC_REQ                          10 // M
#define LL_CTRL_PAUSE_ENC_RSP                          11 //  , S
#define LL_CTRL_VERSION_IND                            12 // M, S
#define LL_CTRL_REJECT_IND                             13 //  , S
#define LL_CTRL_SLAVE_FEATURE_REQ                      14 //  , S
#define LL_CTRL_CONNECTION_PARAM_REQ                   15 // M, S
#define LL_CTRL_CONNECTION_PARAM_RSP                   16 //  , S
#define LL_CTRL_REJECT_IND_EXT                         17 // M, S
#define LL_CTRL_PING_REQ                               18 // M, S
#define LL_CTRL_PING_RSP                               19 // M, S
#define LL_CTRL_LENGTH_REQ                             20 // M, S
#define LL_CTRL_LENGTH_RSP                             21 // M, S
#define LL_CTRL_PHY_REQ                                22 // M, S
#define LL_CTRL_PHY_RSP                                23 //  , S
#define LL_CTRL_PHY_UPDATE_REQ                         24 // M
//
#define LL_CTRL_TERMINATE_RX_WAIT_FOR_TX_ACK           0xF0 // M (internal to LL only)
//
// The following "control packet types" are internally defined to assist the
// Link Layer with control procedure processing.
//
// The LL_CTRL_DUMMY_PLACE_HOLDER_TRANSMIT type is used as a place holder at the
// head of the control packet queue so that other control packets cannot
// interleave a currently executing control procedure. This entry would then be
// replaced with the appropriate control packet at a subsequent time. Any new
// control packets would remain queued behind until the current control
// procedure completes.
//
// The LL_CTRL_DUMMY_PLACE_HOLDER_TX_PENDING type is used as a place holder for
// a control packet that has already been queued for Tx, but due to a collision
// caused by a received packet, must be dequeued and restored at a later time
// without being re-queued on the Tx FIFO.
//
// The LL_CTRL_DUMMY_PLACE_HOLDER_RECEIVE type is used to delay the processing
// of a received control packet.
#define LL_CTRL_DUMMY_PLACE_HOLDER_TRANSMIT            0xFC
#define LL_CTRL_DUMMY_PLACE_HOLDER_TX_PENDING          0xFD
#define LL_CTRL_DUMMY_PLACE_HOLDER_RECEIVE             0xFE
#define LL_CTRL_UNDEFINED_PKT                          0xFF
//

// There is only supposed to be at most one control procedure pending, but some
// extra space is allocated here just in case some queueing is required.
#define LL_MAX_NUM_CTRL_PROC_PKTS                      4

// Control Procedure Actions
#define LL_CTRL_PROC_STATUS_SUCCESS                    0
#define LL_CTRL_PROC_STATUS_TERMINATE                  1

// Setup Next Slave Procedure Actions
#define LL_SETUP_NEXT_LINK_STATUS_SUCCESS              0
#define LL_SETUP_NEXT_LINK_STATUS_TERMINATE            1

// Receive Flow Control
#define LL_RX_FLOW_CONTROL_DISABLED                    0
#define LL_RX_FLOW_CONTROL_ENABLED                     1

// Scanner Advertisement Channels
#define LL_SCAN_ADV_CHAN_37                            37
#define LL_SCAN_ADV_CHAN_38                            38
#define LL_SCAN_ADV_CHAN_39                            39

// Advertiser Synchronization Word
#define ADV_SYNCH_WORD                                 0x8E89BED6  // Adv channel sync
#define ADV_CRC_INIT_VALUE                             0x00555555  // not needed; handled by NR hardware automatically

// Connection Related
#define LL_INVALID_CONNECTION_ID                       0xFF
#define LL_RESERVED_CONNECTION_ID                      0x0F00

// Feature Response Flag
#define LL_FEATURE_RSP_INIT                            0
#define LL_FEATURE_RSP_PENDING                         1
#define LL_FEATURE_RSP_DONE                            2
#define LL_FEATURE_RSP_FAILED                          3

// Encryption Related
#define LL_ENC_RAND_LEN                                8
#define LL_ENC_EDIV_LEN                                2
#define LL_ENC_LTK_LEN                                 16
#define LL_ENC_IV_M_LEN                                4
#define LL_ENC_IV_S_LEN                                4
#define LL_ENC_IV_LINK_LEN                             4
#define LL_ENC_IV_LEN                                  (LL_ENC_IV_M_LEN + LL_ENC_IV_S_LEN)
#define LL_ENC_SKD_M_LEN                               8
#define LL_ENC_SKD_S_LEN                               8
#define LL_ENC_SKD_LINK_LEN                            8
#define LL_ENC_SKD_LEN                                 (LL_ENC_SKD_M_LEN + LL_ENC_SKD_S_LEN)
#define LL_ENC_SK_LEN                                  16
#define LL_ENC_NONCE_LEN                               13
#define LL_ENC_NONCE_IV_OFFSET                         5
#define LL_ENC_MIC_LEN                                 LL_PKT_MIC_LEN
//
#define LL_ENC_IV_M_OFFSET                             LL_ENC_IV_S_LEN
#define LL_ENC_IV_S_OFFSET                             0
#define LL_ENC_SKD_M_OFFSET                            LL_ENC_SKD_S_LEN
#define LL_ENC_SKD_S_OFFSET                            0
//
#define LL_ENC_BLOCK_LEN                               16
#define LL_ENC_CCM_BLOCK_LEN                           LL_ENC_BLOCK_LEN
#define LL_ENC_BLOCK_B0_FLAGS                          0x49
#define LL_ENC_BLOCK_A0_FLAGS                          0x01

// TX Output Power Related
#define LL_TX_POWER_5_DBM                              5
#define LL_TX_POWER_4_DBM                              4
#define LL_TX_POWER_3_DBM                              3
#define LL_TX_POWER_2_DBM                              2
#define LL_TX_POWER_1_DBM                              1
#define LL_TX_POWER_0_DBM                              0
#define LL_TX_POWER_MINUS_3_DBM                        -3
#define LL_TX_POWER_MINUS_6_DBM                        -6
#define LL_TX_POWER_MINUS_9_DBM                        -9
#define LL_TX_POWER_MINUS_12_DBM                       -12
#define LL_TX_POWER_MINUS_15_DBM                       -15
#define LL_TX_POWER_MINUS_18_DBM                       -18
#define LL_TX_POWER_MINUS_21_DBM                       -21
#define LL_TX_POWER_INVALID                            -128

// TX Data Context
#define LL_TX_DATA_CONTEXT_TX_ISR                      0
#define LL_TX_DATA_CONTEXT_POST_PROCESSING             1

// Direct Test Mode Related
#define LL_DIRECT_TEST_SYNCH_WORD                      0x71764129
#define LL_DIRECT_TEST_CRC_INIT_VALUE                  0x55555500
#define LL_DIRECT_TEST_CRC_LEN                         3
#define LL_DTM_MAX_PAYLOAD_LEN                         37

// Post-Radio Operations
#define LL_POST_RADIO_SET_RX_GAIN_STD                  0x0001
#define LL_POST_RADIO_SET_RX_GAIN_HIGH                 0x0002
#define LL_POST_RADIO_SET_TX_POWER_MINUS_23_DBM        0x0004
#define LL_POST_RADIO_SET_TX_POWER_MINUS_6_DBM         0x0008
#define LL_POST_RADIO_SET_TX_POWER_0_DBM               0x0010
#define LL_POST_RADIO_SET_TX_POWER_4_DBM               0x0020
#define LL_POST_RADIO_GET_TRNG                         0x0040
#define LL_POST_RADIO_CACHE_RANDOM_NUM                 0x0080
#define LL_POST_RADIO_EXTEND_RF_RANGE                  0x0100

/*
** LL RF Frequencies
*/
// BLE RF Values
#define LL_FIRST_RF_CHAN_FREQ                          2402      // MHz
#define LL_LAST_RF_CHAN_FREQ                           (LL_FIRST_RF_CHAN_FREQ+(2*LL_LAST_RF_CHAN))
//
// The BLE RF interface uses a 8 bit channel field to specify the RF channel:
//  0 ..  39: BLE Advertising/Data Channels
// 40 ..  59: Reserved
// 60 .. 207: Customer Frequency given by 2300+channel MHz
//       255: Use existing frequency.
//
#define LL_FIRST_RF_CHAN_FREQ_OFFSET                   2300
#define LL_FIRST_RF_CHAN_ADJ                           (LL_FIRST_RF_CHAN_FREQ - LL_FIRST_RF_CHAN_FREQ_OFFSET)
#define LL_LAST_RF_CHAN_ADJ                            (LL_LAST_RF_CHAN_FREQ - LL_FIRST_RF_CHAN_FREQ_OFFSET)

/*
** FCFG and CCFG Offsets, and some Miscellaneous
*/

// Flash Size
#define LL_FLASH_PAGE_SIZE                             4096      // in bytes
#define LL_FLASH_SIZE_OFFSET                           0x2B1     // in FCFG; num of pages

// BADDR Flash Address Offset in CCA (i.e. flash programmer BLE address)
#define LL_BADDR_PAGE_OFFSET                           0xFD0     // in CCFG (CCA); LSB..MSB
#define LL_BADDR_PAGE_LEN                              6

// BADDR Address Offset in FCFG1 (i.e. permanent BLE address)
#define LL_BDADDR_OFFSET                               0x2E8     // in FCFG; LSB..MSB

// Chip ID offset in FCFG1
#define LL_INFO_PAGE_CHIP_ID_OFFSET                    0x118     // in FCFG; LSB..MSB, 16 bytes

// RSSI Offset (i.e. correction) in FCFG1 (PG1 only!)
#define LL_RSSI_OFFSET                                 0x380     // in FCFG; bits 16..9, signed 8 bit value?

#if !defined(DISABLE_RCOSC_SW_FIX)
// MODE_CONF SCLK_LF_OPTION selection for SCLK_LF
#define SCLK_LF_OPTION_OFFSET                          0xFB6     // in CCFG (CCA); LSB..MSB

// SCLK_LF Options
#define SCLK_LF_MASK                                   0xC0
//
#define SCLK_LF_XOSC_HF                                0
#define SCLK_LF_EXTERNAL                               1
#define SCLK_LF_XOSC_LF                                2
#define SCLK_LF_RCOSC_LF                               3
//
#define RCOSC_LF_SCA                                   1500      // possible worst case drift in PPM
#endif // !DISABLE_RCOSC_SW_FIX

// values for pendingParamUpdate
#define PARAM_UPDATE_NOT_PENDING                       0
#define PARAM_UPDATE_PENDING                           1
#define PARAM_UPDATE_APPLIED                           2

// Update State Values for Slave Latency
#define UPDATE_SL_OKAY                                 0
#define UPDATE_RX_CTRL_ACK_PENDING                     1
#define UPDATE_NEW_TRANS_PENDING                       2

// Feature Set Related
#define LL_MAX_FEATURE_SET_SIZE                        8         // in bytes
//
#define LL_FEATURE_NONE                                0x00
// Byte 0
#define LL_FEATURE_ENCRYPTION                          0x01
#define LL_FEATURE_CONN_PARAMS_REQ                     0x02
#define LL_FEATURE_REJECT_IND_EXT                      0x04
#define LL_FEATURE_SLV_FEATURES_EXCHANGE               0x08
#define LL_FEATURE_PING                                0x10
#define LL_FEATURE_DATA_PACKET_LENGTH_EXTENSION        0x20
#define LL_FEATURE_PRIVACY                             0x40
#define LL_FEATURE_EXTENDED_SCANNER_FILTER_POLICIES    0x80
// Byte 1
#define LL_FEATURE_2MBPS                               0x01
#define LL_FEATURE_STABLE_MODULATION_INDEX             0x02
#define LL_FEATURE_RESERVED01                          0x04
#define LL_FEATURE_RESERVED02                          0x08
#define LL_FEATURE_RESERVED03                          0x10
#define LL_FEATURE_RESERVED04                          0x20
#define LL_FEATURE_RESERVED05                          0x40
#define LL_FEATURE_RESERVED06                          0x80
// Byte 2 - Byte 7
#define LL_FEATURE_RESERVED07                          0x01
#define LL_FEATURE_RESERVED08                          0x02
#define LL_FEATURE_RESERVED09                          0x04
#define LL_FEATURE_RESERVED10                          0x08
#define LL_FEATURE_RESERVED11                          0x10
#define LL_FEATURE_RESERVED12                          0x20
#define LL_FEATURE_RESERVED13                          0x40
#define LL_FEATURE_RESERVED14                          0x80

// V4.1 Connection Parameter Request
// ALT: REPLACE WITH ONE BYTE, USE MACROS
//uint8 connParamFlags;
//#define CONN_PARAM_CLEAR_ALL_FLAGS                   0x00
//#define CONN_PARAM_UNKNOWN_RSP_RECEIVED_FLAG         0x01
//#define CONN_PARAM_REJECT_IND_EXT_RECEIVED_FLAG      0x02
//#define CONN_PARAM_HOST_INITIATED_FLAG               0x04
//#define CONN_PARAM_REQ_RECEIVED_FLAG                 0x08
//#define CONN_PARAM_RSP_RECEIVED_FLAG                 0x10
//#define CONN_PARAM_UPDATE_ACTIVE_FLAG                0x20

// V4.2 - Extended Data Length
// V4.2 - Privacy V1.2
// V4.2 - Secure Connections

// V5.0 - 2 Mbps PHY
#define LL_PHY_NUMBER_OF_PHYS                          2
#define LL_PHY_BASE_PHY                                LL_PHY_1_MBPS
#define LL_PHY_FASTEST_PHY                             LL_PHY_2_MBPS
#define LL_PHY_SUPPORTED_PHYS                          (LL_PHY_1_MBPS | LL_PHY_2_MBPS)
// Control Procedure Flags
#define CLEAR_ALL_FLAGS                                0x00
#define REJECT_IND_EXT_RECEIVED                        0x01
#define UNKNOWN_RSP_RECEIVED                           0x02
#define HOST_INITIATED                                 0x04
#define DISABLE_PHY_REQUEST                            0x08
#define PHY_RSP_RECEIVED                               0x10
#define UPDATE_PHY_RECEIVED                            0x20

/*
** Miscellaneous
*/
#define BITS_PER_BYTE                                  8
#define BYTES_PER_WORD                                 4

// HCI Connection Complete Roles
#define HCI_EVT_MASTER_ROLE                            0
#define HCI_EVT_SLAVE_ROLE                             1

/*******************************************************************************
 * TYPEDEFS
 */

/*
** Data PDU Control Packets
**
** The following structures are used to represent the various types of control
** packets. They are only used for recasting a data buffer for convenient
** field access
*/

// Connection Parameters
typedef struct
{
  uint8  winSize;                                    // window size
  uint16 winOffset;                                  // window offset
  uint16 connInterval;                               // connection interval
  uint16 slaveLatency;                               // number of connection events the slave can ignore
  uint16 connTimeout;                                // supervision connection timeout
} connParam_t;

// Flags for Connection Parameters Request Control Procedure
typedef struct
{
  uint8 hostInitiated;                               // flag to indicate the Host initiated the Update
  uint8 unknownRspRcved;                             // flag to indicate Unknown Response received
  uint8 rejectIndExtRcved;                           // flag to indicate Reject Ind Extended received
  uint8 connParamReqRcved;                           // flag to indicate Connection Parameter Request received
  //uint8 connParamRspRcved;                           // flag to indicate Connection Parameter Response received
  uint8 connUpdateActive;                            // flag to indicate a Connection Update Parameter or Update Channel procedure active
} connParamFlags_t;

// Connection Parameters Request or Response
typedef struct
{
  uint16 intervalMin;                                // lower connection interval limit
  uint16 intervalMax;                                // upper connection interval limit
  uint16 latency;                                    // slave latency
  uint16 timeout;                                    // connection timeout
  uint8  periodicity;                                // preferred periodicity
  uint16 refConnEvtCount;                            // reference connection event count
  uint16 offset0;                                    // offset 0
  uint16 offset1;                                    // offset 1
  uint16 offset2;                                    // offset 2
  uint16 offset3;                                    // offset 3
  uint16 offset4;                                    // offset 4
  uint16 offset5;                                    // offset 5
} connParamReq_t;

// Channel Map
typedef struct
{
  uint8 chanMap[ LL_NUM_BYTES_FOR_CHAN_MAP ];        // bit map corresponding to the data channels 0..39
} chanMap_t;

// Encryption Request
typedef struct
{
  uint8 RAND[LL_ENC_RAND_LEN];                       // random vector from Master
  uint8 EDIV[LL_ENC_EDIV_LEN];                       // encrypted diversifier from Master
  uint8 SKDm[LL_ENC_SKD_M_LEN];                      // master SKD values concatenated
  uint8 IVm[LL_ENC_IV_M_LEN];                        // master IV values concatenated
} encReq_t;

// Encryption Response
typedef struct
{
  uint8 SKDs[LL_ENC_SKD_S_LEN];                      // slave SKD values concatenated
  uint8 IVs[LL_ENC_IV_S_LEN];                        // slave IV values concatenated
} encRsp_t;

// Unknown Response
typedef struct
{
  uint8 unknownType;                                 // control type of the control
} unknownRsp_t;                                      // packet that caused was unknown

// Feature Request
typedef struct
{
  uint8 featureSet[ LL_MAX_FEATURE_SET_SIZE ];       // features that are used or not
} featureReq_t;

// Feature Response
typedef struct
{
  uint8 featureSet[ LL_MAX_FEATURE_SET_SIZE ];       // features that are used or not
} featureRsp_t;

// Version Information
typedef struct
{
  uint8  verNum;                                     // controller spec version
  uint16 comId;                                      // company identifier
  uint16 subverNum;                                  // implementation version
} verInfo_t;

/*
** Connection Data
**
** The following structures are used to hold the data needed for a LL
** connection.
*/

// Encryption
typedef struct
{
  // Note: IV and SKD provide enough room for the full IV and SKD. When the
  //       Master and Slave values are provided, the result is one combined
  //       (concatenated) value.
  uint8  IV[ LL_ENC_IV_LEN ];                        // combined master and slave IV values concatenated
  uint8  SKD [ LL_ENC_SKD_LEN ];                     // combined master and slave SKD values concatenated
  uint8  RAND[ LL_ENC_RAND_LEN ];                    // random vector from Master
  uint8  EDIV[ LL_ENC_EDIV_LEN ];                    // encrypted diversifier from Master
  uint8  nonce[ LL_ENC_NONCE_LEN ];                  // current nonce with current IV value
  uint8  SK[ LL_ENC_SK_LEN ];                        // session key derived from LTK and SKD
  uint8  LTK[ LL_ENC_LTK_LEN ];                      // Long Term Key from Host
  uint8  SKValid;                                    // flag that indicates the Session Key is valid
  uint8  LTKValid;                                   // Long Term Key is valid
  uint32 txPktCount;                                 // used for nonce formation during encryption (Note: 39 bits!)??
  uint32 rxPktCount;                                 // used for nonce formation during encryption (Note: 39 bits!)??
  // ALT: Could use one variable with one bit for each state.
  uint8  encRestart;                                 // flag to indicate if an encryption key change took place
  uint8  encRejectErrCode;                           // error code for rejecting encryption request
  uint8  startEncRspRcved;                           // flag to indicate the Start Request has been responded to
  uint8  pauseEncRspRcved;                           // flag to indicate the Pause Request has been responded to
  uint8  encReqRcved;                                // flag to indicate an Enc Req was received in a Enc Pause procedure
  uint8  encInProgress;                              // flag used to prevent a enc control procedure while one is already running

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  uint8  startEncReqRcved;                           // flag to indicate the Start Request has been responded to
  uint8  rejectIndRcved;                             // flag to indicate the Start Encryption needs to be aborted
#endif // CTRL_CONFIG=INIT_CFG

} encInfo_t;

// Feature Set Data
typedef struct
{
  uint8 featureRspRcved;                             // flag to indicate the Feature Request has been responded to
  uint8 featureSet[ LL_MAX_FEATURE_SET_SIZE ];       // features supported by this device
} featureSet_t;

// Connection Termination
typedef struct
{
  uint8 connId;                                      // connection ID
  uint8 termIndRcvd;                                 // indicates a TERMINATE_IND was received
  uint8 reason;                                      // reason code to return to Host when connection finally ends
} termInfo_t;

// Version Information Exchange
typedef struct
{
  uint8 peerInfoValid;                               // flag to indicate the peer's version information is valid
  uint8 hostRequest;                                 // flag to indicate the host has requested the peer's version information
  uint8 verInfoSent;                                 // flag to indicate this device's version information has been sent
} verExchange_t;

// Reject Indication Extended
typedef struct
{
  uint8 rejectOpcode;                                // opcode that was rejected
  uint8 errorCode;                                   // error code for rejection
} rejectIndExt_t;

// Control Procedure Information
typedef struct
{
  uint8  ctrlPktActive;                              // control packet at head of queue has been queued for Tx
  uint8  ctrlPkts[ LL_MAX_NUM_CTRL_PROC_PKTS ];      // queue of control packets to be processed
  uint8  ctrlPktCount;                               // number of queued control packets
  uint16 ctrlTimeoutVal;                             // timeout in CI events for control procedure for this connection
  uint16 ctrlTimeout;                                // timeout counter in CI events for control procedure
  //uint8  ctrlPktPending;                             // type of received control packet whose processing has been delayed
} ctrlPktInfo_t;

// Packet Error Rate Information - General
typedef struct
{
  uint16 numPkts;                                    // total number of packets received
  uint16 numCrcErr;                                  // number of packets with CRC error
  uint16 numEvents;                                  // number of connection events
  uint16 numMissedEvts;                              // number of missed connection events
} perInfo_t;

// TX Data
typedef struct txData_t
{
  struct txData_t *pNext;                            // pointer to next Tx data entry on queue
  uint8            fragFlag;                         // packet boundary flag
  uint16           len;                              // data length
  uint8           *pData;                            // pointer to data payload
} txData_t;

// Data Packet Queue
typedef struct
{
  txData_t *head;                                    // pointer to the head of the data queue
  txData_t *tail;                                    // pointer to the tail of the data queue
  uint8     numEntries;                              // number of data queue entries
} llDataQ_t;

// Peer Device Information
typedef struct
{
  uint8     peerAddrType;                            // peer device address type of public or random
  uint8     reserved[3];
  uint8     peerAddr[ LL_DEVICE_ADDR_LEN ];          // peer device address
} peerInfo_t;
// ROM WORKAROUND - change to this on next ROM spin
//PACKED_TYPEDEF_STRUCT
//{
//  uint8     peerAddr[ LL_DEVICE_ADDR_LEN ];          // peer device address
//  uint8     peerAddrType;                            // peer device address type of public or random
//  uint8     reserved;
//} peerInfo_t;

// PHY Information
typedef struct
{
  uint8             curPhy;
  uint8             updatePhy;
  uint8             phyFlags;
} phyInfo_t;

// Connection Data
typedef struct
{
  taskInfo_t       *llTask;                             // pointer to associated task block
  // General Connection Data
  uint8             allocConn;                          // flag to indicate if this connection is allocated
  uint8             activeConn;                         // flag to indicate if this connection is active
  uint8             connId;                             // connection ID
  uint16            currentEvent;                       // current event number
  uint16            nextEvent;                          // next active event number
  uint16            expirationEvent;                    // event at which the LSTO has expired
  uint16            expirationValue;                    // number of events to a LSTO expiration
  uint8             firstPacket;                        // flag to indicate when the first packet has been received
  uint16            scaFactor;                          // SCA factor for timer drift calculation
  uint32            timerDrift;                         // saved timer drift adjustment to avoid recalc
  // Connection Parameters
  uint32            lastTimeToNextEvt;                  // the time to next event from the previous connection event
  uint8             slaveLatencyAllowed;                // flag to indicate slave latency is permitted
  uint16            slaveLatency;                       // current slave latency; 0 means inactive
  uint8             lastSlaveLatency;                   // last slave latency value used
  uint16            slaveLatencyValue;                  // current slave latency value (when enabled)
  uint32            accessAddr;                         // saved synchronization word to be used by Slave
  uint32            crcInit;                            // connection CRC initialization value (24 bits)
  uint8             sleepClkAccuracy;                   // peer's sleep clock accuracy; used by own device to determine timer drift
  connParam_t       curParam;                           // current connection parameters
  // Channel Map
  uint8             nextChan;                           // the channel for the next active connection event
  uint8             currentChan;                        // the channel for the currently completed connection event
  uint8             numUsedChans;                       // count of the number of usable data channels
  uint8             hopLength;                          // used for finding next data channel at next connection event
  uint8             chanMapTable[LL_MAX_NUM_DATA_CHAN]; // current chan map table that is in use for this connection
  chanMap_t         curChanMap;
  // TX Related
  uint8             txDataEnabled;                      // flag that indicates whether data output is allowed
  dataEntryQ_t     *pTxDataEntryQ;
  // RX Related
  uint8             rxDataEnabled;                      // flag that indicates whether data input is allowed
  dataEntryQ_t     *pRxDataEntryQ;
  uint8             lastRssi;                           // last data packet RSSI received on this connection
  // Control Packet Information
  ctrlPktInfo_t     ctrlPktInfo;                        // information for control procedure processing
  // Parameter Update Control Procedure
  uint8             pendingParamUpdate;                 // flag to indicate connection parameter update is pending
  uint16            paramUpdateEvent;                   // event count to indicate when to apply pending param update
  connParam_t       paramUpdate;                        // update parameters
  // Channel Map Update Control Procedure
  uint8             pendingChanUpdate;                  // flag to indicate connection channel map update is pending
  uint16            chanMapUpdateEvent;                 // event count to indicate when to apply pending chan map update
  // Encryption Data Control Procedure
  uint8             encEnabled;                         // flag to indicate that encryption is enabled for this connection
  encInfo_t         encInfo;                            // structure that holds encryption related data
  // Feature Set
  featureSet_t      featureSetInfo;                     // feature set for this connection
  // Version Information
  verExchange_t     verExchange;                        // version information exchange
  verInfo_t         verInfo;                            // peer version information
  // Termination Control Procedure
  termInfo_t        termInfo;                           // structure that holds connection termination data
  // Unknnown Control Packet
  uint8             unknownCtrlType;                    // value of unknown control type
  // Packet Error Rate
  perInfo_t         perInfo;                            // PER
  perByChan_t      *perInfoByChan;                      // PER by Channel
  // Connection Event Notification
  uint8             taskID;                             // user task ID to send task event
  uint16            taskEvent;                          // user event to send at end of connection event
  // ROM WORKAROUND
  // Peer Address
  //peerInfo_t        peerInfo;                           // peer device address and address type
} llConnState_t;

// Per BLE LL Connection
typedef struct
{
  uint8         numLLConns;                          // number of allocated connections
  uint8         numActiveConns;                      // number of allocated connections that are active
  uint8         currentConn;                         // the LL connection currently in use
  uint8         nextConn;                            // the next LL connection
  llConnState_t *llConnection;                       // connection state information
} llConns_t;

// ROM WORKAROUND
typedef struct
{
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & PING_CFG)
  // Authenticated Payload Timeout
  uint32            aptoValue;                          // APTO value, in ms
  uint8             pingReqPending;                     // flag to indicate PING Request control procedure in progress
  uint8             numAptoExp;                         // number of 1/2 APTO expirations
  uint8             aptoTimerId;                        // cbTimer timer ID needed to stop the timer
#endif // CTRL_V41_CONFIG=PING_CFG

  // Connection Parameter Control Procedure
  connParamFlags_t  connParamReqFlags;                  // flags for handling connection parameter request control procedure
  connParamReq_t    connParams;                         // connection parameters for Request and Response packets
  rejectIndExt_t    rejectIndExt;                       // Reject Indication Extended Sent

  // TEMP: THIS BELONGS IN ctrlPktInfo_t BUT THAT MESSES UP FLASH_ROM
  uint8             ctrlPktPending;                     // type of received control packet whose processing has been delayed

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG)
  uint16            numEventsLeft;                      // events left before LSTO expiration
  uint16            prevConnInterval;                   // saved curParam CI before overwritten by updateParam CI
#endif // CTRL_V41_CONFIG & MST_SLV_CFG

  // TEMP: THIS BELONGS IN taskInfo_t, BUT THEN ANOTHER MALLOC IS NEEDED.
  uint32            lastTimeoutTime;

  // TEMP: THIS BELONGS AT END OF llConnState_t.
  peerInfo_t        peerInfo;                           // peer device address and address type

#if defined(CTRL_V50_CONFIG) && (CTRL_V50_CONFIG & PHY_2MBPS_CFG)
  // V5.0
  uint8             pendingPhyUpdate;                   // flag to indicate a PHY update is pending
  uint16            phyUpdateEvent;                     // instant event for PHY update
  phyInfo_t         phyInfo;                            // PHY info for update
#endif // PHY_2MBPS_CFG

#if !defined(DISABLE_RCOSC_SW_FIX)
  // save off master contribution
  uint16            mstSCA;                             // Master's portion of connection SCA
#endif // !DISABLE_RCOSC_SW_FIX

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  uint8             updateSLPending;                    // flag to monitor Master confirmation of Slave's ACK for update
#endif // ADV_CONN_CFG
} llConnStateExt_t;

// Advertising Event Parameters
typedef struct
{
  taskInfo_t *llTask;                                // pointer to associated task block
  uint8       ownAddrType;                           // own device address type of public or random
  uint8       reserved[3];                           // required for alignment
  uint8       ownAddr[ LL_DEVICE_ADDR_LEN ];         // own device address
  //
  uint8       advMode;                               // flag to indicate if currently advertising
  uint16      advInterval;                           // the advertiser interval, based on advIntMin and advIntMax
  uint8       advEvtType;                            // connectable directed, undirected, discoverable, or non-connectable
  uint8       advChanMap;                            // saved Adv channel map; note, only lower three bits used
  uint8       wlPolicy;                              // white list policy for Adv
  uint16      scaValue;                              // Slave SCA in PPM
  // For Connectable Undirected, Discoverable, and Nonconnectable Events only
  uint8       advDataChanged;                        // advertiser data has changed during advertising
  uint8       activeAdvBuf;
  uint8       advDataLen;                            // advertiser data length
  uint8       advData[2][LL_MAX_ADV_DATA_LEN];
  // Scan Response Parameters
  uint8       scanDataChanged;                        // advertiser data has changed during advertising
  uint8       activeScanRspBuf;
  uint8       scanRspLen;                            // scan response data length
  uint8       scanRspData[2][LL_MAX_SCAN_DATA_LEN];
  // Adv Event Notification
  uint8       taskID;                                // user task ID to send task event
  uint16      taskEvent;                             // user event to send at end of Adv event
} advInfo_t;

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG)
// Advertising Event Extension Parameters
typedef struct
{
  uint8       connId;                                // allocated connection ID
} advInfoExt_t;
#endif // CTRL_V41_CONFIG & MST_SLV_CFG

// Scanner Event Parameters
typedef struct
{
  taskInfo_t *llTask;                                // pointer to associated task block
  uint8       ownAddrType;                           // own device address type of public or random
  uint8       reserved[3];                           // required for alignment
  uint8       ownAddr[ LL_DEVICE_ADDR_LEN ];         // own device address
  //
  uint8       scanMode;                              // flag to indicate if currently scanning
  uint8       scanType;                              // passive or active scan
  uint16      scanInterval;                          // the interval between scan events
  uint16      scanWindow;                            // the duration of a scan event
  uint8       wlPolicy;                              // white list policy for Scan
  uint8       filterReports;                         // flag to indicate if duplicate Adv packet reports are to be filtered
  uint16      scanBackoffUL;                         // backoff upper limit count
  uint8       numSuccess;                            // for adjusting backoff count by tracking successive successes
  uint8       numFailure;                            // for adjusting backoff count by tracking successive failures
} scanInfo_t;

// Initiator Event Parameters
typedef struct
{
  taskInfo_t *llTask;                                // pointer to associated task block
  uint8       ownAddrType;                           // own device address type of public or random
  uint8       reserved[3];                           // required for alignment
  uint8       ownAddr[ LL_DEVICE_ADDR_LEN ];         // own device address
  //
  uint8       initPending;                           // flag to indicate if Scan needs to be initialized
  uint8       scanMode;                              // flag to indicate if currently scanning
  uint16      scanInterval;                          // the interval between scan events
  uint16      scanWindow;                            // the duration of a scan event
  uint8       wlPolicy;                              // white list policy for Init
  uint8       connId;                                // allocated connection ID
  uint8       scaValue;                              // Master SCA as an ordinal value for PPM
} initInfo_t;

// Direct Test Mode
typedef struct
{
  uint8       rfChan;                                // 0..39
  uint8       packetLen;                             // 0..39 bytes
  uint8       packetType;                            // data pattern
  uint16      numPackets;                            // number of packets received
  uint16      numRxCrcNOK;                           // number of packets received with CRC error
  uint8       lastRssi;                              // RSSI of last packet received
} dtmInfo_t;

// Direct Test Mode Enhanced
// ROM WORKAROUND - COMBINE WITH DTMINFO
typedef struct
{
  uint8       trxPhy;                                // 1Mbps or 2Mbps
  uint8       modIndex;                              // standard or stable
} dtmInfoExt_t;

// Build Revision
typedef struct
{
  uint16      userRevNum;                            // user revision number
} buildInfo_t;

// Link Layer Test Mode
#if defined(LL_TEST_MODE)

#define LL_TEST_MODE_TP_CON_MAS_BV_19                0
#define LL_TEST_MODE_TP_CON_MAS_BV_26                1
#define LL_TEST_MODE_TP_CON_MAS_BV_28                2
#define LL_TEST_MODE_TP_CON_MAS_BV_31_1              3
#define LL_TEST_MODE_TP_CON_MAS_BV_31_2              4
#define LL_TEST_MODE_TP_CON_MAS_BV_31_3              5
#define LL_TEST_MODE_TP_CON_MAS_BV_32                6
#define LL_TEST_MODE_TP_CON_MAS_BV_33                7
#define LL_TEST_MODE_TP_CON_MAS_BI_02                8
#define LL_TEST_MODE_TP_CON_MAS_BI_04                9
#define LL_TEST_MODE_TP_CON_MAS_BI_06                10
#define LL_TEST_MODE_TP_CON_SLA_BV_26                11
#define LL_TEST_MODE_TP_CON_SLA_BV_28                12
#define LL_TEST_MODE_TP_CON_SLA_BV_30_1              13
#define LL_TEST_MODE_TP_CON_SLA_BV_30_2              14
#define LL_TEST_MODE_TP_CON_SLA_BV_30_3              15
#define LL_TEST_MODE_TP_CON_SLA_BV_31                16
#define LL_TEST_MODE_TP_CON_SLA_BV_32                17
#define LL_TEST_MODE_TP_CON_SLA_BV_33                18
#define LL_TEST_MODE_TP_CON_SLA_BV_34                19
#define LL_TEST_MODE_TP_CON_SLA_BI_02                20
#define LL_TEST_MODE_TP_CON_SLA_BI_04                21
#define LL_TEST_MODE_TP_CON_SLA_BI_05                22
#define LL_TEST_MODE_TP_CON_SLA_BI_06                23
#define LL_TEST_MODE_TP_CON_SLA_BI_08                24
#define LL_TEST_MODE_TP_SEC_MAS_BV_08                25
#define LL_TEST_MODE_TP_SEC_SLA_BV_08                26
#define LL_TEST_MODE_TP_ENC_ADV_BI_02                27
#define LL_TEST_MODE_TP_TIM_SLA_BV_05                28
// v5.0 - 2Mbps
#define LL_TEST_MODE_TP_CON_SLA_BV_05                29
#define LL_TEST_MODE_TP_CON_SLA_BV_06                30
// Tickets
#define LL_TEST_MODE_JIRA_220                        200
#define LL_TEST_MODE_INVALID                         0xFF

typedef struct
{
  uint8 testCase;                                    // Core Test Spec Test Case
} llTestMode_t;

#endif // LL_TEST_MODE

// Invalid System Boot Message
#define INVALID_SYSBOOTMSG (uint8 *)0xFFFFFFFF

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

extern uint8        LL_TaskID;                       // link layer task ID
extern uint8        llState;                         // link layer state
extern llConns_t    llConns;                         // link layer connection table
extern advInfo_t    advInfo;                         // advertiser data
#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG)
extern advInfoExt_t advInfoExt;                      // advertiser data
#endif // CTRL_V41_CONFIG & MST_SLV_CFG
extern scanInfo_t   scanInfo;                        // scan data
extern initInfo_t   initInfo;                        // initiator data
extern verInfo_t    verInfo;                         // own version information
extern peerInfo_t   peerInfo;                        // peer device related data
extern dtmInfo_t    dtmInfo;                         // direct test mode data
// ROM WORKAROUND - COMBINE WITH DTMINFO
extern dtmInfoExt_t dtmInfoExt;                       // direct test mode data enhanced
extern buildInfo_t  buildInfo;                       // build revision data
extern featureSet_t deviceFeatureSet;                // device feature set
extern uint8        curTxPowerVal;                   // current Tx Power value
extern uint8        maxTxPwrForDTM;                  // max power override for DTM
extern uint16       taskEndStatus;                   // radio task end status
extern uint16       postRfOperations;                // flags for post-RF operations
extern int8         rssiCorrection;                  // RSSI correction
extern uint8        onePktPerEvt;                    // one packet per event enable flag
extern uint8        fastTxRespTime;                  // fast TX response time enable flag
extern uint8        rxFifoFlowCtrl;                  // flag to indicate if RX Flow Control is enabled
extern uint8        slOverride;                      // flag for user suspension of SL
// ROM WORKAROUND - REMOVE FOR NEXT ROM FREEZE
extern uint16       sleepDelay;                      // delay sleep for XOSC stabilization upon reset and wake from PM3
extern uint8        numComplPkts;                    // number of completed Tx buffers
extern uint8        numComplPktsLimit;               // minimum number of completed Tx buffers before event
extern uint8        numComplPktsFlush;               // flag to indicate send number of completed buffers at end of event
#if defined( CC26XX ) || defined( CC13XX )
extern uint16       rfCfgAdiVal;                     // RF Config Value for ADI init
#endif // CC26XX/CC13XX

#ifdef LL_COLLECT_METRICS
extern metricInfo_t metricInfo;                      // metric collection data
#endif // LL_COLLECT_METRICS

#if defined( LL_TEST_MODE )
extern llTestMode_t llTestMode;                      // LL Test Mode test cases
//
extern volatile uint8 firstTx;
extern volatile uint8 timSlvBv05Done;
extern volatile uint8 numSets;
extern volatile uint8 numTxPkts;
extern volatile uint8 nomCI;
extern volatile uint8 numTxEvts;
extern volatile uint8 setFailed;
extern volatile uint8 numFailedSets;
extern volatile uint8 numFailedTx;
#endif // LL_TEST_MODE

// ROM WORKAROUND
extern llConnStateExt_t *llConnectionExt;
extern uint8            *activeConns;

// System Boot Message
extern uint8 *SysBootMsg;

// V5.0 - 2Mbps
extern uint8 defaultPhy;

#if !defined(DISABLE_RCOSC_SW_FIX)
// pointer to the sclkSrc, as defined in the CCFG (for RCOSC workaround)
extern uint8 *sclkSrc;
#endif // !DISABLE_RCOSC_SW_FIX

/*******************************************************************************
 * FUNCTIONS
 */

// Taskend Jump Tables
extern void (*taskEndAction)( void );

/*
** Link Layer Common
*/

// RF Management
extern void               llRfInit( void );
extern void               llResetRadio( void );
extern void               llHaltRadio( uint16 );
extern void               llRfStartFS( uint8, uint16 );
extern void               llSetFreqTune( uint8 );
extern void               llDisableRfInts( void );
extern void               llClearRfInts( void );
extern void               llEnableRfInts( void );
extern void               llProcessPostRfOps( void );
extern void               llSetTxPower( uint8 );
extern int8               llGetTxPower( void );
extern void               llExtendRfRange( void );
extern void               llGetTimeToStableXOSC( void );
extern regOverride_t     *llGetRfOverrideRegs( void );

// Task Setup
extern void               llSetupAdv( void );
extern void               llSetupDirectedAdvEvt( void );
extern void               llSetupUndirectedAdvEvt( void );
extern void               llSetupNonConnectableAdvEvt( void );
extern void               llSetupDiscoverableAdvEvt( void );
extern void               llSetupScan( void );
extern void               llSetupInit( uint8 );
extern void               llSetupConn( uint8 );

// Control Procedure Setup
extern uint8              llSetupUpdateParamReq( llConnState_t * );           // M
extern uint8              llSetupUpdateChanReq( llConnState_t * );            // M
extern uint8              llSetupEncReq( llConnState_t * );                   // M
extern uint8              llSetupEncRsp( llConnState_t * );                   // S
extern uint8              llSetupStartEncReq( llConnState_t * );              // S
extern uint8              llSetupStartEncRsp( llConnState_t * );              // M, S
extern uint8              llSetupPauseEncReq( llConnState_t * );              // M
extern uint8              llSetupPauseEncRsp( llConnState_t * );              // S
extern uint8              llSetupRejectInd( llConnState_t * );                // S
extern uint8              llSetupFeatureSetReq( llConnState_t * );            // M, S
extern uint8              llSetupFeatureSetRsp( llConnState_t * );            // M, S
extern uint8              llSetupVersionIndReq( llConnState_t * );            // M
extern uint8              llSetupTermInd( llConnState_t * );                  // M, S
extern uint8              llSetupUnknownRsp( llConnState_t * );               // M, S
extern uint8              llSetupPingReq( llConnState_t * );                  // M, S
extern uint8              llSetupPingRsp( llConnState_t * );                  // M, S
extern uint8              llSetupConnParamReq( llConnState_t * );             // M, S
extern uint8              llSetupConnParamRsp( llConnState_t * );             //  , S
extern uint8              llSetupRejectIndExt( llConnState_t * );             // M, S
//
extern uint8              llSetupPhyCtrlPkt( llConnState_t *, uint8 opcode ); // M, S

// Control Procedure Management
extern void               llEnqueueCtrlPkt( llConnState_t *, uint8 );
extern void               llDequeueCtrlPkt( llConnState_t * );
extern void               llReplaceCtrlPkt( llConnState_t *, uint8 );
extern void               llSendReject( llConnState_t *, uint8, uint8 );
extern uint8              llPendingUpdateParam( void );
extern void               llInitFeatureSet( void );
extern void               llConvertCtrlProcTimeoutToEvent( llConnState_t * );
extern uint8              llVerifyConnParamReqParams( uint16, uint16, uint16, uint8, uint16, uint16 *);

// Data Channel Management
extern void               llProcessChanMap( llConnState_t *, uint8 * );
extern NEAR_FUNC uint8    llGetNextDataChan( llConnState_t *, uint16 );
extern void               llSetNextDataChan( llConnState_t * );
extern uint8              llAtLeastTwoChans( uint8 * );

// Connection Management
extern llConnState_t      *llAllocConnId( void );
extern void               llReleaseConnId( llConnState_t * );
extern void               llReleaseAllConnId( void );
extern uint16             llGetMinCI( uint16  );
extern uint8              llGetNextConn( void );
extern void               llRealignConn( llConnState_t *, uint32 );
extern void               llSortActiveConns( uint8 *, uint8 );
extern void               llConnCleanup( llConnState_t * );
extern void               llConnTerminate( llConnState_t *, uint8  );
extern uint8              llConnExists( uint8, uint8 *, uint8 );
extern uint32             llGenerateCRC( void );
extern NEAR_FUNC uint8    llEventInRange( uint16 , uint16 , uint16  );
extern NEAR_FUNC uint16   llEventDelta( uint16 , uint16  );
extern void               llConvertLstoToEvent( llConnState_t *, connParam_t * );
extern uint8              llAdjustForMissedEvent( llConnState_t *, uint32  );
extern void               llAlignToNextEvent( llConnState_t *connPtr );
extern void               llGetAdvChanPDU( uint8 *, uint8 *, uint8 *, uint8 *, uint8 *, int8 * );
// Access Address
extern uint32             llGenerateValidAccessAddr( void );
extern uint8              llValidAccessAddr( uint32 );
extern uint8              llGtSixConsecZerosOrOnes( uint32 );
extern uint8              llEqSynchWord( uint32 );
extern uint8              llOneBitSynchWordDiffer( uint32 );
extern uint8              llEqualBytes( uint32 );
extern uint8              llGtTwentyFourTransitions( uint32 );
extern uint8              llLtTwoChangesInLastSixBits( uint32 );
extern uint8              llEqAlreadyValidAddr( uint32  );

// Data Management
extern uint8              llEnqueueDataQ( llDataQ_t *, txData_t * );
extern uint8              llEnqueueHeadDataQ( llDataQ_t *, txData_t * );
extern uint8              llDequeueDataQ( llDataQ_t *, txData_t ** );
extern uint8              llDataQFull( llDataQ_t * );
extern uint8              llDataQEmpty( llDataQ_t * );
extern void               llProcessTxData( llConnState_t *, uint8 );
extern uint8              llWriteTxData( llConnState_t *, uint8 *, uint8 , uint8 );
extern void               llCombinePDU( uint16, uint8 *, uint16, uint8 );
extern uint8              llFragmentPDU( llConnState_t *, uint8 *, uint16 );
extern uint8              *llMemCopySrc( uint8 *, uint8 *, uint8 );
extern uint8              *llMemCopyDst( uint8 *, uint8 *, uint8 );

// Failure Management
extern void               llHardwareError( uint8 );

// Advertising Task End Cause
extern void               llDirAdv_TaskEnd( void );
extern void               llAdv_TaskEnd( void );
extern void               llAdv_TaskConnect( void );
extern void               llAdv_TaskAbort( void );

// Scanner Task End Cause
extern void               llScan_TaskEnd( void );
extern void               llProcessScanRxFIFO( uint8 scanStatus );

// Initiator Task End Cause
extern void               llInit_TaskConnect( void );
extern void               llInit_TaskEnd( void );

// Master Task End Cause
extern void               llMaster_TaskEnd( void );
extern uint8              llProcessMasterControlProcedures( llConnState_t *connPtr );
extern uint8              llSetupNextMasterEvent( void );

// Slave Task End Cause
extern void               llSlave_TaskEnd( void );
extern uint8              llSetupNextSlaveEvent( void );
extern uint8              llProcessSlaveControlProcedures( llConnState_t * );
extern uint8              llCheckForLstoDuringSL( llConnState_t * );

// Error Related End Cause
extern void               llTaskError( void );

// White List Related
extern llStatus_t         llCheckWhiteListUsage( void );

// Timer Related Management
extern void               llCBTimer_AptoExpiredCback( uint8 * );

#ifdef __cplusplus
}
#endif

#endif /* LL_COMMON_H */
