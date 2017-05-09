/*******************************************************************************
  Filename:       bleUserConfig.c
  Revised:        $Date: 2015-12-07 14:22:14 -0800 (Mon, 07 Dec 2015) $
  Revision:       $Revision: 44564 $

  Description:    This file contains user configurable variables for the BLE
                  Application.

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
#include "bleUserConfig.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

// Tx Power
#define NUM_TX_POWER_VALUES (sizeof( txPowerTable ) / sizeof( txPwrVal_t ))

// Default Tx Power Index
#define DEFAULT_TX_POWER               7

// Override NOP
#define OVERRIDE_NOP                   0xC0000001

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

#if defined( USE_FPGA )
#else  // CC26xx Normal Package with Flash Settings for 48 MHz device
  #if defined( CC26XX_PG1 )
  // Override Tx Shape
  #pragma data_alignment=4
  static uint8_t txShape[] =
    {   0,   0,   0,   0,
        1,   1,   3,   5,
        8,  14,  21,  31,
       45,  61,  79,  99,
      119, 139, 158, 174,
      186, 196, 202, 205};
  #elif defined( CC26XX ) || defined( CC13XX )
  #else // unknown device
    #error "ERROR: Unknown device!"
  #endif // CC26XX_PG1
#endif // USE_FPGA

// RF Override Registers
// Note: Used with CMD_RADIO_SETUP; called at boot time and after wake.
// Note: Must be in RAM as these overrides may need to be modified at runtime
//       based on temperature compensation, although it is possible this may
//       be automated in CM0 in PG2.0.
#if defined( USE_FPGA )
  regOverride_t rfRegTbl[] =
    { 0x00015164,
      0x00041110,
      0x00000083,
      0x016302A3,
      0x00800403,
      0x80000303,
      0xFFFFFFFF };
#else // CC26xx Normal Package with Flash Settings for 48 MHz device
  #if defined( CC26XX_PG1 )
    regOverride_t rfRegTbl[] =
    { 0x00001107, // Enable MCE and RFE patch
      0x0000401C,
      0x40014019,
      0x84000000,
      0x4001402D,
      0x00608202,
      0x40014035,
      0x177F4804,
      0x00604064,
      0x25E00423,
      //0x000006E3,  // For 48 MHz crystal ONLY; remove for 24MHz
      0xC00C0061,
      0x007F4128,
      0x01343010,
      0x0001F800,
      0x04000243, // Upper halfword is freq synth cal timeout in 250ns ticks. Default is 256us.
      0x014C02A3, // two MSBs adjust T_IFS down by 0.25us per increment
      0x01700263, // Adjust start of Rx after T_IFS to compensate for extra tone in fromt of packet
      0xC004000D, // New Tx shape - address in RAM given on next word
      (uint32)txShape, // Address of new Tx shape
      0x00606098, // set Tx Power to 0dBm
      0xFFFFFFFF };
  #elif defined( CC26XX )
    regOverride_t rfRegTbl[] =
    { 0x00001007, // Run patched RFE code from RAM
      0x00354038, // Synth: Set RTRIM (POTAILRESTRIM) to 5
      0x4001402D, // Synth: Correct CKVD latency setting (address)
      0x00608402, // Synth: Correct CKVD latency setting (value)
      0x4001405D, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address)
      0x1801F800, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value)
      0x000784A3, // Synth: Set FREF = 3.43 MHz (24 MHz / 7)
      0xA47E0583, // Synth: Set loop bandwidth after lock to 80 kHz (K2)
      0xEAE00603, // Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB)
      0x00010623, // Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB)
      0x00456088, // Adjust AGC reference level
      0x013800C3, // Use enhanced BLE shape
      0x02010403, // Synth: Use 24 MHz XOSC as synth clock, enable phase error discard feature
      0x00008463, // Synth: modify phase error discard threshold factor
      0x00388473, // Synth: modify phase error discard threshold Offset
      0x40014035, // Set phase error error discard count to 1 (default2) to get faster settling in TX (address)
      0x177F0408, // Set phase error error discard count to 1 (default2) to get faster settling in TX (value)
      0x036052AC, // Add 6 us extra to tone in front of packet
      0x01AD02A3, // Compensate for 6 us added to tone in front of packet
      0x01680263, // Compensate for 6 us added to tone in front of packet
      0xFFFFFFFF };
  #elif defined( CC13XX )
    //#if defined( CC1350EM_7XD )
    #if defined( CC2650EM_7ID )
      regOverride_t rfRegTbl[] =
      { 0x003A4038, // Synth: Set RTRIM (POTAILRESTRIM) to 10
        0x7F004020, // Synth: Set bottom fine code to 0 due to modified fine bank (fine top code unchanged at 127)
        0x00404064, // Synth: Set fine start code to 0x40 due to modified fine bank
        0xC0040141, // Synth: Set K1 to compensate for modified fine bank
        0x0533B107, // Synth: New K1 value
        0x000784A3, // Synth: Set FREF = 3.43 MHz (24 MHz / 7)
        0xA47E0583, // Synth: Set loop bandwidth after lock to 80 kHz (K2)
        0xEAE00603, // Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB)
        0x00010623, // Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB)
        0x841F0002, // Synth: No COMP_CAP
        0x00456088, // Adjust AGC reference level
        0x013800C3, // Use enhanced BLE shape
        0xFFFFFFFF };
    #endif // package type
  #else // unknown device
    #error "ERROR: Unknown device!"
  #endif // CC26XX_PG1
#endif // USE_FPGA

//
// Tx Power Table Used Depends on Device Package
//

#if defined(CC2650EM_7ID) || defined(CC2650EM_5XD) ||  defined(CC2650EM_4XD)

// Differential Output
// ALT: Consider basing this direction on type of output (RF_FE_MODE_AND_BIAS)?

// Tx Power Values (Pout, IB, GC, TC)
const txPwrVal_t txPowerTable[] =
  { { TX_POWER_MINUS_21_DBM, GEN_TX_POWER_VAL( 0x07, 3, 0x0C ) },
    { TX_POWER_MINUS_18_DBM, GEN_TX_POWER_VAL( 0x09, 3, 0x0C ) },
    { TX_POWER_MINUS_15_DBM, GEN_TX_POWER_VAL( 0x0B, 3, 0x0C ) },
    { TX_POWER_MINUS_12_DBM, GEN_TX_POWER_VAL( 0x0B, 1, 0x14 ) },
    { TX_POWER_MINUS_9_DBM,  GEN_TX_POWER_VAL( 0x0E, 1, 0x19 ) },
    { TX_POWER_MINUS_6_DBM,  GEN_TX_POWER_VAL( 0x12, 1, 0x1D ) },
    { TX_POWER_MINUS_3_DBM,  GEN_TX_POWER_VAL( 0x18, 1, 0x25 ) },
    { TX_POWER_0_DBM,        GEN_TX_POWER_VAL( 0x21, 1, 0x31 ) },
    { TX_POWER_1_DBM,        GEN_TX_POWER_VAL( 0x14, 0, 0x42 ) },
    { TX_POWER_2_DBM,        GEN_TX_POWER_VAL( 0x18, 0, 0x4E ) },
    { TX_POWER_3_DBM,        GEN_TX_POWER_VAL( 0x1C, 0, 0x5A ) },
    { TX_POWER_4_DBM,        GEN_TX_POWER_VAL( 0x24, 0, 0x93 ) },
    { TX_POWER_5_DBM,        GEN_TX_POWER_VAL( 0x30, 0, 0x93 ) } };

#elif defined( CC2650EM_4XS )

// Single-Ended Output
// ALT: Consider basing this direction on type of output (RF_FE_MODE_AND_BIAS)?

// Tx Power Values (Pout, IB, GC, TC)
const txPwrVal_t txPowerTable[] =
  { { TX_POWER_MINUS_21_DBM, GEN_TX_POWER_VAL( 0x07, 3, 0x0C ) },
    { TX_POWER_MINUS_18_DBM, GEN_TX_POWER_VAL( 0x09, 3, 0x10 ) },
    { TX_POWER_MINUS_15_DBM, GEN_TX_POWER_VAL( 0x0B, 3, 0x14 ) },
    { TX_POWER_MINUS_12_DBM, GEN_TX_POWER_VAL( 0x0E, 3, 0x14 ) },
    { TX_POWER_MINUS_9_DBM,  GEN_TX_POWER_VAL( 0x0F, 1, 0x21 ) },
    { TX_POWER_MINUS_6_DBM,  GEN_TX_POWER_VAL( 0x14, 1, 0x29 ) },
    { TX_POWER_MINUS_3_DBM,  GEN_TX_POWER_VAL( 0x1C, 1, 0x35 ) },
    { TX_POWER_0_DBM,        GEN_TX_POWER_VAL( 0x2C, 1, 0x56 ) },
    { TX_POWER_1_DBM,        GEN_TX_POWER_VAL( 0x1F, 0, 0x6A ) },
    { TX_POWER_2_DBM,        GEN_TX_POWER_VAL( 0x29, 0, 0x9C ) } };

#else // unknown device package

#error "***BLE USER CONFIG BUILD ERROR*** Unknown package type!"

#endif // CC2650EM_7ID

// Tx Power Table
const txPwrTbl_t txPwrTbl = { txPowerTable,
                              NUM_TX_POWER_VALUES,  // max
                              DEFAULT_TX_POWER };   // default

/*******************************************************************************
 */
