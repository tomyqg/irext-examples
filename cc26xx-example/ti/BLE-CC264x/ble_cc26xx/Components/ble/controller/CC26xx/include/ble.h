/*******************************************************************************
  Filename:       ble.h
  Revised:        $Date: 2012-02-15 14:12:20 -0800 (Wed, 15 Feb 2012) $
  Revision:       $Revision: 29309 $

  Description:    This file contains the data structures and APIs for CC26xx
                  RF Core Firmware Specification for Bluetooth Low Energy.

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

#ifndef BLE_H
#define BLE_H

/*******************************************************************************
 * INCLUDES
 */

#include "rfHal.h"
#include "ll_wl.h"
#include "ll_common.h"
#include "ll_config.h"

/*******************************************************************************
 * CONSTANTS
 */

// Mailbox CPE Interrupts for BLE
#define MB_NO_INT                      0
#define MB_TX_DONE_INT                 BV(4)
#define MB_TX_ACK_INT                  BV(5)
#define MB_TX_CTRL_INT                 BV(6)
#define MB_TX_CTRL_ACK_INT             BV(7)
#define MB_TX_CTRL_ACK_ACK_INT         BV(8)
#define MB_TX_RETRANS_INT              BV(9)
#define MB_TX_ENTRY_DONE_INT           BV(10)
#define MB_TX_BUF_CHANGED_INT          BV(11)
#define MB_RX_OK_INT                   BV(16)
#define MB_RX_NOK_INT                  BV(17)
#define MB_RX_IGNORED_INT              BV(18)
#define MB_RX_EMPTY_INT                BV(19)
#define MB_RX_CTRL_INT                 BV(20)
#define MB_RX_CTRL_ACK_INT             BV(21)
#define MB_RX_BUF_FULL_INT             BV(22)
#define MB_RX_ENTRY_DONE_INT           BV(23)
#if defined( CC26XX ) || defined( CC13XX )
#define MB_RX_DATA_WRITTEN             BV(24)
#define MB_RX_N_DATA_WRITTEN           BV(25)
#define MB_RX_ABORTED                  BV(26)
#endif // CC26XX/CC13XX

// Mailbox Hardware Interrupts for BLE
#define MB_HW_RAT_CHAN_5_INT           RAT_CHAN_5_IRQ
#define MB_HW_RAT_CHAN_6_INT           RAT_CHAN_6_IRQ
#define MB_HW_RAT_CHAN_7_INT           RAT_CHAN_7_IRQ

// Mailbox BLE Immediate Commands
#define CMD_BLE_ADV_PAYLOAD            0x1001

// Mailbox BLE Direct Commands

// Mailbox BLE Radio Commands
#define CMD_BLE_SLAVE                  0x1801
#define CMD_BLE_MASTER                 0x1802
#define CMD_BLE_ADV                    0x1803
#define CMD_BLE_ADV_DIR                0x1804
#define CMD_BLE_ADV_NC                 0x1805
#define CMD_BLE_ADV_SCAN               0x1806
#define CMD_BLE_SCANNER                0x1807
#define CMD_BLE_INITIATOR              0x1808
#define CMD_BLE_RX_TEST                0x1809
#define CMD_BLE_TX_TEST                0x180A

// BLE Radio Operation Command Status
#define BLESTAT_IDLE                   RFSTAT_IDLE
#define BLESTAT_PENDING                RFSTAT_PENDING
#define BLESTAT_ACTIVE                 RFSTAT_ACTIVE
#define BLESTAT_SKIPPED                RFSTAT_SKIPPED
//
#define BLESTAT_DONE_OK                0x1400  // result True
#define BLESTAT_DONE_RXTIMEOUT         0x1401  // result False
#define BLESTAT_DONE_NOSYNC            0x1402  // result True
#define BLESTAT_DONE_RXERR             0x1403  // result True
#define BLESTAT_DONE_CONNECT           0x1404  // result True (False for Slave)
#define BLESTAT_DONE_MAXNACK           0x1405  // result True
#define BLESTAT_DONE_ENDED             0x1406  // result False
#define BLESTAT_DONE_ABORT             0x1407  // result Abort
#define BLESTAT_DONE_STOPPED           0x1408  // result False
//
#define BLESTAT_ERROR_PAR              0x1800  // result Abort
#define BLESTAT_ERROR_RXBUF            0x1801  // result False
#define BLESTAT_ERROR_NO_SETUP         0x1802  // result Abort
#define BLEBTAT_ERROR_NO_FS            0x1803  // result Abort
#define BLESTAT_ERROR_SYNTH_PROG       0x1804  // result Abort
#define BLESTAT_ERROR_RX_OVERFLOW      0x1805  // result Abort
#define BLESTAT_ERROR_TX_UNDERFLOW     0x1806  // result Abort

