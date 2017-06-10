/*******************************************************************************
  Filename:       bleUserConfig.h
  Revised:        $Date: 2014-05-12 13:33:44 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38503 $

  Description:    This file contains user configurable variables for the BLE
                  Controller and Host. This file is also used for defining the
                  type of RF Front End used with the TI device, such as using
                  a Differential front end with External Bias, etc. Please see
                  below for more detail.

                  To change the default values of configurable variables:
                    - Include the followings in your application main.c file:
                      #ifndef USE_DEFAULT_USER_CFG

                      #include "bleUserConfig.h"

                      // BLE user defined configuration
                      bleUserCfg_t user0Cfg = BLE_USER_CFG;

                      #endif // USE_DEFAULT_USER_CFG
                    - Set the preprocessor symbol MAX_NUM_BLE_CONNS, MAX_NUM_PDU,
                      MAX_PDU_SIZE, L2CAP_NUM_PSM or L2CAP_NUM_CO_CHANNELS
                      to a desired value in your application project.
                    - Include "bleUserConfig.h" in your stack OSAL_ICallBle.c
                      file.
                    - Call setBleUserConfig at the start of stack_main. Actually,
                      it is okay to set the variables anywhere in stack_main as
                      long as it is BEFORE osal_init_system, but best to set at
                      the very start of stack_main.

                  Note: User configurable variables are only used during the
                        initialization of the Controller and Host. Changing
                        the values of these variables after this will have no
                        effect.

                  Note: To use the default user configurable variables, define
                        the preprocessor symbol USE_DEFAULT_USER_CFG in your
                        application project.

                  For example:
                    - In your application main.c, include:
                      #ifndef USE_DEFAULT_USER_CFG

                      #include "bleUserConfig.h"

                      // BLE user defined configuration
                      bleUserCfg_t user0Cfg = BLE_USER_CFG;
                      #endif // USE_DEFAULT_USER_CFG
                    - In your application project, set the preprocessor symbol
                      MAX_NUM_BLE_CONNS to 1 to change the maximum number of BLE
                      connections to 1 from the default value of 3.
                    - In your stack OSAL_ICallBle.c file, call setBleUserCfg to
                      update the user configuration variables:
                      #include "bleUserConfig.h"
                      :
                      int stack_main( void *arg )
                      {
                        setBleUserConfig( (bleUserCfg_t *)arg );
                        :
                      }

                  Default values:
                    maxNumConns     : 3
                    maxNumPDUs      : 6
                    maxPduSize      : 27
                    maxNumPSM       : 3
                    maxNumCoChannels: 3


  Copyright 2014 - 2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef BLE_USER_CONFIG_H
#define BLE_USER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include "bleDispatch.h"
#include "rfhal.h"

/*******************************************************************************
 * MACROS
 */

//
// Tx Power
//
// PG1:              PG2:
// 7..6 | 5..0         15..8   | 7..6 | 5..0
//  GC  |  IB        tempCoeff |  GC  |  IB
//
#if defined( CC26XX_PG1 )
#define GEN_TX_POWER_VAL( ib, gc, tc )                                         \
  (uint16)(((ib) & 0x3F) | (((gc) & 0x03) << 6) | (((0)&0xFF) << 8))
#elif defined( CC26XX ) || defined( CC13XX )
#define GEN_TX_POWER_VAL( ib, gc, tc )                                         \
  (uint16)(((ib) & 0x3F) | (((gc) & 0x03) << 6) | (((tc)&0xFF) << 8))
#else // unknown device
  #error "ERROR: Unknown device!"
#endif // CC26XX_PG1

/*******************************************************************************
 * CONSTANTS
 */

// Defines only required by Application.
#if !(defined( CTRL_CONFIG ) || defined( HOST_CONFIG ))

#include "board.h"

// RF Front End Settings
// Note: The use of these values completely depends on how the PCB is laid out.
//       Please see Device Package and Evaluation Module (EM) Board below.
#define RF_FE_DIFFERENTIAL              0
#define RF_FE_SINGLE_ENDED_RFP          1
#define RF_FE_SINGLE_ENDED_RFN          2
#define RF_FE_ANT_DIVERSITY_RFP_FIRST   3
#define RF_FE_ANT_DIVERSITY_RFN_FIRST   4
#define RF_FE_SINGLE_ENDED_RFP_EXT_PINS 5
#define RF_FE_SINGLE_ENDED_RFN_EXT_PINS 6
//
#define RF_FE_INT_BIAS                  (0<<3)
#define RF_FE_EXT_BIAS                  (1<<3)

