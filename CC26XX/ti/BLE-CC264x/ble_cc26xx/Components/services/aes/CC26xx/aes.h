/**
  @file  aes.h
  @brief AES service (OS dependent) interface

  <!--
  Revised:        $Date: 2015-07-22 10:45:09 -0700 (Wed, 22 Jul 2015) $
  Revision:       $Revision: 44392 $

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  -->
*/

#ifndef     _AES_H_
#define     _AES_H_

#define     STATE_BLENGTH   16      // Number of bytes in State
#define     KEY_BLENGTH     16      // Number of bytes in Key
#define     KEY_EXP_LENGTH  176     // Nb * (Nr+1) * 4

//
// Key Functions
//
extern void ssp_KeyInit_Sw( uint8_t * );
extern void sspKeyExpansion_Sw( uint8_t *, uint8_t * );
extern void RoundKey_Sw( uint8_t *, uint8_t );

//
// Encryption Functions
//
extern void sspAesEncryptKeyExp_Sw( uint8_t *, uint8_t * );
extern void sspAesEncryptBasic_Sw( uint8_t *, uint8_t * );

extern void AddRoundKeySubBytes_Sw( uint8_t *, uint8_t * );
extern void ShiftRows_Sw( uint8_t * );
extern void MixColumns_Sw( uint8_t * );

extern void (*pSspAesEncrypt_Sw)( uint8_t *, uint8_t * );
extern void sspKeyExpansion_Sw (uint8_t *, uint8_t *);

extern void sspAesEncrypt_Sw (uint8_t *, uint8_t *);
extern void sspAesEncryptKeyExp_Sw (uint8_t *, uint8_t *);
extern void sspAesEncryptBasic_Sw (uint8_t *, uint8_t *);

//
// Decryption Functions
//
#ifdef INCLUDE_AES_DECRYPT
  extern uint8_t FFMult_Sw( uint8_t, uint8_t );
  extern void sspAesDecrypt_Sw( uint8_t *, uint8_t * );
  extern void InvSubBytes_Sw( uint8_t * );
  extern void InvShiftRows_Sw( uint8_t * );
  extern void InvMixColumns_Sw( uint8_t * );
#endif  // INCLUDE_AES_DECRYPT

#ifdef INCLUDE_AES_DECRYPT
#ifdef USE_KEY_EXPANSION
  extern void InvAddRoundKey_Sw( uint8_t, uint8_t *, uint8_t * );
#else
  extern void InvAddRoundKey_Sw( uint8_t, uint8_t *, uint8_t *, uint8_t * );
  extern void InvRoundKey_Sw( uint8_t *, uint8_t, uint8_t * );
#endif  // USE_KEY_EXPANSION
#endif  // INCLUDE_AES_DECRYPT
  

// Following are definitions needed for CC2430 hardware AES engine


#define AES_BUSY    0x08
#define ENCRYPT     0x00
#define DECRYPT     0x01

// Macro for setting the mode of the AES operation
#define AES_SETMODE(mode) do { ENCCS &= ~0x70; ENCCS |= mode; } while (0)

// _mode_ is one of
#define CBC         0x00
#define CFB         0x10
#define OFB         0x20
#define CTR         0x30
#define ECB         0x40
#define CBC_MAC     0x50

// Macro for starting or stopping encryption or decryption
#define AES_SET_ENCR_DECR_KEY_IV(mode) \
   do {                                \
    ENCCS = (ENCCS & ~0x07) | mode     \
   } while(0)

// Where _mode_ is one of
#define AES_ENCRYPT     0x00;
#define AES_DECRYPT     0x02;
#define AES_LOAD_KEY    0x04;
#define AES_LOAD_IV     0x06;

// Macro for starting the AES module for either encryption, decryption,
// key or initialization vector loading.
#define AES_START()     ENCCS |= 0x01

// End of CC2430 hardware AES engine definitions

#endif  // _AES_H_