// Advertisement Data Type
#define BLE_ADV_DATA_TYPE              0
#define BLE_SCAN_RSP_DATA_TYPE         1

// Advertising Configuration
#define ADV_CFG_INCLUDE_BAD_LEN_MSG    0
#define ADV_CFG_DISCARD_BAD_LEN_MSG    1

// Scan Configuration
#define SCAN_CFG_WL_POLICY_ANY         0
#define SCAN_CFG_WL_POLICY_USE_WL      1
#define SCAN_CFG_PASSIVE_SCAN          0
#define SCAN_CFG_ACTIVE_SCAN           1
#define SCAN_CFG_ALLOW_ILLEGAL_LEN     0
#define SCAN_CFG_DISCARD_ILLEGAL_LEN   1
#define SCAN_CFG_NO_SET_IGNORE_BIT     0
#define SCAN_CFG_AUTO_SET_IGNORE_BIT   1

// Init Configuration
#define INIT_CFG_USE_PEER_ADDR         0
#define INIT_CFG_WL_POLICY_USE_WL      1
#define INIT_CFG_NO_DYN_WINOFFSET      0
#define INIT_CFG_USE_DYN_WINOFFSET     1
#define INIT_CFG_ALLOW_ILLEGAL_LEN     0
#define INIT_CFG_DISCARD_ILLEGAL_LEN   1

// RX Queue Configuration Bits
#define RXQ_CFG_CLEAR                  0
#define RXQ_CFG_AUTOFLUSH_IGNORED_PKT  BV(0)
#define RXQ_CFG_AUTOFLUSH_CRC_ERR_PKT  BV(1)
#define RXQ_CFG_AUTOFLUSH_EMPTY_PKT    BV(2)
#define RXQ_CFG_INCLUDE_PKT_LEN_BYTE   BV(3)
#define RXQ_CFG_INCLUDE_CRC            BV(4)
#define RXQ_CFG_APPEND_RSSI            BV(5)
#define RXQ_CFG_APPEND_STATUS          BV(6)
#define RXQ_CFG_APPEND_TIMESTAMP       BV(7)

// Sequence Number Status Bits
#define SEQ_NUM_CFG_CLEAR              0
#define SEQ_NUM_CFG_LAST_RX_SN         BV(0)
#define SEQ_NUM_CFG_LAST_TX_SN         BV(1)
#define SEQ_NUM_CFG_NEXT_TX_SN         BV(2)
#define SEQ_NUM_CFG_FIRST_PKT          BV(3)
#define SEQ_NUM_CFG_AUTO_EMPTY         BV(4)
#define SEQ_NUM_CFG_CTRL_TX            BV(5)
#define SEQ_NUM_CFG_CTRL_ACK_RX        BV(6)
#define SEQ_NUM_CFG_CTRL_ACK_PENDING   BV(7)

// RX Test
#define RX_TEST_END_AFTER_RX_PKT       0
#define RX_TEST_REPEAT_AFTER_RX_PKT    1

// Radio Timer (RAT) Times (in 250ns ticks)
#define RAT_TICKS_IN_1US               4         // Connection Jitter
#define RAT_TICKS_IN_6US               24        // Timestamp correction
#define RAT_TICKS_IN_10US              40        // Connection Jitter
#define RAT_TICKS_IN_15_5US            62        // TP/TIM/SLA/BV-05
#define RAT_TICKS_IN_16US              64        // Connection Jitter
#define RAT_TICKS_IN_64US              256       // Radio Rx Settle Time
#define RAT_TICKS_IN_85US              340       // Radio Rx Synch Time
#define RAT_TICKS_IN_100US             400       // 1M / 2500 RAT ticks (SCA PPM)
#define RAT_TICKS_IN_140US             560       // Rx Back-end Time
#define RAT_TICKS_IN_150US             600       // T_IFS
#define RAT_TICKS_IN_200US             800       // LL Topology margin
#define RAT_TICKS_IN_256US             1024      // Radio Overhead + FS Calibration
#define RAT_TICKS_IN_280US             1120      // Radio Overhead + FS Calibration
#define RAT_TICKS_IN_352US             1408      // CONNECT_REQ is 44 bytes
#define RAT_TICKS_IN_625US             2500      // Fundamental BLE Time Slot
#define RAT_TICKS_IN_1MS               4000      // Multiple of Adv Random Delay
#define RAT_TICKS_IN_1_25MS            5000      // Fundamental BLE Time Unit
#define RAT_TICKS_IN_10MS              40000     // General Purpose Delay
#define RAT_TICKS_IN_1_28S             5120000   // Directed Advertising Timeout
#define RAT_TICKS_IN_32S               128000000 // Max LSTO
//
#define RAT_TICKS_FOR_CONNECT_REQ      RAT_TICKS_IN_352US