// Tx Power
#define TX_POWER_5_DBM                   5
#define TX_POWER_4_DBM                   4
#define TX_POWER_3_DBM                   3
#define TX_POWER_2_DBM                   2
#define TX_POWER_1_DBM                   1
#define TX_POWER_0_DBM                   0
#define TX_POWER_MINUS_3_DBM             -3
#define TX_POWER_MINUS_6_DBM             -6
#define TX_POWER_MINUS_9_DBM             -9
#define TX_POWER_MINUS_12_DBM            -12
#define TX_POWER_MINUS_15_DBM            -15
#define TX_POWER_MINUS_18_DBM            -18
#define TX_POWER_MINUS_21_DBM            -21

// Maximum number of BLE connections. It should be set based on the
// device GAP role. Here're some recommended values:
//      * Central:     3
//      * Peripheral:  1
//      * Observer:    0
//      * Broadcaster: 0
#ifndef MAX_NUM_BLE_CONNS
  #define MAX_NUM_BLE_CONNS             1
#endif

// Maximum number of BLE HCI PDUs. If the maximum number connections (above)
// is set to 0 then this number should also be set to 0.
#ifndef MAX_NUM_PDU
  #define MAX_NUM_PDU                   10
#endif

// Maximum size in bytes of the BLE HCI PDU. Valid range: 27 to 255
// The maximum ATT_MTU is MAX_PDU_SIZE - 4.
#ifndef MAX_PDU_SIZE
  #define MAX_PDU_SIZE                  27
#endif

// Maximum number of L2CAP Protocol/Service Multiplexers (PSM)
#ifndef L2CAP_NUM_PSM
  #define L2CAP_NUM_PSM                 3
#endif

// Maximum number of L2CAP Connection Oriented Channels
#ifndef L2CAP_NUM_CO_CHANNELS
  #define L2CAP_NUM_CO_CHANNELS         3
#endif

//
// Device Package and Evaluation Module (EM) Board
//
// The CC26xx device comes in three types of packages: 7x7, 5x5, 4x4.
// For each package, the user may change how the RF Front End (FE) is
// configured. The possible FE settings are provided as a set of defines.
// (The user can also set the FE bias, the settings of which are also provided
// as defines.) The user can change the value of RF_FE_MODE_AND_BIAS to
// configure the RF FE as desired. However, while setting the FE configuration
// determines how the device is configured at the package, it is the PCB the
// device is mounted on (the EM) that determines how those signals are routed.
// So while the FE is configurable, how signals are used is fixed by the EM.
// As can be seen, the value of RF_FE_MODE_AND_BIAS is organized by the EM
// board as defined by EMs produced by Texas Instruments Inc. How the device
// is mounted, routed, and configured for a user product would of course be
// user defined, and the value of RF_FE_MODE_AND_BIAS would have to be set
// accordingly; the user could even dispense with the conditional board
// compiles entirely. So too with the usage of the Tx Power tables. As can be
// seen in bleUserConfig.c, there are two tables, one for packages using a
// differential FE, and one for single-end. This too has been organized by TI
// defined EMs and would have to be set appropriately by the user.
//
// For example:
// Let's say you decided to build several boards using the CC26xx 4x4 package.
// For one board, you plan to use a differential RF FE, while on the other you
// wish to use a single ended RFN RF FE. You would then create your own board.h
// (located by your preprocessor Include path name) that contains one of two
// defines that you create (say MY_CC26xx_4ID and MY_CC26xx_4XS). Then you can
// define your own choice of RF FE conditionally based on these defines
// (completely replacing those shown below), as follows:
//
//#if defined( MY_CC26xx_4ID )
//
//  #define RF_FE_MODE_AND_BIAS           ( RF_FE_DIFFERENTIAL |               \
//                                          RF_FE_INT_BIAS )
//#elif defined( MY_CC26xx_4XS )
//
//  #define RF_FE_MODE_AND_BIAS           ( RF_FE_SINGLE_ENDED_RFN |           \
//                                          RF_FE_EXT_BIAS )
//#else // unknown device package
// :
//
// In this way, you can define your own board I/O settings, and configure your
// RF FE based on your own board defines.
//
// For additional details and examples, please see the Software Developer's
// Guide.
//

// RF Front End Mode and Bias Configuration
#if defined( CC2650EM_7ID )

