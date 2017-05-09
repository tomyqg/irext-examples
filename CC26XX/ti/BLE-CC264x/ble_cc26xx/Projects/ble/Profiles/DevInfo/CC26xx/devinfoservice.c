/**************************************************************************************************
  Filename:       devinfoservice.c
  Revised:        $Date: 2015-05-08 08:20:47 -0700 (Fri, 08 May 2015) $
  Revision:       $Revision: 43731 $

  Description:    This file contains the Device Information service.


  Copyright 2012 - 2015 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "gattservapp.h"

#include "devinfoservice.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Device information service
CONST uint8 devInfoServUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(DEVINFO_SERV_UUID), HI_UINT16(DEVINFO_SERV_UUID)
};

// System ID
CONST uint8 devInfoSystemIdUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SYSTEM_ID_UUID), HI_UINT16(SYSTEM_ID_UUID)
};

// Model Number String
CONST uint8 devInfoModelNumberUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(MODEL_NUMBER_UUID), HI_UINT16(MODEL_NUMBER_UUID)
};

// Serial Number String
CONST uint8 devInfoSerialNumberUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SERIAL_NUMBER_UUID), HI_UINT16(SERIAL_NUMBER_UUID)
};

// Firmware Revision String
CONST uint8 devInfoFirmwareRevUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(FIRMWARE_REV_UUID), HI_UINT16(FIRMWARE_REV_UUID)
};

// Hardware Revision String
CONST uint8 devInfoHardwareRevUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(HARDWARE_REV_UUID), HI_UINT16(HARDWARE_REV_UUID)
};

// Software Revision String
CONST uint8 devInfoSoftwareRevUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SOFTWARE_REV_UUID), HI_UINT16(SOFTWARE_REV_UUID)
};

// Manufacturer Name String
CONST uint8 devInfoMfrNameUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(MANUFACTURER_NAME_UUID), HI_UINT16(MANUFACTURER_NAME_UUID)
};

// IEEE 11073-20601 Regulatory Certification Data List
CONST uint8 devInfo11073CertUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(IEEE_11073_CERT_DATA_UUID), HI_UINT16(IEEE_11073_CERT_DATA_UUID)
};

// PnP ID
CONST uint8 devInfoPnpIdUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(PNP_ID_UUID), HI_UINT16(PNP_ID_UUID)
};


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

extern void* memcpy(void *dest, const void *src, size_t len);

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * Profile Attributes - variables
 */

// Device Information Service attribute
static CONST gattAttrType_t devInfoService = { ATT_BT_UUID_SIZE, devInfoServUUID };

