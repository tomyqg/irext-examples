/**
  @headerfile:       central.h
  $Date: 2015-04-02 13:31:45 -0700 (Thu, 02 Apr 2015) $
  $Revision: 43348 $

  @mainpage TI BLE GAP Central Role for for RTOS Applications

  This GAP profile discovers and initiates connections.

  Copyright 2011 - 2015 Texas Instruments Incorporated. All rights reserved.

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
*/

#ifndef CENTRAL_H
#define CENTRAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "gap.h"

/*********************************************************************
 * CONSTANTS
 */

/** @defgroup GAPCENTRALROLE_PROFILE_PARAMETERS GAP Central Role Parameters
 * @{
 */
#define GAPCENTRALROLE_IRK                 0x400  //!< Identity Resolving Key. Read/Write. Size is uint8_t[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.
#define GAPCENTRALROLE_SRK                 0x401  //!< Signature Resolving Key. Read/Write. Size is uint8_t[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.
#define GAPCENTRALROLE_SIGNCOUNTER         0x402  //!< Sign Counter. Read/Write. Size is uint32_t. Default is 0.
#define GAPCENTRALROLE_BD_ADDR             0x403  //!< Device's Address. Read Only. Size is uint8_t[B_ADDR_LEN]. This item is read from the controller.
#define GAPCENTRALROLE_MAX_SCAN_RES        0x404  //!< Maximum number of discover scan results to receive. Default is 0 = unlimited.
/** @} End GAPCENTRALROLE_PROFILE_PARAMETERS */

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/**
 * Central Event Structure
 */
typedef union
{
  gapEventHdr_t             gap;                //!< GAP_MSG_EVENT and status.
  gapDeviceInitDoneEvent_t  initDone;           //!< GAP initialization done.
  gapDeviceInfoEvent_t      deviceInfo;         //!< Discovery device information event structure.
  gapDevDiscEvent_t         discCmpl;           //!< Discovery complete event structure.
  gapEstLinkReqEvent_t      linkCmpl;           //!< Link complete event structure.
  gapLinkUpdateEvent_t      linkUpdate;         //!< Link update event structure.
  gapTerminateLinkEvent_t   linkTerminate;      //!< Link terminated event structure.
} gapCentralRoleEvent_t;

/**
 * Central Event Callback Function
 *
 * return  TRUE if safe to deallocate event message, FALSE otherwise.
 */
typedef uint8_t (*pfnGapCentralRoleEventCB_t)
(
  gapCentralRoleEvent_t *pEvent         //!< Pointer to event structure.
);

/**
 * Central Callback Structure - must be setup by the application and used when
 *                              GAPCentralRole_StartDevice is called.
 */
typedef struct
{
  pfnGapCentralRoleEventCB_t  eventCB;  //!< Event callback.
} gapCentralRoleCB_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * API FUNCTIONS
 */

/*-------------------------------------------------------------------
 * Central Profile Public APIs
 */

/**
 * @defgroup CENTRAL_PROFILE_API Central Profile API Functions
 *
 * @{
 */

/**
 * @brief   Start the device in Central role.  This function is typically
 *          called once during system startup.
 *
 * @param   pAppCallbacks - pointer to application callbacks
 *
 * @return  SUCCESS: Operation successful.<BR>
 *          bleAlreadyInRequestedMode: Device already started.<BR>
 */
extern bStatus_t GAPCentralRole_StartDevice(gapCentralRoleCB_t *pAppCallbacks);

/**
 * @brief   Set a parameter in the Central Profile.
 *
 * @param   param - profile parameter ID: @ref GAPCENTRALROLE_PROFILE_PARAMETERS
 * @param   len - length of data to write
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type.
 *
 * @return  SUCCESS: Operation successful.<BR>
 *          INVALIDPARAMETER: Invalid parameter ID.<BR>
 */
extern bStatus_t GAPCentralRole_SetParameter(uint16_t param, uint8_t len, void *pValue);

/**
 * @brief   Get a parameter in the Central Profile.
 *
 * @param   param - profile parameter ID: @ref GAPCENTRALROLE_PROFILE_PARAMETERS
 * @param   pValue - pointer to buffer to contain the read data
 *
 * @return  SUCCESS: Operation successful.<BR>
 *          INVALIDPARAMETER: Invalid parameter ID.<BR>
 */
extern bStatus_t GAPCentralRole_GetParameter(uint16_t param, void *pValue);

/**
 * @brief   Terminate a link.
 *
 * @param   connHandle - connection handle of link to terminate
 *          or @ref GAP_CONN_HANDLE_DEFINES
 *
 * @return  SUCCESS: Terminate started.<BR>
 *          bleIncorrectMode: No link to terminate.<BR>
 */
extern bStatus_t GAPCentralRole_TerminateLink(uint16_t connHandle);

/**
 * @brief   Establish a link to a peer device.
 *
 * @param   highDutyCycle -  TRUE to high duty cycle scan, FALSE if not
 * @param   whiteList - determines use of the white list: @ref GAP_WHITELIST_DEFINES
 * @param   addrTypePeer - address type of the peer device: @ref GAP_ADDR_TYPE_DEFINES
 * @param   peerAddr - peer device address
 *
 * @return  SUCCESS: started establish link process.<BR>
 *          bleIncorrectMode: invalid profile role.<BR>
 *          bleNotReady: a scan is in progress.<BR>
 *          bleAlreadyInRequestedMode: can’t process now.<BR>
 *          bleNoResources: too many links.<BR>
 */
extern bStatus_t GAPCentralRole_EstablishLink(uint8_t highDutyCycle, uint8_t whiteList,
                                              uint8_t addrTypePeer, uint8_t *peerAddr);

/**
 * @brief   Update the link connection parameters.
 *
 * @param   connHandle - connection handle
 * @param   connIntervalMin - minimum connection interval in 1.25ms units
 * @param   connIntervalMax - maximum connection interval in 1.25ms units
 * @param   connLatency - number of LL latency connection events
 * @param   connTimeout - connection timeout in 10ms units
 *
 * @return  SUCCESS: Connection update started started.<BR>
 *          bleIncorrectMode: No connection to update.<BR>
 */
extern bStatus_t GAPCentralRole_UpdateLink(uint16_t connHandle, uint16_t connIntervalMin,
                                           uint16_t connIntervalMax, uint16_t connLatency,
                                           uint16_t connTimeout);
/**
 * @brief   Start a device discovery scan.
 *
 * @param   mode - discovery mode: @ref GAP_DEVDISC_MODE_DEFINES
 * @param   activeScan - TRUE to perform active scan
 * @param   whiteList - TRUE to only scan for devices in the white list
 *
 * @return  SUCCESS: Discovery scan started.<BR>
 *          bleIncorrectMode: Invalid profile role.<BR>
 *          bleAlreadyInRequestedMode: Not available.<BR>
 */
extern bStatus_t GAPCentralRole_StartDiscovery(uint8_t mode, uint8_t activeScan, uint8_t whiteList);

/**
 * @brief   Cancel a device discovery scan.
 *
 * @return  SUCCESS: Cancel started.<BR>
 *          bleInvalidTaskID: Not the task that started discovery.<BR>
 *          bleIncorrectMode: Not in discovery mode.<BR>
 */
extern bStatus_t GAPCentralRole_CancelDiscovery(void);

/**
 * @}
 */

/*-------------------------------------------------------------------
 * TASK FUNCTIONS - Don't call these. These are system functions.
 */
   
/**
 * @brief       Task creation function for the GAP Central Role.
 *
 * @param       none
 *
 * @return      none
 */
extern void GAPCentralRole_createTask(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CENTRAL_H */