// Radio Timer (RAT) Channels
#define RAT_CHAN_5                     5
#define RAT_CHAN_6                     6
#define RAT_CHAN_7                     7

// Miscellaneous
#define BLE_BDADDR_SIZE                6
#define MAX_BLE_DATA_PDU_SIZE          31 // data PDU payload + MIC
#define MAX_BLE_CONNECT_REQ_SIZE       34 // init addr + own addr + payload
#define MAX_BLE_ADV_PKT_SIZE           37 // payload
#define MAX_BLE_DTM_PKT_SIZE           37 // payload

// HCI Rx Packet Header
// | Packet Type (1) | Handler(2) | Length(2) |
// Note: This is the same define as what's in hci_tl.h: HCI_DATA_MIN_LENGTH!
#define HCI_RX_PKT_HDR_SIZE            5

//
#define BLE_CRC_LEN                    LL_PKT_CRC_LEN
#define BLE_CHAN_MAP_LEN               LL_NUM_BYTES_FOR_CHAN_MAP

/*******************************************************************************
 * MACROS
 */

// Receive Queue Entry Configuration
#define SET_RXQ_ENTRY_CFG( d, b )                                              \
  (d) |= BV(b)

#define CLR_RXQ_ENTRY_CFG( d, b )                                              \
  (d) &= ~BV(b)

// Sequence Number Status
#define SET_ENTRY_CFG( d, b )                                                  \
  (d) |= BV(b)

#define CLR_ENTRY_CFG( d, b )                                                  \
  (d) &= ~BV(b)

// Whitening Configuration
#define CLR_WHITENING( x )                                                     \
  (x) = 0x80

#define SET_WHITENING_BLE( x )                                                 \
  (x) = 0x00;

#define SET_WHITENING_INIT( x, v )                                             \
  (x) = 0x80 | ((v) & ~0x80)

// Advertising Configuration
#define SET_ADV_CFG_FILTER_POLICY( c, v )                                      \
  (c) = ((c) & ~0x03) | ((v) & 0x03)

#define SET_ADV_CFG_DEV_ADDR_TYPE( c, v )                                      \
  (c) = ((c) & ~0x04) | (((v) & 0x01)<<2)

#define SET_ADV_CFG_PEER_ADDR_TYPE( c, v )                                     \
  (c) = ((c) & ~0x08) | (((v) & 0x01)<<3)

#define SET_ADV_CFG_STRICT_LEN_FILTER( c, v )                                  \
  (c) = ((c) & ~0x10) | (((v) & 0x01)<<4)

// Scan Configuration
#define SET_SCAN_CFG_FILTER_POLICY( c, v )                                     \
  (c) = ((c) & ~BV(0)) | ((v) & 0x01);

#define SET_SCAN_CFG_ACTIVE_SCAN( c, v )                                       \
  (c) = ((c) & ~BV(1)) | (((v) & 0x01) << 1)

#define SET_SCAN_CFG_DEV_ADDR_TYPE( c, v )                                     \
  (c) = ((c) & ~BV(2)) | (((v) & 0x01) << 2)

#define SET_SCAN_CFG_STRICT_LEN_FILTER( c, v )                                 \
  (c) = ((c) & ~BV(4)) | (((v) & 0x01) << 4)

#define SET_SCAN_CFG_AUTO_SET_WL_IGNORE( c, v )                                \
  (c) = ((c) & ~BV(5)) | (((v) & 0x01) << 5)

#define SET_SCAN_CFG_END_ON_REPORT( c, v )                                     \
  (c) = ((c) & ~BV(6)) | (((v) & 0x01) << 6)