// System ID characteristic
static uint8 devInfoSystemIdProps = GATT_PROP_READ;
static uint8 devInfoSystemId[DEVINFO_SYSTEM_ID_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

// Model Number String characteristic
static uint8 devInfoModelNumberProps = GATT_PROP_READ;
static uint8 devInfoModelNumber[DEVINFO_STR_ATTR_LEN+1] = "Model Number";

// Serial Number String characteristic
static uint8 devInfoSerialNumberProps = GATT_PROP_READ;
static uint8 devInfoSerialNumber[DEVINFO_STR_ATTR_LEN+1] = "Serial Number";

// Firmware Revision String characteristic
static uint8 devInfoFirmwareRevProps = GATT_PROP_READ;
static uint8 devInfoFirmwareRev[DEVINFO_STR_ATTR_LEN+1] = "Firmware Revision";

// Hardware Revision String characteristic
static uint8 devInfoHardwareRevProps = GATT_PROP_READ;
static uint8 devInfoHardwareRev[DEVINFO_STR_ATTR_LEN+1] = "Hardware Revision";

// Software Revision String characteristic
static uint8 devInfoSoftwareRevProps = GATT_PROP_READ;
static uint8 devInfoSoftwareRev[DEVINFO_STR_ATTR_LEN+1] = "Software Revision";

// Manufacturer Name String characteristic
static uint8 devInfoMfrNameProps = GATT_PROP_READ;
static uint8 devInfoMfrName[DEVINFO_STR_ATTR_LEN+1] = "Manufacturer Name";

// IEEE 11073-20601 Regulatory Certification Data List characteristic
static uint8 devInfo11073CertProps = GATT_PROP_READ;
static uint8 defaultDevInfo11073Cert[] =
{
  DEVINFO_11073_BODY_EXP,     // authoritative body type
  0x00,                       // authoritative body structure type
                              // authoritative body data follows below:
  'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'
};

// The length of this characteristic is not fixed
static uint8 *devInfo11073Cert = defaultDevInfo11073Cert;
static uint8 devInfo11073CertLen = sizeof(defaultDevInfo11073Cert);

// PnP ID characteristic
static uint8 devInfoPnpIdProps = GATT_PROP_READ;
static uint8 devInfoPnpId[DEVINFO_PNP_ID_LEN] =
{
  1,                                      // Vendor ID source (1=Bluetooth SIG)
  LO_UINT16(0x000D), HI_UINT16(0x000D),   // Vendor ID (Texas Instruments)
  LO_UINT16(0x0000), HI_UINT16(0x0000),   // Product ID (vendor-specific)
  LO_UINT16(0x0110), HI_UINT16(0x0110)    // Product version (JJ.M.N)
};

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t devInfoAttrTbl[] =
{
  // Device Information Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&devInfoService                  /* pValue */
  },

    // System ID Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoSystemIdProps
    },

      // System ID Value
      {
        { ATT_BT_UUID_SIZE, devInfoSystemIdUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoSystemId
      },

    // Model Number String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoModelNumberProps
    },

      // Model Number Value
      {
        { ATT_BT_UUID_SIZE, devInfoModelNumberUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoModelNumber
      },

    // Serial Number String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoSerialNumberProps
    },

      // Serial Number Value
      {
        { ATT_BT_UUID_SIZE, devInfoSerialNumberUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoSerialNumber
      },

    // Firmware Revision String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoFirmwareRevProps
    },

      // Firmware Revision Value
      {
        { ATT_BT_UUID_SIZE, devInfoFirmwareRevUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoFirmwareRev
      },

    // Hardware Revision String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoHardwareRevProps
    },

      // Hardware Revision Value
      {
        { ATT_BT_UUID_SIZE, devInfoHardwareRevUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoHardwareRev
      },

    // Software Revision String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoSoftwareRevProps
    },

      // Software Revision Value
      {
        { ATT_BT_UUID_SIZE, devInfoSoftwareRevUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoSoftwareRev
      },

    // Manufacturer Name String Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoMfrNameProps
    },

      // Manufacturer Name Value
      {
        { ATT_BT_UUID_SIZE, devInfoMfrNameUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoMfrName
      },

    // IEEE 11073-20601 Regulatory Certification Data List Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfo11073CertProps
    },

      // IEEE 11073-20601 Regulatory Certification Data List Value
      {
        { ATT_BT_UUID_SIZE, devInfo11073CertUUID },
        GATT_PERMIT_READ,
        0,
        defaultDevInfo11073Cert
      },

    // PnP ID Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &devInfoPnpIdProps
    },

      // PnP ID Value
      {
        { ATT_BT_UUID_SIZE, devInfoPnpIdUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) devInfoPnpId
      }
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t devInfo_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset,
                                     uint16 maxLen, uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Device Info Service Callbacks
CONST gattServiceCBs_t devInfoCBs =
{
  devInfo_ReadAttrCB, // Read callback function pointer
  NULL,               // Write callback function pointer
  NULL                // Authorization callback function pointer
};

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DevInfo_AddService
 *
 * @brief   Initializes the Device Information service by registering
 *          GATT attributes with the GATT server.
 *
 * @return  Success or Failure
 */
bStatus_t DevInfo_AddService( void )
{
  // Register GATT attribute list and CBs with GATT Server App
  return GATTServApp_RegisterService( devInfoAttrTbl,
                                      GATT_NUM_ATTRS( devInfoAttrTbl ),
                                      GATT_MAX_ENCRYPT_KEY_SIZE,
                                      &devInfoCBs );
}

