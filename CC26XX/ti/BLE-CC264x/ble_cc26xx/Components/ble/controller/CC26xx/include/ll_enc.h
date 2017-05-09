/*******************************************************************************
  Filename:       ll_enc.h
  Revised:        $Date: 2011-08-22 08:41:40 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27235 $

  Description:    This file contains the Link Layer (LL) types, constants,
                  API's etc. for the Bluetooth Low Energy (BLE) Controller
                  CCM encryption and decryption.

                  This API is based on ULP BT LE D09R23.

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

#ifndef LL_ENC_H
#define LL_ENC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "bcomdef.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define LL_ENC_TX_DIRECTION_MASTER   1
#define LL_ENC_TX_DIRECTION_SLAVE    0
#define LL_ENC_RX_DIRECTION_MASTER   0
#define LL_ENC_RX_DIRECTION_SLAVE    1

#define LL_ENC_DATA_BANK_MASK        0xFF7F

#define LL_ENC_TRUE_RAND_BUF_SIZE    ((LL_ENC_IV_LEN/2) + (LL_ENC_SKD_LEN/2))

// Generate Session Key using LTK for key and SKD for plaintext.
#define LL_ENC_GenerateSK            MAP_LL_ENC_AES128_Encrypt

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// ALT: Remove caching as CC26xx is fast enough.
extern uint8 cachedTRNGdata[];

/*******************************************************************************
 * Functions
 */

extern void  LL_ENC_Init( void );
extern void  LL_ENC_LoadKey( uint8 *key );
extern void  LL_ENC_EncryptMsg( uint8 *nonce, uint8 pktHdr, uint8 pktLen, uint8 *pBuf );
extern void  LL_ENC_DecryptMsg( uint8 *nonce, uint8 pktHdr, uint8 pktLen, uint8 *pBuf, uint8 *mic );

// Random Number Generation
extern uint8 LL_ENC_GeneratePseudoRandNum( void );
extern uint8 LL_ENC_GenerateTrueRandNum( uint8 *buf, uint8 len );

// CCM Encryption
extern void  LL_ENC_AES128_Encrypt( uint8 *key, uint8 *plaintext,  uint8 *ciphertext );
extern void  LL_ENC_AES128_Decrypt( uint8 *key, uint8 *ciphertext, uint8 *plaintext );
extern void  LL_ENC_ReverseBytes( uint8 *buf, uint8 len );
extern void  LL_ENC_GenDeviceSKD( uint8 *SKD );
extern void  LL_ENC_GenDeviceIV( uint8 *IV );
extern void  LL_ENC_GenerateNonce( uint32 pktCnt, uint8 direction, uint8 *nonce );
extern void  LL_ENC_Encrypt( llConnState_t *connPtr, uint8 pktHdr, uint8 pktLen, uint8 *pBuf );
extern uint8 LL_ENC_Decrypt( llConnState_t *connPtr, uint8 pktHdr, uint8 pktLen, uint8 *pBuf );

#ifdef __cplusplus
}
#endif

#endif /* LL_ENC_H */
