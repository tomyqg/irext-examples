/**************************************************************************************************
  Filename:       central.c
  Revised:        $Date: 2015-07-20 11:31:07 -0700 (Mon, 20 Jul 2015) $
  Revision:       $Revision: 44370 $

  Description:    GAP Central Role for RTOS Applications


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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include <driverlib/ioc.h>

#include "gap.h"
#include "gatt.h"
#include "hci_tl.h"
#include "linkdb.h"
#include "util.h"

#include "gattservapp.h"
#include "central.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "ICallBleAPIMSG.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Task configuration
#define GAPCENTRALROLE_TASK_PRIORITY     3

#ifndef GAPCENTRALROLE_TASK_STACK_SIZE
#define GAPCENTRALROLE_TASK_STACK_SIZE   440
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Link DB maximum number of connections
uint8 linkDBNumConns;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Semaphore globally used to post events to the application thread
static ICall_Semaphore sem;

// Task setup
Task_Struct gapCentralRoleTask;
Char gapCentralRoleTaskStack[GAPCENTRALROLE_TASK_STACK_SIZE];

// App callbacks
static gapCentralRoleCB_t *pGapCentralRoleCB;

/*********************************************************************
 * Profile Parameters - reference GAPCENTRALROLE_PROFILE_PARAMETERS for
 * descriptions
 */

static uint8_t  gapCentralRoleIRK[KEYLEN];
static uint8_t  gapCentralRoleSRK[KEYLEN];
static uint32_t gapCentralRoleSignCounter;
static uint8_t  gapCentralRoleBdAddr[B_ADDR_LEN];
static uint8_t  gapCentralRoleMaxScanRes = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void gapCentralRole_init(void);
static void gapCentralRole_taskFxn(UArg a0, UArg a1);

static uint8_t gapCentralRole_processStackMsg(ICall_Hdr *pMsg);
static uint8_t gapCentralRole_ProcessGAPMsg(gapEventHdr_t *pMsg);