// Scan Backoff Configuration
#define SET_SCAN_BACKOFF_CFG_UL( c, v )                                        \
  (c) = ((c) & ~0x0F) | ((v) & 0x0F)

#define SET_SCAN_BACKOFF_CFG_LAST_SUCCCEEDED( c, v )                           \
  (c) = ((c) & ~BV(4)) | (((v) & 0x01) << 4)

#define SET_SCAN_BACKOFF_CFG_LAST_FAILED( c, v )                               \
  (c) = ((c) & ~BV(5)) | (((v) & 0x01) << 5)

// Init Configuration
#define SET_INIT_CFG_FILTER_POLICY( c, v )                                     \
  (c) = ((c) & ~BV(0)) | ((v) & 0x01);

#define SET_INIT_CFG_DYN_WINOFFSET( c, v )                                     \
  (c) = ((c) & ~BV(1)) | (((v) & 0x01) << 1)

#define SET_INIT_CFG_DEV_ADDR_TYPE( c, v )                                     \
  (c) = ((c) & ~BV(2)) | (((v) & 0x01) << 2)

#define SET_INIT_CFG_PEER_ADDR_TYPE( c, v )                                    \
  (c) = ((c) & ~BV(3)) | (((v) & 0x01) << 3)

#define SET_INIT_CFG_STRICT_LEN_FILTER( c, v )                                 \
  (c) = ((c) & ~BV(4)) | (((v) & 0x01) << 4)

// Transmit Test Configuration
#define SET_TX_TEST_CFG_OVERRIDE( c )                                          \
  (c) |= BV(0)

#define CLR_TX_TEST_CFG_OVERRIDE( c )                                          \
  (c) &= ~BV(0)

#define SET_TX_TEST_CFG_USE_PRBS9( c )                                         \
  (c) = ((c) & 0x06) | BV(1);

#define SET_TX_TEST_CFG_USE_PRBS15( c )                                        \
  (c) = ((c) & 0x06) | BV(2);

// Receive Status Byte Bit Field
#define VALID_TIMESTAMP( s )                                                   \
  (s) & BV(0)

#define LAST_RX_PKT_CRC_ERROR( s )                                             \
  (s) & BV(1)

#define LAST_RX_PKT_IGNORED( s )                                               \
  (s) & BV(2)

#define LAST_RX_PKT_EMPTY( s )                                                 \
  (s) & BV(3)

#define LAST_RX_PKT_CONTROL( s )                                               \
  (s) & BV(4)

#define LAST_RX_PKT_MD( s )                                                    \
  (s) & BV(5)

#define LAST_RX_PKT_ACK( s )                                                   \
  (s) & BV(6)

#define WAIT( t ) {volatile uint32 time = (t); while( time-- );}

/*******************************************************************************
 * TYPEDEFS
 */

// BLE Radio Operation Whitening Configuration
// |     7    |       6..0     |
// | Override | Whitening Init |
//
typedef uint8 whitenCfg_t;

// BLE Radio Operation Receive Queue Configuration
// |     7     |   6    |   5  |  4  |      3      |       2        |         1           |         0         |
// | Timestamp | Status | RSSI | CRC | Length Byte | AutoFlushEmpty | AutoFlush CRC Error | AutoFlush Ignored |
//
typedef uint8 rxQCfg_t;

// BLE Radio Operation Sequence Number Status
// |         7        |      6      |    5    |      4       |      3       |     2      |      1     |     0      |
// | Ctrl Ack Pending | Ctrl Ack Rx | Ctrl Tx | Tx AutoEmpty | First Packet | Next Tx SN | Last Tx SN | Last Rx SN |
//
typedef uint8 seqNumStat_t;

// BLE Advertiser Configuration
// | 7..5 |        4          |        3       |        2         |     1..0      |
// |  N/A | Strict Len Filter | Peer Addr Type | Device Addr Type | Filter Policy |
//
typedef uint8 advCfg_t;

// BLE Scanner Configuration
// |  7  |       6       |         5         |       4       |  3  |        2         |      1              0
// | N/A | End On Report | AutoSet WL Ignore | Length Filter | N/A | Device Addr Type | Active Scan | Filter Policy |
//
typedef uint8 scanCfg_t;

// BLE Backoff Parameters
// | 7..6 |      5      |      4       |     3..0        |
// |  N/A | Last Failed | Last Success | Log Upper Limit |
//
typedef uint8 bkOff_t;