/*********************************************************************
 * @fn      DevInfo_SetParameter
 *
 * @brief   Set a Device Information parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
     case DEVINFO_SYSTEM_ID:
      // verify length
      if (len == sizeof(devInfoSystemId))
      {
        memcpy(devInfoSystemId, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_MODEL_NUMBER:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoModelNumber, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoModelNumber, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    case DEVINFO_SERIAL_NUMBER:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoSerialNumber, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoSerialNumber, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_FIRMWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoFirmwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoFirmwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_HARDWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoHardwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoHardwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_SOFTWARE_REV:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoSoftwareRev, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoSoftwareRev, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_MANUFACTURER_NAME:
      // verify length, leave room for null-terminate char
      if (len <= DEVINFO_STR_ATTR_LEN)
      {
        memset(devInfoMfrName, 0, DEVINFO_STR_ATTR_LEN+1);
        memcpy(devInfoMfrName, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case DEVINFO_11073_CERT_DATA:
      {
        // Allocate buffer for new certification
        uint8 *pCert = ICall_malloc(len);
        if (pCert != NULL)
        {
          if (devInfo11073Cert != defaultDevInfo11073Cert)
          {
            // Free existing certification buffer
            ICall_free(devInfo11073Cert);
          }
          
          // Copy over new certification
          memcpy(pCert, value, len);
          
          // Update our globals
          devInfo11073Cert = pCert;
          devInfo11073CertLen = len;
        }
        else
        {
          ret = bleMemAllocError;
        }
      }
      break;

    case DEVINFO_PNP_ID:
      // verify length
      if (len == sizeof(devInfoPnpId))
      {
        memcpy(devInfoPnpId, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn      DevInfo_GetParameter
 *
 * @brief   Get a Device Information parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to get.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t DevInfo_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case DEVINFO_SYSTEM_ID:
      memcpy(value, devInfoSystemId, sizeof(devInfoSystemId));
      break;

    case DEVINFO_MODEL_NUMBER:
      memcpy(value, devInfoModelNumber, DEVINFO_STR_ATTR_LEN);
      break;
    case DEVINFO_SERIAL_NUMBER:
      memcpy(value, devInfoSerialNumber, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_FIRMWARE_REV:
      memcpy(value, devInfoFirmwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_HARDWARE_REV:
      memcpy(value, devInfoHardwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_SOFTWARE_REV:
      memcpy(value, devInfoSoftwareRev, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_MANUFACTURER_NAME:
      memcpy(value, devInfoMfrName, DEVINFO_STR_ATTR_LEN);
      break;

    case DEVINFO_11073_CERT_DATA:
      memcpy(value, devInfo11073Cert, devInfo11073CertLen);
      break;

    case DEVINFO_PNP_ID:
      memcpy(value, devInfoPnpId, sizeof(devInfoPnpId));
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn          devInfo_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t devInfo_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset,
                                     uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;
  uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

  // If the value offset of the Read Blob Request is greater than the
  // length of the attribute value, an Error Response shall be sent with
  // the error code Invalid Offset.
  switch (uuid)
  {
    case SYSTEM_ID_UUID:
      // verify offset
      if (offset > sizeof(devInfoSystemId))
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (sizeof(devInfoSystemId) - offset));

        // copy data
        memcpy(pValue, &devInfoSystemId[offset], *pLen);
      }
      break;

    case MODEL_NUMBER_UUID:
    case SERIAL_NUMBER_UUID:
    case FIRMWARE_REV_UUID:
    case HARDWARE_REV_UUID:
    case SOFTWARE_REV_UUID:
    case MANUFACTURER_NAME_UUID:
      {
        uint16 len = strlen((char *)(pAttr->pValue));
        
        // verify offset
        if (offset > len)
        {
          status = ATT_ERR_INVALID_OFFSET;
        }
        else
        {
          // determine read length (exclude null terminating character)
          *pLen = MIN(maxLen, (len - offset));

          // copy data
          memcpy(pValue, &(pAttr->pValue[offset]), *pLen);
        }
      }
      break;

    case IEEE_11073_CERT_DATA_UUID:
      // verify offset
      if (offset > devInfo11073CertLen)
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (devInfo11073CertLen - offset));

        // copy data
        memcpy(pValue, &devInfo11073Cert[offset], *pLen);
      }
      break;

    case PNP_ID_UUID:
      // verify offset
      if (offset > sizeof(devInfoPnpId))
      {
        status = ATT_ERR_INVALID_OFFSET;
      }
      else
      {
        // determine read length
        *pLen = MIN(maxLen, (sizeof(devInfoPnpId) - offset));

        // copy data
        memcpy(pValue, &devInfoPnpId[offset], *pLen);
      }
      break;

    default:
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  return ( status );
}


/*********************************************************************
*********************************************************************/
