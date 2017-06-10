/*******************************************************************************
  Filename:       ll_config.h
  Revised:        $Date: 2011-12-15 13:45:50 -0800 (Thu, 15 Dec 2011) $
  Revision:       $Revision: 28688 $

  Description:    This file contains the BLE link layer configuration table
                  macros, constants, typedefs and externs.

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

#ifndef LL_CONFIG_H
#define LL_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "ll_userConfig.h"

/*******************************************************************************
 * MACROS
 */

#define CRC_SUFFIX_PRESENT()  (llConfigTable.rxPktSuffixPtr->suffixSel & SUFFIX_CRC_FLAG)
#define RSSI_SUFFIX_PRESENT() (llConfigTable.rxPktSuffixPtr->suffixSel & SUFFIX_RSSI_FLAG)

/*******************************************************************************
 * CONSTANTS
 */

// Receive Suffix Data Sizes
#define SUFFIX_SIZE_NONE                0
#define SUFFIX_CRC_SIZE                 3
#define SUFFIX_RSSI_SIZE                1
#define SUFFIX_STATUS_SIZE              1
#define SUFFIX_TIMESTAMP_SIZE           4
#define SUFFIX_MAX_SIZE                 (SUFFIX_CRC_SIZE    +                  \
                                         SUFFIX_RSSI_SIZE   +                  \
                                         SUFFIX_STATUS_SIZE +                  \
                                         SUFFIX_TIMESTAMP_SIZE)

#define SUFFIX_FLAG_NONE                0
#define SUFFIX_CRC_FLAG                 1
#define SUFFIX_RSSI_FLAG                2
#define SUFFIX_STATUS_FLAG              4
#define SUFFIX_TIMESTAMP_FLAG           8

// Receive Suffix Flags
#define ADV_SUFFIX_FLAGS                (SUFFIX_FLAG_NONE)                          // Connect Req and Scan Req Packets
#define SCAN_SUFFIX_FLAGS               (SUFFIX_RSSI_FLAG)                          // Adv and Scan Rsp Packets
#define INIT_SUFFIX_FLAGS               (SUFFIX_FLAG_NONE)                          // Adv Packets
#define LINK_SUFFIX_FLAGS               (SUFFIX_RSSI_FLAG + SUFFIX_TIMESTAMP_FLAG)  // Data and Control Packets

// Suffix Sizes
#define ADV_SUFFIX_SIZE                 (SUFFIX_SIZE_NONE)
#define SCAN_SUFFIX_SIZE                (SUFFIX_RSSI_SIZE)
#define INIT_SUFFIX_SIZE                (SUFFIX_SIZE_NONE)
#define LINK_SUFFIX_SIZE                (SUFFIX_RSSI_SIZE + SUFFIX_TIMESTAMP_SIZE)

// Number of Link Connections
#define MAX_NUM_BLE_CONNS               3

// Number of TX Data Entries
#define MAX_NUM_TX_ENTRIES              8

// Number Data Entries
#define NUM_RX_DATA_ENTRIES             4

// Max Data Size
// Note: When greater than 27, fragmenation is used.
#define MAX_DATA_SIZE                   27

  // Number Scan Entries
#define NUM_RX_SCAN_ENTRIES             4

// Packets Per Event
#define UNLIMITED_PKTS_PER_EVENT        0
#define ONE_PKT_PER_EVENT               1

#define LL_STARTUP_MARGIN               2100

// RF Operation Pointer and RF Front End Settings for Reset and Wake
#if defined( CC26XX_PG1 )

  #define RF_OP_PTR_LOCATION            0x21000070
  //
  #define RF_SETUP_CONFIG_ON_RESET      0  // not used
  #define RF_SETUP_CONFIG_ON_WAKE       0  // not used

#elif defined( CC26XX ) || defined( CC13XX )

  #define RF_OP_PTR_LOCATION            0x210000EC
  //
  #define RF_SETUP_CONFIG_ON_RESET      0x0000
  #define RF_SETUP_CONFIG_ON_WAKE       0x02D0

#else // unknown device

  #error BLE Build Configuration Error - Unkonwn Device!

#endif // CC26XX_PG1

/*******************************************************************************
 * TYPEDEFS
 */

// This structure is used to specify Slave RF timing margins. All values are
// in Radio Timer ticks (i.e. 250ns ticks).
//
// preRfMargin: This is the amount of time to subtract from the RF operation's
//              start time (i.e. how much earlier the RF operation will start),
//              and is used to account for the Frequency Synthesizer Calibration
//              time and Rx settle time (i.e. Rx Ramp time) such that the radio
//              is ready to receive.
// postRfMargin This is the amount of time to add to the duration of Rx while
//              waiting for packet synch. This effectively extends the receive
//              window.
PACKED_TYPEDEF_STRUCT
{
  uint8 preRfMargin;
  uint8 postRfMargin;
} slvRfMargin_t;

PACKED_TYPEDEF_STRUCT
{
  uint8 suffixSel;
  uint8 suffixSize;
} pktSuffix_t;

//PACKED typedef uint8 pktSize_t;
typedef uint8 pktSize_t;

//PACKED typedef uint8 wlSize_t;
typedef uint8 wlSize_t;

PACKED_TYPEDEF_STRUCT
{
  uint8 maxMstPktsPerEvt;
  uint8 maxSlvPktsPerEvt;
} maxPktsPerEvt_t;

typedef void (patchCM0_t)(void);

PACKED_TYPEDEF_STRUCT
{
  uint16 resetRfCfgVal;
  uint16 wakeRfCfgVal;
} rfCfgVal_t;

//PACKED typedef uint32 rfOp_t;
typedef uint32 rfOp_t;

PACKED_TYPEDEF_STRUCT
{
  const uint8           *placeHolder0;
  patchCM0_t            *patchCM0Ptr;
  const rfOp_t          *rfOpPtr;
  const rfCfgVal_t      *rfCfgValPtr;
  const uint8           *placeHolder1;
  //const pktSize_t     *rxPktSizePtr;
  //const pktSize_t     *advPktSizePtr;
  //const pktSize_t     *scanPktSizePtr;
  //const pktSize_t     *initPktSizePtr;
  const pktSuffix_t     *rxPktSuffixPtr;
  const pktSuffix_t     *advPktSuffixPtr;
  const pktSuffix_t     *scanPktSuffixPtr;
  const pktSuffix_t     *initPktSuffixPtr;
  const wlSize_t        *wlSizePtr;
  const maxPktsPerEvt_t *maxPktsPerEvtPtr;
  const llUserCfg_t     *userCfgPtr;
} llCfgTable_t;

/*******************************************************************************
 * LOCAL VARIABLES
 */


/*******************************************************************************
 * GLOBAL VARIABLES
 */

extern uint8 numTxDataBufs;
extern uint8 maxNumConns;
extern uint8 maxPduSize;
extern uint8 rfFeModeBias;
//
extern const llCfgTable_t llConfigTable;

#ifdef __cplusplus
}
#endif

#endif /* LL_CONFIG_H */