// BLE Initiator Configuration
// | 7..5  |      4        |       3        |        2         |         1          |       0        |
// |  N/A  | Length Filter | Peer Addr Type | Device Addr Type | Dyn. Window Offset | Use White list |
//
typedef uint8 initCfg_t;

// BLE Transmit Test Override Configuration
// | 7..3 |      2     |     1     |         0         |
// |  N/A | Use PRBS15 | Use PRBS9 | Encoding Override |
//
typedef uint8 txTestCfg_t;

// Receive Status Byte Bit Field
// |  7  |    6    |    5   |     4    |     3     |    2    |    1    |        0        |
// | N/A | lastACK | lastMD | lastCTRL | lastEmpty | lastIgn | lastErr | Timestamp Valid |
//
typedef uint8 pktStat_t;

/*
** BLE Data Entry Structures
*/

// IDEA:
// USE OVERLAY TO MAKE THIS WORK.
// E.G. SIZE = sizeof( dataEntry_t ) + sizeof( dataEntryPrefix_t ) + dataSize + sizeof( dataEntrySuffix_t )
//      THEN CREATE A BUFFER OF THAT SIZE: uint8 buf[ SIZE ]
//      THEN MAP DATA ENTRY POINTER: dataQueue->pCurEntry = &buf[ sizeof( dataEntryPrefix_t ) ]
//      THEN BEGIN AT: &(dataQueue->pCurEntry->length+sizeof(uint16))?

// Data Entry Prefix
// Note: Not formally part of the radio data entry definition.
// ALT: Could move to BLE file.
PACKED_TYPEDEF_STRUCT
{
  uint8  bleStateRole;                 // could be combined wtih connection ID
  uint8  reserved;                     // for packing
  uint16 bleConnID;                    // could be combined with task state NOT NEEDED IF QUEUE PER CONNECTION
  uint32 rxPktCount;                   // used for building nonce for encryption
} dataEntryPrefix_t;

/*
** BLE Radio Commands
**
** R  = System CPU reads; Radio CPU will not read.
** W  = System CPU writes; Radio CPU reads but will not modify.
** RW = System CPU writes initially; Radio CPU reads and may modify.
*/

// BLE Radio Operation Command Common Structure
PACKED_TYPEDEF_STRUCT
{
  rfOpCmd_t     rfOpCmd;               // radio command common structure
  uint8         chan;                  // W:  channel number
  whitenCfg_t   whitening;             // W:  whitening configuration
  uint8        *pParams;               // W:  ptr to cmd specific parameters
  uint8        *pOutput;               // W:  ptr to cmd specific results, or NULL
} bleOpCmd_t;

/*
** BLE Input Command Parameter Structures
*/

// Advertiser Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  advCfg_t      advCfg;                // W:  advertiser configuration
  uint8         advLen;                // W:  size of Adv data
  uint8         scanRspLen;            // W:  size of Scan Response data
  uint8        *pAdvData;              // W:  ptr to Adv data
  uint8        *pScanRspData;          // W:  ptr to Scan Response data
  uint8        *pDeviceAddr;           // W:  ptr to device BLE address
  wlEntry_t    *pWhiteList;            // W:  ptr to white list
  uint8         reserved[3];           // unused
  trig_t        endTrig;               // W:  end trig for adv event
  uint32        endTime;               // W:  time for end trigger
} advParam_t;

// Scanner Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  scanCfg_t     scanCfg;               // W:  advertiser configuration
  uint16        randState;             // RW: a pseudo-random number
  uint16        backoffCount;          // RW: backoff count
  bkOff_t       backoffParam;          // RW: backoff parameters
  uint8         scanReqLen;            // W:  size of Scan Request data
  uint8        *pScanReqData;          // W:  ptr to Scan Request data
  uint8        *pDeviceAddr;           // W:  ptr to device address
  wlEntry_t    *pWhiteList;            // W:  ptr to white list
  uint16        reserved1;             // unused
  trig_t        timeoutTrig;           // W:  timeout trig for first Rx operation
  trig_t        endTrig;               // W:  end trig for connection event
  uint32        timeoutTime;           // W:  time for timeout trigger
  uint32        endTime;               // W:  time for end trigger
} scanParam_t;