/*********************************************************************
 * CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Start the device in Central role.  This function is typically
 *          called once during system startup.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_StartDevice(gapCentralRoleCB_t *pAppCallbacks)
{
  if (pAppCallbacks)
  {
    pGapCentralRoleCB = pAppCallbacks;
  }

  return GAP_DeviceInit(selfEntity, GAP_PROFILE_CENTRAL,
                        gapCentralRoleMaxScanRes, gapCentralRoleIRK,
                        gapCentralRoleSRK, (uint32*)&gapCentralRoleSignCounter);
}

/**
 * @brief   Set a parameter in the Central Profile.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_SetParameter(uint16_t param, uint8_t len, void *pValue)
{
  bStatus_t ret = SUCCESS;

  switch (param)
  {
    case GAPCENTRALROLE_IRK:
      if (len == KEYLEN)
      {
        VOID memcpy(gapCentralRoleIRK, pValue, KEYLEN) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPCENTRALROLE_SRK:
      if (len == KEYLEN)
      {
        VOID memcpy(gapCentralRoleSRK, pValue, KEYLEN) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPCENTRALROLE_SIGNCOUNTER:
      if (len == sizeof (uint32_t))
      {
        gapCentralRoleSignCounter = *((uint32_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPCENTRALROLE_MAX_SCAN_RES:
      if (len == sizeof (uint8_t))
      {
        gapCentralRoleMaxScanRes = *((uint8_t*)pValue);
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

  return ret;
}

/**
 * @brief   Get a parameter in the Central Profile.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_GetParameter(uint16_t param, void *pValue)
{
  bStatus_t ret = SUCCESS;

  switch (param)
  {
    case GAPCENTRALROLE_IRK:
      VOID memcpy(pValue, gapCentralRoleIRK, KEYLEN) ;
      break;

    case GAPCENTRALROLE_SRK:
      VOID memcpy(pValue, gapCentralRoleSRK, KEYLEN) ;
      break;

    case GAPCENTRALROLE_SIGNCOUNTER:
      *((uint32_t*)pValue) = gapCentralRoleSignCounter;
      break;

    case GAPCENTRALROLE_BD_ADDR:
      VOID memcpy(pValue, gapCentralRoleBdAddr, B_ADDR_LEN) ;
      break;

    case GAPCENTRALROLE_MAX_SCAN_RES:
      *((uint8_t*)pValue) = gapCentralRoleMaxScanRes;
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ret;
}

/**
 * @brief   Terminate a link.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_TerminateLink(uint16_t connHandle)
{
  return GAP_TerminateLinkReq(selfEntity, connHandle, HCI_DISCONNECT_REMOTE_USER_TERM) ;
}

/**
 * @brief   Establish a link to a peer device.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_EstablishLink(uint8_t highDutyCycle, uint8_t whiteList,
                                        uint8_t addrTypePeer, uint8_t *peerAddr)
{
  gapEstLinkReq_t params;

  params.taskID = ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, 
                                            selfEntity);
  params.highDutyCycle = highDutyCycle;
  params.whiteList = whiteList;
  params.addrTypePeer = addrTypePeer;
  VOID memcpy(params.peerAddr, peerAddr, B_ADDR_LEN);

  return GAP_EstablishLinkReq(&params);
}

/**
 * @brief   Update the link connection parameters.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_UpdateLink(uint16_t connHandle, uint16_t connIntervalMin,
                                    uint16_t connIntervalMax, uint16_t connLatency,
                                    uint16_t connTimeout)
{
  gapUpdateLinkParamReq_t params;

  params.connectionHandle = connHandle;
  params.intervalMin = connIntervalMin;
  params.intervalMax = connIntervalMax;
  params.connLatency = connLatency;
  params.connTimeout = connTimeout;
  
  return GAP_UpdateLinkParamReq(&params);
}

/**
 * @brief   Start a device discovery scan.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_StartDiscovery(uint8_t mode, uint8_t activeScan, uint8_t whiteList)
{
  gapDevDiscReq_t params;

  params.taskID = ICall_getLocalMsgEntityId(ICALL_SERVICE_CLASS_BLE_MSG, 
                                            selfEntity);
  params.mode = mode;
  params.activeScan = activeScan;
  params.whiteList = whiteList;

  return GAP_DeviceDiscoveryRequest(&params);
}

/**
 * @brief   Cancel a device discovery scan.
 *
 * Public function defined in central.h.
 */
bStatus_t GAPCentralRole_CancelDiscovery(void)
{
  return GAP_DeviceDiscoveryCancel(selfEntity);
}

/**
 * @brief   Task creation function for the GAP Central Role.
 *
 * @param   none
 *
 * @return  none
 */
void GAPCentralRole_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = gapCentralRoleTaskStack;
  taskParams.stackSize = GAPCENTRALROLE_TASK_STACK_SIZE;
  taskParams.priority = GAPCENTRALROLE_TASK_PRIORITY;
  
  Task_construct(&gapCentralRoleTask, gapCentralRole_taskFxn, &taskParams, NULL);
}

/**
 * @brief   Central Profile Task initialization function.
 *
 * @param   none
 *
 * @return  none
 */
static void gapCentralRole_init(void)
{
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &sem);

  // Get link DB maximum number of connections
  linkDBNumConns = linkDB_NumConns();

  // Initialize parameters

  // Restore items from NV
  VOID osal_snv_read(BLE_NVID_IRK, KEYLEN, gapCentralRoleIRK);
  VOID osal_snv_read(BLE_NVID_CSRK, KEYLEN, gapCentralRoleSRK);
  VOID osal_snv_read(BLE_NVID_SIGNCOUNTER, sizeof(uint32_t), 
                     &gapCentralRoleSignCounter);
}

/**
 * @brief   Task entry point for the GAP Peripheral Role.
 *
 * @param   a0 - first argument
 * @param   a1 - second argument
 *
 * @return  none
 */