#if defined( CC13XX )
  #define RF_FE_MODE_AND_BIAS           ( RF_FE_DIFFERENTIAL |                 \
                                          RF_FE_EXT_BIAS)
#else
  #define RF_FE_MODE_AND_BIAS           ( RF_FE_DIFFERENTIAL |                 \
                                          RF_FE_INT_BIAS )
#endif // CC13XX

#elif defined( CC2650EM_5XD ) || defined( CC2650EM_4XD )

  #define RF_FE_MODE_AND_BIAS           ( RF_FE_DIFFERENTIAL |                 \
                                          RF_FE_EXT_BIAS)

#elif defined( CC2650EM_4XS )

  #define RF_FE_MODE_AND_BIAS           ( RF_FE_SINGLE_ENDED_RFP |             \
                                          RF_FE_EXT_BIAS )
#else // unknown device package

#error "***BLE USER CONFIG BUILD ERROR*** Unknown package type!"

#endif // CC2650EM_7ID

#ifndef PM_STARTUP_MARGIN
  #define PM_STARTUP_MARGIN             300
#endif


#define BLE_USER_CFG                    { MAX_NUM_BLE_CONNS,    \
                                          MAX_NUM_PDU,          \
                                          MAX_PDU_SIZE,         \
                                          RF_FE_MODE_AND_BIAS,  \
                                          rfRegTbl,             \
                                          &txPwrTbl,            \
                                          L2CAP_NUM_PSM,        \
                                          L2CAP_NUM_CO_CHANNELS,\
                                          &pfnBMAlloc,          \
                                          &pfnBMFree,           \
                                          PM_STARTUP_MARGIN, }

// Make sure there's enough heap needed for BLE connection Tx buffers, which
// is based on MAX_PDU_SIZE and MAX_NUM_PDU configured by the application.
// The heap memory needed for BLE connection Tx buffers should not be more
// that 1/3 of the total ICall heap size (HEAPMGR_SIZE).
//
//  Note: Over the Air (OTA) PDU Size = 27, and LL Header Size = 14
//
#if ( MAX_NUM_BLE_CONNS > 0 ) && !defined( NO_HEAPSIZE_VALIDATE )
  #if  ( ( ( ( ( MAX_PDU_SIZE / 27 ) + 1 ) * MAX_NUM_PDU ) * ( 27 + 14 ) ) > ( HEAPMGR_SIZE / 3 ) )
    #warning Not enough heap for configured MAX_NUM_PDU and MAX_PDU_SIZE! Adjust HEAPMGR_SIZE.
  #endif
#endif

#endif // !(CTRL_CONFIG | HOST_CONFIG)

/*******************************************************************************
 * TYPEDEFS
 */

PACKED_TYPEDEF_CONST_STRUCT
{
  int8   Pout;
  uint16 txPwrVal;
} txPwrVal_t;

PACKED_TYPEDEF_CONST_STRUCT
{
  txPwrVal_t *txPwrValsPtr;
  uint8       numTxPwrVals;
  int8        defaultTxPwrVal;
} txPwrTbl_t;

//#endif // !(CTRL_CONFIG | HOST_CONFIG)

typedef struct
{
  uint8_t        maxNumConns;        // Max number of BLE connections
  uint8_t        maxNumPDUs;         // Max number of BLE PDUs
  uint8_t        maxPduSize;         // Max size of the BLE PDU.
  uint8_t        rfFeModeBias;       // RF Front End Mode and Bias (based on package)
  regOverride_t *rfRegTbl;           // RF Override Register Table
  txPwrTbl_t    *txPwrTbl;           // Tx Power Table
  uint8_t        maxNumPSM;          // Max number of L2CAP Protocol/Service Multiplexers
  uint8_t        maxNumCoChannels;   // Max number of L2CAP Connection Oriented Channels
  pfnBMAlloc_t   *pfnBMAlloc;        // BM allocator function pointer
  pfnBMFree_t    *pfnBMFree;         // BM de-allocator function pointer
  uint32_t       startupMarginUsecs; // power management MARGIN
} bleUserCfg_t;

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

extern       regOverride_t rfRegTbl[];
extern const txPwrTbl_t    txPwrTbl;
//
extern pfnBMAlloc_t pfnBMAlloc;
extern pfnBMFree_t  pfnBMFree;

/*********************************************************************
 * FUNCTIONS
 */

extern void setBleUserConfig( bleUserCfg_t *userCfg );

#ifdef __cplusplus
}
#endif

#endif /* BLE_USER_CONFIG_H */