// Initiator Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  initCfg_t     initCfg;               // W:  initiator configuration
  uint8         reserved1;             // unused
  uint8         connReqLen;            // W:  size of Connect Request data
  uint8        *pConnReqData;          // W:  ptr to Connect Request data
  uint8        *pDeviceAddr;           // W:  ptr to device address
  wlEntry_t    *pWhiteList;            // W:  ptr to white list
  uint32        connectTime;           // RW: time of first connection event
  uint16        reserved2;             // unused
  trig_t        timeoutTrig;           // W:  timeout trig for first Rx operation
  trig_t        endTrig;               // W:  end trig for connection event
  uint32        timeoutTime;           // W:  time for timeout trigger
  uint32        endTime;               // W:  time for end trigger
} initParam_t;

// Initiator Command CONNECT_REQ LL_Data
PACKED_TYPEDEF_STRUCT
{
  uint32        accessAddress;         // W:  access address used in connection
  uint8         crcInit[BLE_CRC_LEN];  // W:  CRC init value
  uint8         winSize;
  uint16        winOffset;
  uint16        connInterval;
  uint16        latency;
  uint16        timeout;
  uint8         chanMap[BLE_CHAN_MAP_LEN];
  uint8         hopSca;
} connReqData_t;

// Master Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  dataEntryQ_t *pTXQ;                  // W:  ptr to Tx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  seqNumStat_t  seqStat;               // RW: sequence status bit field
  uint8         maxNAck;               // W:  max number of NACKs allowed
  uint8         maxTxPkt;              // W:  max number of Tx pkts allowed
  uint32        accessAddress;         // W:  access address used in connection
  uint8         crcInit[BLE_CRC_LEN];  // W:  CRC init value
  trig_t        endTrig;               // W:  end trig for connection event
  uint32        endTime;               // W:  time for end trigger
} masterParam_t;

// Slave Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  dataEntryQ_t *pTXQ;                  // W:  ptr to Tx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  seqNumStat_t  seqStat;               // RW: sequence status bit field
  uint8         maxNAck;               // W:  max number of NACKs allowed
  uint8         maxTxPkt;              // W:  max number of Tx pkts allowed
  uint32        accessAddress;         // W:  access address used in connection
  uint8         crcInit[BLE_CRC_LEN];  // W:  CRC init value
  trig_t        timeoutTrig;           // W:  timeout trig for first Rx operation
  uint32        timeoutTime;           // W:  time for timeout trigger
  uint8         reserved[3];           // unused
  trig_t        endTrig;               // W:  end trig for connection event
  uint32        endTime;               // W:  time for end trigger
} slaveParam_t;

// Connection Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue
  dataEntryQ_t *pTXQ;                  // W:  ptr to Tx queue
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  seqNumStat_t  seqStat;               // RW: sequence status bit field
  uint8         maxNAck;               // W:  max number of NACKs allowed
  uint8         maxTxPkt;              // W:  max number of Tx pkts allowed
  uint32        accessAddress;         // W:  access address used in connection
  uint8         crcInit[BLE_CRC_LEN];  // W:  CRC init value
  trig_t        timeoutTrig;           // W:  timeout trig for first Rx operation
  uint32        timeoutTime;           // W:  time for timeout trigger
  uint8         reserved[3];           // unused
  trig_t        endTrig;               // W:  end trig for connection event
  uint32        endTime;               // W:  time for end trigger
} linkParam_t;

// Generic Rx Command Parameters
PACKED_TYPEDEF_STRUCT
{
  dataEntryQ_t *pRXQ;                  // W:  ptr to Rx queue, or NULL
  rxQCfg_t      rxCfg;                 // W:  rx queue configuration
  uint8         repeatMode;            // W:  end/restart after pkt Rx
  uint16        reserved;              // unused
  uint32        accessAddress;         // W:  access address used in connection
  uint8         crcInit[BLE_CRC_LEN];  // W:  CRC init value
  trig_t        endTrig;               // W:  end trig for Rx
  uint32        endTime;               // W:  time for end trigger
} rxTestParam_t;

// Test Tx Command Parameters
PACKED_TYPEDEF_STRUCT
{
  uint16        numPkts;               // W:  number of pkts to Tx
  uint8         payloadLen;            // W:  size of Tx pkt
  uint8         pktType;               // W:  packet type
  uint32        period;                // W:  inter-pkt time, in radio cycles
  txTestCfg_t   config;                // W:  override packet encoding
  uint8         byteVal;               // W:  override byte to Tx
  uint8         reserved;              // unused
  trig_t        endTrig;               // W:  end trig for Tx
  uint32        endTime;               // W:  time for end trigger
} txTestParam_t;

/*
** BLE Output Command Structures
*/