static void gapCentralRole_taskFxn(UArg a0, UArg a1)
{
  // Initialize profile
  gapCentralRole_init();
  
  // Profile main loop
  for (;;)
  {
    // Waits for a signal to the semaphore associated with the calling thread.
    // Note that the semaphore associated with a thread is signaled when a
    // message is queued to the message receive queue of the thread or when
    // ICall_signal() function is called onto the semaphore.
    ICall_Errno errno = ICall_wait(ICALL_TIMEOUT_FOREVER);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;
      
      if (ICall_fetchServiceMsg(&src, &dest, 
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        uint8_t safeToDealloc = TRUE;
              
        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          ICall_Event *pEvt = (ICall_Event *)pMsg;
          
          // Check for BLE stack events first
          if (pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & GAP_EVENT_SIGN_COUNTER_CHANGED)
            {
              // Sign counter changed, save it to NV
              VOID osal_snv_write(BLE_NVID_SIGNCOUNTER, sizeof(uint32_t), 
                                  &gapCentralRoleSignCounter);
            }
          }
          else
          {
            // Process inter-task message
            safeToDealloc = gapCentralRole_processStackMsg((ICall_Hdr *)pMsg);
          }
        }

        if (pMsg && safeToDealloc)
        {
          ICall_freeMsg(pMsg);
        }
      }
    }
  }
}

/*********************************************************************
 * @fn      gapCentralRole_processStackMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming GAP message, FALSE otherwise.
 */
static uint8_t gapCentralRole_processStackMsg(ICall_Hdr *pMsg)
{
  uint8_t safeToDealloc = TRUE;
  
  switch (pMsg->event)
  {
    case GAP_MSG_EVENT:
      safeToDealloc = gapCentralRole_ProcessGAPMsg((gapEventHdr_t *) pMsg);
      break;

    default:
      break;
  }
  
  return (safeToDealloc);
}

/*********************************************************************
 * @fn      gapCentralRole_ProcessGAPMsg
 *
 * @brief   Process an incoming task message from GAP.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming GAP message, FALSE otherwise.
 */
static uint8_t gapCentralRole_ProcessGAPMsg(gapEventHdr_t *pMsg)
{
  switch (pMsg->opcode)
  {
    case GAP_DEVICE_INIT_DONE_EVENT:
      {
        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *) pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
          // Save off the generated keys
          VOID osal_snv_write(BLE_NVID_IRK, KEYLEN, gapCentralRoleIRK);
          VOID osal_snv_write(BLE_NVID_CSRK, KEYLEN, gapCentralRoleSRK);

          // Save off the information
          VOID memcpy(gapCentralRoleBdAddr, pPkt->devAddr, B_ADDR_LEN);
        }
      }
      break;

    case GAP_LINK_ESTABLISHED_EVENT:
      {
        gapEstLinkReqEvent_t *pPkt = (gapEstLinkReqEvent_t *) pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
          // Notify the Bond Manager of the connection
          VOID GAPBondMgr_LinkEst(pPkt->devAddrType, pPkt->devAddr,
                                   pPkt->connectionHandle, GAP_PROFILE_CENTRAL);
        }
      }
      break;

    case GAP_LINK_TERMINATED_EVENT:
      {
        uint16_t connHandle = ((gapTerminateLinkEvent_t *)pMsg)->connectionHandle;
        
        GAPBondMgr_LinkTerm(connHandle);
      }
      break;

    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
      {
        uint16_t connHandle = ((gapSlaveSecurityReqEvent_t *)pMsg)->connectionHandle;
        
        GAPBondMgr_SlaveReqSecurity(connHandle);
      }
      break;

    default:
      break;
  }

  // Pass event to app
  if (pGapCentralRoleCB && pGapCentralRoleCB->eventCB)
  {
    return (pGapCentralRoleCB->eventCB((gapCentralRoleEvent_t *)pMsg));
  }
  
  return (TRUE);
}


/*********************************************************************
*********************************************************************/