// Advertiser Command
PACKED_TYPEDEF_STRUCT
{
  uint16        nTxAdv;                // RW: num ADV*_IND Tx pkts
  uint8         nTxScanRsp;            // RW: num SCAN_RSP Tx pkts
  uint8         nRxScanReq;            // RW: num SCAN_REQ okay Rx pkts
  uint8         nRxConnReq;            // RW: num CONNECT_REQ okay Rx pkts
  uint8         reserved;              // unused
  uint16        nRxNok;                // RW: num not okay Rx pkts
  uint16        nRxIgn;                // RW: num okay Rx pkts ignored
  uint8         nRxBufFull;            // RW: num pkts discarded
  uint8         lastRssi;              // R:  RSSI of last Rx pkt
  uint32        timeStamp;             // R:  timestamp of last Rx pkt
} advOut_t;

// Scanner Command
PACKED_TYPEDEF_STRUCT
{
  uint16        nTxScanReq;            // RW: num SCAN_REQ Tx pkts
  uint16        nBoffScanReq;          // RW: num SCAN_REQ pkts not sent due to backoff
  uint16        nRxAdvOk;              // RW: num ADV*_IND okay Rx pkts
  uint16        nRxAdvIgn;             // RW: num ADV*_IND okay Rx pkts ignored
  uint16        nRxAdvNok;             // RW: num ADV*_IND not okay Rx pkts
  uint16        nRxScanRspOk;          // RW: num SCAN_RSP okay Rx pkts
  uint16        nRxScanRspIgn;         // RW: num SCAN_RSP okay Rx pkts ignored
  uint16        nRxScanRspNok;         // RW: num SCAN_RSP not okay Rx pkts
  uint8         nRxAdvBufFull;         // RW: num ADV*_IND pkts discarded
  uint8         nRxScanRspBufFull;     // RW: num SCAN_RSP pkts discarded
  uint8         lastRssi;              // R:  RSSI of last Rx pkt
  uint8         reserved;              // unused
  uint32        timeStamp;             // R:  timestamp of first Rx pkt (Slave Only)
} scanOut_t;

// Initiator Command
PACKED_TYPEDEF_STRUCT
{
  uint8         nTxConnReq;            // RW: num CONN_REQ Tx pkts
  uint8         nRxAdvOk;              // RW: num ADV*_IND okay Rx pkts
  uint16        nRxAdvIgn;             // RW: num ADV*_IND okay Rx pkts ignored
  uint16        nRxAdvNok;             // RW: num ADV*_IND not okay Rx pkts
  uint8         nRxAdvBufFull;         // RW: num ADV*_IND pkts discarded
  uint8         lastRssi;              // R:  RSSI of last Rx pkt
  uint32        timeStamp;             // R:  timestamp of first Rx pkt (Slave Only)
} initOut_t;

// Master or Slave Command
PACKED_TYPEDEF_STRUCT
{
  uint8         nTx;                   // RW: num Tx pkts
  uint8         nTxAck;                // RW: num Tx pkts Acked
  uint8         nTxCtrl;               // RW: num Tx ctrl pkts
  uint8         nTxCtrlAck;            // RW: num Tx ctrl pkts Acked
  uint8         nTxCtrlAckAck;         // RW: num Tx ctrl pkts Acked that were Acked
  uint8         nTxRetrans;            // RW: num retransmissions
  uint8         nTxEntryDone;          // RW: num pkts on Tx queue that are finished
  uint8         nRxOk;                 // RW: num okay Rx pkts
  uint8         nRxCtrl;               // RW: num okay Rx ctrl pkts
  uint8         nRxCtrlAck;            // RW: num okay Rx ctrl pkts Acked
  uint8         nRxNok;                // RW: num not okay Rx pkts
  uint8         nRxIgn;                // RW: num okay Rx pkts ignored
  uint8         nRxEmpty;              // RW: num okay Rx pkts with no payload
  uint8         nRxBufFull;            // RW: num pkts discarded
  uint8         lastRssi;              // R:  RSSI of last Rx pkt
  pktStat_t     pktStatus;             // RW: last pkt status and timestamp
  uint32        timeStamp;             // R:  timestamp of first Rx pkt (Slave Only)
} connOut_t;

// Generic Rx Command
PACKED_TYPEDEF_STRUCT
{
  uint16        nRxOk;                 // RW: num okay Rx pkts
  uint16        nRxNok;                // RW: num not okay Rx pkts
  uint16        nRxBufFull;            // RW: num ADV*_IND pkts discarded
  uint8         lastRssi;              // R:  RSSI of last Rx pkt
  uint8         reserved;              // unused
  uint32        timeStamp;             // R:  timestamp of first Rx pkt (Slave Only)
} rxOut_t;

// Test Tx Command
PACKED_TYPEDEF_STRUCT
{
  uint16        nTx;                   // RW: num Tx pkts
} txOut_t;

/*
** BLE Radio Immediate Commands
*/

// Update Advertising Data Command
PACKED_TYPEDEF_STRUCT
{
  uint16        cmdNum;                // W:  radio command number
  uint8         dataType;              // W:  Adv or Scan Rsp data
  uint8         dataLen;               // W:  length of update
  uint8        *pData;                 // W:  ptr to update data
  uint8        *pParams;               // W:  ptr to update parameters
} bleUpdateAdvData_t;


/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// Receive Queue
extern dataQ_t *rxDataQ;
extern dataQ_t *txDataQ;
extern dataEntryPtr_t (*rxRingBuf)[NUM_RX_DATA_ENTRIES];

// Advertising Data Structures
extern bleOpCmd_t       advCmd[];
extern advParam_t       advParam;
extern advOut_t         advOutput;
extern const uint8      advChan[];
extern const uint16     advEvt2Cmd[];
extern const uint8      advEvt2State[];

// Scan Data Structures
extern bleOpCmd_t       scanCmd;
extern scanParam_t      scanParam;
extern scanOut_t        scanOutput;

// Init Data Structures
extern bleOpCmd_t       initCmd;
extern initParam_t      initParam;
extern initOut_t        initOutput;
extern connReqData_t    connReqData;

#if defined(CTRL_V41_CONFIG) && (CTRL_V41_CONFIG & MST_SLV_CFG)

// Connection Data Structures
extern bleOpCmd_t       *linkCmd;
extern linkParam_t      *linkParam;

#else // !MST_SLV_CFG

// Slave Data Structures
extern bleOpCmd_t       slvCmd;
extern slaveParam_t     slvParam;

// Master Data Structures
extern bleOpCmd_t       *mstCmd;
extern masterParam_t    *mstParam;

#endif // CTRL_V41_CONFIG & MST_SLV_CFG

// Connection Output
extern connOut_t        connOutput;

// Direct Test Mode Data Structures
extern bleOpCmd_t       trxTestCmd;
extern txTestParam_t    txTestParam;
extern txOut_t          txTestOut;
extern rxTestParam_t    rxTestParam;
extern rxOut_t          rxTestOut;

// Modem Tests (TELECO)
extern rfOpCmd_TxTest_t txModemTestCmd;
extern rfOpCmd_RxTest_t rxModemTestCmd;

/*******************************************************************************
 * APIs
 */

// ISR Callbacks
extern void cpe0IntCback( void );
extern void cpe1IntCback( void );
extern void hwIntCback( void );

// Mailbox
extern void llSetupMailbox( void );

// RF HAL
extern void llSetupRfHal( void );
extern void llPatchCM0( void );

// RAT Channel
extern void llSetupRATChanCompare( uint8 ratChan, uint32 compareTime );
extern void llRatChanCBack_A( void );
extern void llRatChanCBack_B( void );
extern void llRatChanCBack_C( void );
extern void llRatChanCBack_D( void );

// Data Processing
extern dataEntryQ_t *llSetupScanDataEntryQueue( void );

extern dataEntryQ_t *llSetupInitDataEntryQueue( void );

extern dataEntryQ_t *llSetupAdvDataEntryQueue( void );

extern dataEntryQ_t *llSetupDataEntryQueue( void );

extern dataEntryQ_t *llSetupOneDataEntryQueueDyn( void );

extern dataEntryQ_t *llSetupConnRxDataEntryQueue( uint8 connId );

extern void          llProcessSlaveControlPacket( llConnState_t *connPtr,
                                                  uint8         *pBuf );

extern void          llProcessMasterControlPacket( llConnState_t *connPtr,
                                                   uint8         *pBuf );

extern uint8         llGetNumTxDataEntries( dataEntryQ_t *pDataEntryQ );

extern uint8         llGetTotalNumTxDataEntries( void );

extern void          llMoveTempTxDataEntries( llConnState_t *connPtr );

/*******************************************************************************
 */

#endif /* BLE_H */
